#ifndef SBASEPROPERTIES_INL
#define SBASEPROPERTIES_INL

#include "sbaseproperties.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "XTemporaryAllocator"
#include "shift/Changes/spropertychanges.h"
#include "shift/TypeInformation/spropertytraits.h"
#include "shift/TypeInformation/spropertygroup.h"

#if X_ASSERTS_ENABLED
#include "shift/sdatabase.h"
#endif

namespace Shift
{

namespace detail
{

template <typename T> class PODComputeChange : public Property::DataChange
  {
  typedef typename T::PODType PODType;
  S_CHANGE_TYPED(PODComputeChange, Property::DataChange, Change::ComputeChange, PODType);

public:
  PODComputeChange(T *prop)
    : Property::DataChange(prop)
    {
    xAssert(!prop->database()->stateStorageEnabled());
    }

  bool apply()
    {
    property()->postCompute();
    return true;
    }

private:
  bool unApply()
    {
    xAssertFail();
    return true;
    }
  bool inform(bool)
    {
    if(property()->entity())
      {
      property()->entity()->informDirtyObservers(property());
      }
    return true;
    }
  };

template <typename T> class PODChange : public PODComputeChange<T>
  {
  typedef typename T::PODType PODType;
  S_CHANGE_TYPED(PODChange, PODComputeChange, Change::DataChange, PODType);

XProperties:
  typedef typename T::PODType PODType;
  XRORefProperty(PODType, before);
  XRORefProperty(PODType, after);

public:
  PODChange(const PODType &b, const PODType &a, T *prop)
    : PODComputeChange(prop), _before(b), _after(a)
    { }

  bool apply()
    {
    Attribute *prop = PODComputeChange::property();
    T* d = prop->uncheckedCastTo<T>();
    d->_value = after();
    PODComputeChange::property()->postSet();
    return true;
    }

  bool unApply()
    {
    Attribute *prop = PODComputeChange::property();
    T* d = prop->uncheckedCastTo<T>();
    d->_value = before();
    PODComputeChange::property()->postSet();
    return true;
    }

  bool inform(bool)
    {
    Entity *ent = PODComputeChange::property()->entity();
    if(ent)
      {
      ent->informDirtyObservers(PODComputeChange::property());
      }
    return true;
    }
  };

template <typename T> class PODComputeLock
  {
public:
  typedef typename T::ComputeChange Change;

  PODComputeLock(T *ptr) : _ptr(ptr)
    {
    xAssert(ptr);
    _data = &_ptr->_value;
    }

  ~PODComputeLock()
    {
    if(_ptr)
      {
      PropertyDoChangeNonLocal(Change, _ptr, _ptr);
      }
    }

  PODComputeLock(PODComputeLock&& o)
    {
    _ptr = o._ptr;
    _data = o._data;
    o._ptr = 0;
    o._data = 0;
    }

  PODComputeLock& operator=(PODComputeLock&& o)
    {
    _ptr = o._ptr;
    _data = o._data;
    o._ptr = 0;
    o._data = 0;
    return *this;
    }

  typename T::PODType* data()
    {
    return _data;
    }

  typename T::PODType *operator->()
    {
    return _data;
    }

  operator typename T::PODType&()
    {
    return *_data;
    }

  PODComputeLock &operator=(const typename T::PODType &x)
    {
    *_data = x;
    return *this;
    }

private:
  X_DISABLE_COPY(PODComputeLock);

  T *_ptr;
  typename T::PODType *_data;
  };

template <typename T> class PODLock
  {
public:
  PODLock(T *ptr) : _ptr(ptr)
    {
    xAssert(ptr);
    _oldData = _ptr->value();
    _data = &_ptr->_value;
    }
  ~PODLock()
    {
    PropertyDoChange(Change, _oldData, *_data, _ptr);
    _data = 0;
    }

  T* data()
    {
    return _data;
    }

private:
  X_DISABLE_COPY(PODLock);

  T *_ptr;
  typename T::PODType* _data;
  typename T::PODType _oldData;
  };

}

template <typename T, DataMode Mode>
    void Data<T, Mode>::assign(const T &in)
  {
#if X_ASSERTS_ENABLED
  bool a = Data<T, Mode>::IsCopyable != 0;
  xAssert(a);
#endif
  PropertyDoChange(Change, Data<T, Mode>::_value, in, this);
  }

}

