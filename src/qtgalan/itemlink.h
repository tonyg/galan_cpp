// -*- C++ -*-
#ifndef galan_ITEMLINK_H
#define galan_ITEMLINK_H

#include <map>
#include <vector>

#include <qcanvas.h>

#include "galan/generator.h"

class ItemIcon;		// from itemicon.h
class ItemHandle;	// from itemhandle.h

class ItemLink: public QCanvasLine {
public:
  ItemLink(ItemIcon *_source, ItemIcon *_target, QCanvas *_canvas);
  virtual ~ItemLink();

  void refresh();
  void reposition();

  ItemHandle *getHandle() const { return handle; }
  ItemIcon *getSource() const { return source; }
  ItemIcon *getTarget() const { return target; }

  static int const RTTI = 14642;
  virtual int rtti() const { return RTTI; }

private:
  ItemHandle *handle;
  ItemIcon *source;
  ItemIcon *target;
};

#endif
