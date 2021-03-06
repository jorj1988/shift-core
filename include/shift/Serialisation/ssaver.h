#ifndef SSAVER_H
#define SSAVER_H

#include "Memory/XUniquePointer.h"
#include "shift/sglobal.h"
#include "shift/Serialisation/sattributeinterface.h"
#include "shift/Serialisation/sioblock.h"

class QIODevice;

namespace Shift
{
class PropertyInformation;

class SHIFT_EXPORT Saver : public AttributeInterface, public IOBlockUser
  {
public:
  class WriteBlock : private IOBlock
    {
  XProperties:
    XROProperty(Eks::String *, device);

  public:
    WriteBlock(Saver *w, Eks::String *device);

  private:
    friend class Saver;
    };

  Saver();

  /// \brief Begin writing to [device].
  Eks::UniquePointer<WriteBlock> beginWriting(Eks::String *device);

  Eks::String *activeDevice();

  void onBegin(AttributeData *block, bool includeRoot, Eks::AllocatorBase *alloc) X_OVERRIDE;
  void onEnd(AttributeData *block) X_OVERRIDE;
  };

class SHIFT_EXPORT SaveBuilder
  {
public:
  void save(const Attribute *attr, bool includeRoot, AttributeInterface *receiver);

private:
  void visitAttribute(bool topLevel, const Attribute *attr, Saver::AttributeBlock *data, Eks::AllocatorBase *alloc);
  void visitValues(const Attribute *attr, Saver::AttributeBlock *data, Eks::AllocatorBase *alloc);
  void visitChildren(bool topLevel, const Attribute *attr, Saver::AttributeBlock *data, Eks::AllocatorBase *alloc);
  };

}

#endif // SSAVER_H