namespace Shift
{

template <typename T, DataMode Mode>
    void Data<T, Mode>::createTypeInformation(PropertyInformationTyped<Data<T, Mode>> *,
                                      const PropertyInformationCreateData &)
  {
  }

template <typename T, DataMode Mode>
    const Shift::PropertyInformation *Data<T, Mode>::
      bootstrapStaticTypeInformation(Eks::AllocatorBase *allocator)
  {
  typedef Data<T, Mode> ThisType;
  Shift::detail::checkType<ThisType>();

  Eks::TemporaryAllocator temp(TypeRegistry::temporaryAllocator());
  Eks::String name(&temp);

  const char *modeType = "Full";
  if(Mode == ComputedData)
    {
    modeType = "Computed";
    }
  else if(Mode == AttributeData)
    {
    modeType = "Attribute";
    }

  name.appendType(modeType);
  detail::MetaType::appendTypeName<T>(name);

  Shift::PropertyInformationTyped<ThisType>::bootstrapTypeInformation(
        staticTypeInformationInternal(),
        name.data(),
        ThisType::ParentType::bootstrapStaticTypeInformation(allocator), allocator);

  return staticTypeInformation();
  }

namespace detail
{

template <typename T> void getDefault(T *)
  {
  }

template <typename T, int IsFull> class PODEmbeddedInstanceInformation
    : public Property::EmbeddedInstanceInformation
  {
XProperties:
  typedef typename T::PODType PODType;
  XByRefProperty(PODType, defaultValue, setDefault);

public:
  PODEmbeddedInstanceInformation()
    {
    detail::getDefault(&_defaultValue);
    }

  virtual void initiateAttribute(Attribute *propertyToInitiate) const
    {
    Property::EmbeddedInstanceInformation::initiateAttribute(propertyToInitiate);
    propertyToInitiate->uncheckedCastTo<T>()->_value = defaultValue();
    }

  virtual void setDefaultValueFromString(const Eks::String &val)
    {
    Eks::String::Buffer s(&val);
    Eks::String::IStream stream(&s);
    xAssertFail();
    }

  void setDefaultValue(const PODType &val)
    {
    _defaultValue = val;
    }
  };

template <typename T> class PODEmbeddedInstanceInformation<T, false>
    : public Property::EmbeddedInstanceInformation
  {
XProperties:
  typedef typename T::PODType PODType;

public:
  };

template <typename T, int IsAttribute, int IsFull> class PODPropertyTraits
  : public PropertyBaseTraits
  {
public:
  static bool shouldSaveValue(const Attribute *)
    {
    return false;
    }

  enum
    {
    assign = 0,
    save = 0,
    load = 0
    };
  };

template <typename T, int IsAttribute> class PODPropertyTraits<T, IsAttribute, true>
  : public PropertyBaseTraits
  {
public:
  static void save(const Attribute *p, Saver &l)
    {
    PropertyBaseTraits::save(p, l);
    const T *ptr = p->uncheckedCastTo<T>();
    writeValue(l, ptr->_value);
    }

  static Attribute *load(Container *parent, Loader &l)
    {
    Attribute *prop = PropertyBaseTraits::load(parent, l);
    T *ptr = prop->uncheckedCastTo<T>();
    readValue(l, ptr->_value);
    return prop;
    }

  static bool shouldSaveValue(const Attribute *p)
    {
    const T *ptr = p->uncheckedCastTo<T>();

    if(PropertyBaseTraits::shouldSaveValue(p))
      {
      using ::operator!=;

      const typename T::PODType &def = ptr->embeddedInstanceInformation()->defaultValue();
      const typename T::PODType &val = ptr->value();

      if(ptr->isDynamic() || val != def)
        {
        return true;
        }
      }

    return false;
    }

  static void assign(const Shift::Attribute *p, Shift::Attribute *l)
    {
    T *t = l->uncheckedCastTo<T>();
    Shift::detail::assignTo(p, t);
    }
  };

#define S_POD_INFO_NAME(T, Mode) _staticTypeInformation ## T ## Mode
#define IMPLEMENT_POD_PROPERTY(EXPORT, group, T, Mode, niceName) \
  Shift::PropertyGroup::Information S_POD_INFO_NAME(niceName, Mode) = \
    group::propertyGroup().registerPropertyInformation( \
      &S_POD_INFO_NAME(niceName, Mode), \
      Shift::Data<T, Shift::Mode>::bootstrapStaticTypeInformation); \
  template <> \
      EXPORT const Shift::PropertyInformation *Shift::Data<T, Shift::Mode>::staticTypeInformation() { \
    return S_POD_INFO_NAME(niceName, Mode).information; } \
  template <> \
      EXPORT Shift::PropertyInformation **Shift::Data<T, Shift::Mode>::staticTypeInformationInternal() { \
    return &S_POD_INFO_NAME(niceName, Mode).information; }

}
}

#endif // SBASEPROPERTIES_INL