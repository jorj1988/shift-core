#ifndef SITERATOR_H
#define SITERATOR_H

#include "sentity.h"

namespace SIterator
{
template <typename IteratorType, typename ReturnTypeIn, typename DerivedExtraData> class Base
  {
public:
  typedef DerivedExtraData ExtraData;
  typedef ReturnTypeIn ReturnType;

  inline Base() : _prop(0)
    {
    }

  inline void reset(SProperty *prop)
    {
    _prop = prop;
    }

  inline SProperty *property() const
    {
    return _prop;
    }

  class IteratorBase
    {
  public:
    inline IteratorBase()
      {
      }

    inline IteratorBase(ReturnType *p) : _property(p)
      {
      }

    inline ReturnType *operator*() const
      {
      return _property;
      }

    inline ReturnType *operator->() const
      {
      return _property;
      }

    inline void setProperty(ReturnType *prop)
      {
      _property = prop;
      }

    inline bool operator!=(const IteratorBase& it) const
      {
      return _property != it._property;
      }

  private:
    ReturnType *_property;
    };

  class Iterator : public IteratorBase
    {
  public:
    inline Iterator()
      {
      }

    inline Iterator(ReturnType *p) : IteratorBase(p)
      {
      }

    inline Iterator(ReturnType *p, const ExtraData& d)
        : IteratorBase(p),
          _extra(d)
      {
      }

    inline void operator++()
      {
      IteratorType::next(*this);
      }

    inline void operator++(int)
      {
      IteratorType::next(*this);
      }

    inline ExtraData& data()
      {
      return _extra;
      }

  private:
    ExtraData _extra;
    };

  inline Iterator begin()
    {
    Iterator i;
    static_cast<IteratorType*>(this)->first(i);
    return i;
    }

  inline Iterator end()
    {
    return Iterator(0);
    }

private:
  SProperty *_prop;
  };

namespace
{
struct NilExtraData
  {
  };

template <typename ToForward, typename ParentType> struct ForwarderExtraData
  {
  inline ForwarderExtraData()
    {
    }

  inline ForwarderExtraData(const typename ParentType::Iterator& i) : _parent(i)
    {
    }

  ToForward _fwd;
  typename ToForward::ExtraData _fwdData;
  typename ParentType::Iterator _parent;
  };
}

template <typename ToForward, typename ParentType>
class Forwarder : public Base<Forwarder<ToForward, ParentType>, typename ToForward::ReturnType, ForwarderExtraData<ToForward, ParentType> >
  {
public:
  Forwarder(ParentType *p)
      : _parent(p)
    {
    }

  inline void first(Iterator& i) const
    {
    first(_parent->begin(), i);
    }

  static void next(Iterator &i)
    {
    ToForward::Iterator fwd(*i, i.data()._fwdData);
    ToForward::next(fwd);

    if(!*fwd)
      {
      ++(i.data()._parent);
      first(i.data()._parent, i);
      }
    }

private:
  static void first(typename ParentType::Iterator &i, Iterator& ret)
    {
    if(*i)
      {
      ForwarderExtraData<ToForward, ParentType>& d = ret.data();
      d._parent = i;
      d._fwd.reset(*d._parent);

      ToForward::Iterator fwdIt;
      d._fwd.first(fwdIt);

      while(!*fwdIt)
        {
        ++(d._parent);
        d._fwd.reset(*d._parent);

        d._fwd.first(fwdIt);
        }

      d._fwdData = fwdIt.data();

      ret.setProperty(*fwdIt);
      }
    else
      {
      ret.setProperty(0);
      }
    }

  ParentType *_parent;
  };

template <typename A, typename B> class Compound : public Forwarder<B, A>
  {
public:
  Compound(SProperty *root) : Forwarder<B, A>(&_a)
    {
    _a.reset(root);
    }

  A _a;
  };

namespace
{
struct ChildTreeExtraData
  {
  SProperty *_root;
  };
}

class ChildTree : public Base<ChildTree, SProperty, ChildTreeExtraData>
  {
public:
  inline void first(Iterator& it) const
    {
    it.data()._root = property();
    it.setProperty(property());
    }

  static void next(Iterator &i)
    {
    SProperty *current = *i;
    SPropertyContainer *cont = current->castTo<SPropertyContainer>();
    if(cont)
      {
      SProperty *child = cont->firstChild();
      if(child)
        {
        i.setProperty(child);
        return;
        }
      }

    SProperty *n = current->nextSibling();

    SProperty *currentParent = current->parent();
    while(!n && currentParent != i.data()._root)
      {
      n = currentParent->nextSibling();
      currentParent = currentParent->parent();
      }

    i.setProperty(n);
    }
  };

class ChildEntityTree : public Base<ChildEntityTree, SEntity, ChildTreeExtraData>
  {
public:
  inline void first(Iterator& i) const
    {
    i.data()._root = property();
    i.setProperty(property()->entity());
    }

  inline static void next(Iterator &i)
    {
    SProperty *current = *i;
    SEntity *cont = current->castTo<SEntity>();
    // there is a non-entity in children?
    xAssert(cont);
    if(cont)
      {
      SEntity *child = cont->children.firstChild<SEntity>();
      if(child)
        {
        i.setProperty(child);
        return;
        }
      }

    SEntity *n = current->nextSibling<SEntity>();

    SProperty *currentParent = current->parent()->parent();
    while(!n && currentParent != i.data()._root)
      {
      n = currentParent->nextSibling<SEntity>();
      currentParent = currentParent->parent()->parent();
      }

    i.setProperty(n);
    }
  };

template <typename T> class OfType : public Base<OfType<T>, T, NilExtraData>
  {
public:
  inline void first(Iterator &i) const
    {
    i.setProperty(property()->castTo<T>());
    }

  inline static void next(Iterator &i)
    {
    i.setProperty(0);
    }
  };
}

#endif // SITERATOR_H
