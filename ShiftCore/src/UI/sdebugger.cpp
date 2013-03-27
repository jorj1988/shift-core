#include "shift\UI\sdebugger.h"
#include "QtWidgets/QToolBar"
#include "QtWidgets/QVBoxLayout"
#include "QtWidgets/QGraphicsView"
#include "QtWidgets/QGraphicsScene"
#include "QtWidgets/QMenu"
#include "QtWidgets/QGraphicsSceneMouseEvent"
#include "shift/sdatabase.h"
#include "shift/Properties/sproperty.h"
#include "shift/Properties/spropertycontaineriterators.h"
#include "shift/TypeInformation/sinterfaces.h"
#include "XTemporaryAllocator"

namespace Shift
{

Debugger::Debugger(Shift::Database *db, QWidget *parent) : QWidget(parent)
  {
  _db = db;

  move(10, 10);

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);

  QToolBar *toolbar = new QToolBar("mainTool", this);
  layout->addWidget(toolbar);

  toolbar->addAction("Snapshot", this, SLOT(snapshot()));

  _scene = new QGraphicsScene(this);

  _scene->setBackgroundBrush(QColor(64, 64, 64));

  QGraphicsView *view = new QGraphicsView(_scene, this);
  layout->addWidget(view);
  view->setRenderHints(QPainter::Antialiasing);
  view->setDragMode(QGraphicsView::ScrollHandDrag);
  }

void Debugger::snapshot()
  {
  Eks::TemporaryAllocator alloc(Shift::TypeRegistry::temporaryAllocator());
  Eks::UnorderedMap<Property *, DebugPropertyItem *> props(&alloc);
  DebugPropertyItem *snapshot = createItemForProperty(_db, &props);
  
  connectProperties(props);
  

  snapshot->setPos(width()/2, height()/2);
  snapshot->layout();

  _scene->addItem(snapshot);
  }
  
void Debugger::connectProperties(const Eks::UnorderedMap<Property *, DebugPropertyItem *> &itemsOut)
  {
  Eks::UnorderedMap<Property *, DebugPropertyItem *>::const_iterator it = itemsOut.begin();
  Eks::UnorderedMap<Property *, DebugPropertyItem *>::const_iterator end = itemsOut.end();
  for(; it != end; ++it)
    {
    const Property *p = it.key();
    if(p->input())
      {
      const auto &inItem = itemsOut[p->input()];
      xAssert(inItem);
      xAssert(it.value());
      
      new ConnectionItem(inItem, it.value(), true, Qt::red);
      }

    if(!p->isDynamic())
      {
      const EmbeddedPropertyInstanceInformation *child = p->baseInstanceInformation()->embeddedInfo();
      xAssert(child);

      const xsize *affectsLocations = child->affects();
      if(affectsLocations)
        {
        xuint8* parentLocation = (xuint8*)p;
        parentLocation -= child->location();

        for(;*affectsLocations; ++affectsLocations)
          {
          const xuint8* affectedLocation = parentLocation + *affectsLocations;
          Property *affectsProp = (Property *)affectedLocation;

          const auto &affectItem = itemsOut[affectsProp];

          new ConnectionItem(it.value(), affectItem, true, Qt::blue);
          }
        }
      }
    }
  }

DebugPropertyItem *Debugger::createItemForProperty(Property *prop, Eks::UnorderedMap<Property *, DebugPropertyItem *> *itemsOut)
  {
  QString text = "name: " + prop->name().toQString() + "<br>type: " + prop->typeInformation()->typeName().toQString();
  PropertyVariantInterface *ifc = prop->interface<PropertyVariantInterface>();
  if(ifc)
    {
    NoUpdateBlock b(prop);
    text += "<br>value: " + ifc->asString(prop).toQString();
    }

  bool dirty = prop->isDirty();
  text += "<br>dirty: ";
  text += (dirty ? "true" : "false");
  
  PropertyContainer *c = prop->castTo<PropertyContainer>();
  if(c)
    {
    int count = c->size();
    int embCount = c->typeInformation()->childCount();
    text += "<br>embedded children: " + QString::number(embCount);
    text += "<br>dynamic children: " + QString::number(count-embCount);
    }

  QColor colour = Qt::black;
  if(prop->isDynamic())
    {
    colour = Qt::blue;
    }

  DebugPropertyItem *item = new DebugPropertyItem(text, colour);
  if(itemsOut)
    {
    (*itemsOut)[prop] = item;
    }

  if(c)
    {
    xForeach(auto p, c->walker())
      {
      DebugPropertyItem *childItem = createItemForProperty(p, itemsOut);
      childItem->setParentItem(item);

      new ConnectionItem(item, childItem, false, Qt::black);
      }
    }
  
  return item;
  }

DebugPropertyItem::DebugPropertyItem(const QString &text, const QColor &colour)
    : _info(text), _outerColour(colour)
  {
  setAcceptedMouseButtons(Qt::LeftButton|Qt::RightButton);
  setFlag(QGraphicsItem::ItemIsMovable);
  }

QRectF DebugPropertyItem::boundingRect() const
  {
  qreal penWidth = 1;
  QSizeF size = _info.size();

  QRectF bnds(-5 - (penWidth / 2), - 5 - (penWidth / 2), size.width() + 10 + penWidth, size.height() + 10  + penWidth);

  return bnds;
  }

