#ifndef SATTRIBUTEIO_H
#define SATTRIBUTEIO_H

#include "shift/sglobal.h"
#include "Utilities/XMacroHelpers.h"
#include "Utilities/XProperty.h"
#include "Containers/XStringSimple.h"
#include "Containers/XStringBuffer.h"

namespace Shift
{
class Attribute;

class SerialisationSymbol
  {
public:
  SerialisationSymbol() { }

private:
  X_DISABLE_COPY(SerialisationSymbol);
  };

class SerialisationValue
  {
public:
  virtual bool hasUtf8() const = 0;
  virtual bool hasBinary() const = 0;
  virtual Eks::String asUtf8(Eks::AllocatorBase* a) const { xAssertFail(); return Eks::String(a); }
  virtual Eks::Vector<xuint8> asBinary(Eks::AllocatorBase* a) const { xAssertFail(); return Eks::Vector<xuint8>(a); }
  };

template <typename T> class TypedSerialisationValue : public SerialisationValue
  {
public:
  TypedSerialisationValue(const T &t) : _val(t) { }
  bool hasUtf8() const X_OVERRIDE { return true; }
  bool hasBinary() const X_OVERRIDE { return false; }
  Eks::String asUtf8(Eks::AllocatorBase*) const X_OVERRIDE;

private:
  const T &_val;
  };

class AttributeIO
  {
public:
  typedef SerialisationSymbol Symbol;

  virtual const Symbol &modeSymbol() = 0;
  virtual const Symbol &inputSymbol() = 0;
  virtual const Symbol &valueSymbol() = 0;
  };

class AttributeSaver : public AttributeIO
  {
public:
  /// \brief write a value for the attribute, with symbol [id].
  virtual void writeValue(const Symbol &id, const SerialisationValue& value) = 0;

  template <typename T> void write(const Symbol &id, const T& t)
    {
    TypedSerialisationValue<T> val(t);

    writeValue(id, val);
    }
  };

class AttributeLoader : public AttributeIO
  {
public:
  /// \brief read a value for the attribute, with symbol [id].
  virtual const SerialisationValue& readValue(const Symbol &id) = 0;

  template <typename T> bool read(const Symbol &id, T&, Eks::AllocatorBase* a)
    {
    const SerialisationValue &val = readValue(id);

    if (val.hasUtf8())
      {
      Eks::String ret(a);
      Eks::String::Buffer buf(&ret);
      Eks::String::IStream str(&buf);

      xAssertFail(); // fix for strings - they should get all of toUtf8
      //str >> t;
      return true;
      }

    return false;
    }
  };

template <typename T>
Eks::String TypedSerialisationValue<T>::asUtf8(Eks::AllocatorBase* a) const
  {
  Eks::String ret(a);
  Eks::String::Buffer buf(&ret);
  Eks::String::OStream str(&buf);

  str << _val;
  return ret;
  }

template <> class SHIFT_EXPORT TypedSerialisationValue<QUuid> :  public SerialisationValue
  {
public:
  TypedSerialisationValue(const QUuid &t);

  bool hasUtf8() const X_OVERRIDE { return true; }
  bool hasBinary() const X_OVERRIDE { return false; }

  Eks::String asUtf8(Eks::AllocatorBase *a) const;

private:
  const QUuid &_val;
  };


}

#endif // SATTRIBUTEIO_H
