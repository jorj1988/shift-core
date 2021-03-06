#ifndef SDATABASE_H
#define SDATABASE_H

#include "Memory/XBucketAllocator.h"
#include "shift/sglobal.h"
#include "shift/sentity.h"
#include "shift/Changes/shandler.h"
#include "shift/TypeInformation/spropertyinstanceinformation.h"
#include "Memory/XUniquePointer.h"

namespace Eks
{
class TemporaryAllocatorCore;
}

namespace Shift
{
class AttributeInitialiserHelper;

/** \brief A database.

  \section Introduction
  An Database represents the full database, it holds every element created. There should only need to be single
  database for most applications. A database can contain multiple Handlers, which can (optionally) manage their
  own changes (implementing Undo), or share some common Change architecture. A database should be treated at as
  an Entity, the only difference being unlike normal Entities it should be constructed by the user.

  \sa Handler

  \section Creating a Database
  Databases can be created on the stack. A database should be created after the TypeRegistry is initialised. As a
  database inherits from Entity, once created, entities can be added, and data can be built up.

  \section Deriving from Database
  Deriving from database is similar to deriving from Entity, except as the database is the root object, its type must
  be explicitly stated by calling initiateInheritedDatabaseType in the constructor with the typeInformation() of
  the derived type passed in.
 */
class SHIFT_EXPORT Database : public Entity, public Handler
  {
  S_ENTITY(Database, Entity);

public:
  Database();
  ~Database();

  /// The path separator used when forming string paths to properties.
  static const Eks::Char *pathSeparator();
  /// The path separator used when forming string paths to properties. Escaped for convenience.
  static const Eks::Char *escapedPathSeparator();

  /// The persistent allocator to use for creating properties and property data.
  Eks::AllocatorBase *persistantAllocator()
    {
    return _memory;
    }

  /// A temporary allocator root to be used when allocating temporary memory inside Shift. For example, during computation.
  Eks::TemporaryAllocatorCore *temporaryAllocator()
    {
    return TypeRegistry::temporaryAllocator();
    }

  EditCache* findEditCache(Container *c);
  void addEditCache(Container *c, EditCache *);
  void removeEditCache(Container *c);

protected:
  /// Call this method from derived Database classes to ensure the hierarchy is set up correctly.
  void initiateInheritedDatabaseType(const PropertyInformation *info);

private:
  /// \brief Create a dynamic attribute
  Attribute *addDynamicAttribute(
      const PropertyInformation *info,
      const NameArg &name,
      xsize index,
      Container *parent,
      PropertyInstanceInformationInitialiser *inst);
  /// \brief Delete (for real) a dynamic attribute
  void deleteDynamicAttribute(Attribute *);

  /// \brief Initiate a constructed attribute, including any children.
  void initiateAttribute(
      Attribute *,
      AttributeInitialiserHelper* initialiser);
  /// \brief Initiate an attributes children.
  void initiateAttributeChildrenFromMetaData(
      Container *prop,
      const PropertyInformation *mD,
      AttributeInitialiserHelper *helper);
  /// \brief uninititate (and destroy) and attribute and its children
  void uninitiateAttribute(Attribute *thisProp);
  /// \brief uninitiate an attributes children
  void uninitiateAttributeChildrenFromMetaData(Container *container, const PropertyInformation *mD);

  /// \brief The databases instance information.
  DynamicInstanceInformation _instanceInfoData;

  /// \brief The allocator for all attributes.
  Eks::AllocatorBase *_memory;

#ifdef S_DEBUGGER
  Eks::UniquePointer<Shift::Debugger> _debugger;
#endif

  /// \brief The open edit caches for the database (see Container::createEditCache)
  std::pair<Container*, EditCache*> _lastEditCache;
  Eks::UnorderedMap<Container*, EditCache*> _editCaches;

  friend class Container;
  friend class ContainerTreeChange;
  };

}

S_PROPERTY_INTERFACE(Shift::Database)

#endif // SDATABASE_H