QRectF DebugPropertyItem::boundingRectWithChildProperties() const
  {
  QRectF r = boundingRect();
  xForeach(auto child, childItems())
    {
    DebugPropertyItem *childItem = qgraphicsitem_cast<DebugPropertyItem*>(child);
    if(childItem)
      {
      r |= childItem->boundingRect();
      }
    }

  return r;
  }

void DebugPropertyItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
  {
  QPointF initialPos = event->lastScenePos();
  QPointF nowPos = event->scenePos();

  QPointF diff = nowPos - initialPos;

  setPos(pos() + diff);
  }

void DebugPropertyItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
  {
  QMenu menu;
  menu.addAction("Hide", this, SLOT(hide()));
  menu.addAction("Hide Children", this, SLOT(hideChildren()));
  menu.addAction("Hide Siblings", this, SLOT(hideSiblings()));
  menu.addAction("Show Children", this, SLOT(showChildren()));
  menu.addAction("Auto Layout", this, SLOT(layout()));
  
  menu.exec(event->screenPos());
  }

void DebugPropertyItem::hide()
  {
  setVisible(false);
  }

void DebugPropertyItem::hideChildren()
  {
  xForeach(auto child, childItems())
    {
    DebugPropertyItem *childItem = qgraphicsitem_cast<DebugPropertyItem*>(child);
    if(childItem)
      {
      child->setVisible(false);
      }
    }
  }

void DebugPropertyItem::hideSiblings()
  {
  xForeach(auto child, parentItem()->childItems())
    {
    DebugPropertyItem *childItem = qgraphicsitem_cast<DebugPropertyItem*>(child);
    if(childItem && child != this)
      {
      child->setVisible(false);
      }
    }
  }
  
void DebugPropertyItem::showChildren()
  {
  xForeach(auto child, childItems())
    {
    child->setVisible(true);
    }
  }

void DebugPropertyItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
  {
  painter->setPen(_outerColour);
  painter->setBrush(Qt::white);

  QSizeF size = _info.size();
  painter->drawRoundedRect(-5, -5, size.width()+10, size.height()+10, 2, 2);

  painter->setPen(Qt::black);
  painter->drawStaticText(0, 0, _info);
  }
  
void DebugPropertyItem::layout()
  {
  float childWidth = 0.0f;
  float childHeight = 0.0f;
  xsize children = 0;
  xForeach(auto child, childItems())
    {
    DebugPropertyItem *childItem = qgraphicsitem_cast<DebugPropertyItem*>(child);
    if(childItem)
      {
      childItem->layout();

      QRectF bnds = boundingRectWithChildProperties();

      childWidth += bnds.width();
      childHeight = xMax(childHeight, (float)bnds.height());
      ++children;
      }
    }
    
  enum
    {
    GapX = 10,
    GapY = 10
    };
    
  float fullWidth =
      xMax((float)boundingRect().width(), childWidth + (float)(xMin(children-1, 0U) * GapX));
  
  childHeight += GapY;

  float currentX = -fullWidth/2.0f;
  xForeach(auto child, childItems())
    {
    DebugPropertyItem *childItem = qgraphicsitem_cast<DebugPropertyItem*>(child);
    if(childItem)
      {
      childItem->setPos(currentX, childHeight);
      currentX += boundingRectWithChildProperties().width(); + GapX;
      }
    }
  }

ConnectionItem::ConnectionItem(DebugPropertyItem *from, DebugPropertyItem *owner, bool h, QColor col)
  : QGraphicsObject(owner), _colour(col), _from(from), _owner(owner), _horizontal(h)
  {
  xAssert(_owner);
  xAssert(_from);

  connect(_owner, SIGNAL(xChanged()), this, SLOT(updateEndPoints()));
  connect(_owner, SIGNAL(yChanged()), this, SLOT(updateEndPoints()));
  connect(_from, SIGNAL(xChanged()), this, SLOT(updateEndPoints()));
  connect(_from, SIGNAL(yChanged()), this, SLOT(updateEndPoints()));
  }

void ConnectionItem::updateEndPoints()
  {
  prepareGeometryChange();
  update();
  }

void ConnectionItem::points(QPointF &from, QPointF &to) const
  {
  QRectF rA = _owner->boundingRect();
  QRectF rB = _from->boundingRect();

  QPointF ptA;
  QPointF ptB;
  if(_horizontal)
    {
    ptA = QPointF(rA.left(), Eks::lerp(rA.top(), rA.bottom(), 0.5f));
    ptB = QPointF(rB.right(), Eks::lerp(rB.top(), rB.bottom(), 0.5f));
    }
  else
    {
    ptA = QPointF(Eks::lerp(rA.left(), rA.right(), 0.5f), rA.top());
    ptB = QPointF(Eks::lerp(rB.left(), rB.right(), 0.5f), rB.bottom());
    }

  from = mapFromItem(_owner, ptA);
  to = mapFromItem(_from, ptB);
  }

QRectF ConnectionItem::boundingRect() const
  {
  QPointF ptA;
  QPointF ptB;
  points(ptA, ptB);

  QSizeF s(2, 2);

  return QRectF(ptA, s) | QRectF(ptB, s);
  }

void ConnectionItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
  {
  if(!_from->isVisible())
    {
    return;
    }

  painter->setPen(QPen(_colour));

  QPointF ptA;
  QPointF ptB;
  points(ptA, ptB);

  painter->drawLine(ptA, ptB);
  }

}