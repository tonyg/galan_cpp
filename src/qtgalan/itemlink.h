// -*- C++ -*-
#ifndef galan_ITEMLINK_H
#define galan_ITEMLINK_H

#include <map>
#include <vector>

#include <qcanvas.h>

#include "galan/generator.h"

class ItemIcon;	// from itemicon.h

class ItemLink: public QCanvasLine {
public:
  ItemLink(ItemIcon *_source, ItemIcon *_target, QCanvas *_canvas);
  virtual ~ItemLink();

  void refresh();
  void reposition();

  ItemIcon *getSource() const { return source; }
  ItemIcon *getTarget() const { return target; }

private:
  ItemIcon *source;
  ItemIcon *target;
};

#endif
