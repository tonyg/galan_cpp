// -*- C++ -*-
#ifndef galan_ITEMHANDLE_H
#define galan_ITEMHANDLE_H

#include <map>
#include <vector>

#include <qcanvas.h>

#include "itemlink.h"

class ItemHandle: public QObject, public QCanvasPolygon {
  Q_OBJECT
public:
  ItemHandle(ItemLink *_link, QCanvas *_canvas);
  virtual ~ItemHandle();

  void rotateTo(double radians);

  ItemLink *getLink() const { return link; }

  static int const RTTI = 14643;
  virtual int rtti() const { return RTTI; }

  QString buildMenu(QPopupMenu *menu);

private slots:
  void editLinks();
  void disconnectAll();

private:
  ItemLink *link;
};

#endif
