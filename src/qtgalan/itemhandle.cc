#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "macroview.h"
#include "itemhandle.h"
#include "itemlink.h"
#include "itemicon.h"

#include <qstring.h>
#include <qpopupmenu.h>

GALAN_USE_NAMESPACE

ItemHandle::ItemHandle(ItemLink *_link,
		       QCanvas *_canvas)
  : QCanvasPolygon(_canvas),
    link(_link)
{
  setBrush(Qt::white);
  setZ(MacroView::LINK_HEIGHT);
  rotateTo(0);
  show();
}

ItemHandle::~ItemHandle() {
}

static void rotate(QPointArray &a, double radians) {
  double s = sin(radians);
  double c = cos(radians);

  for (unsigned int i = 0; i < a.size(); i++) {
    double x = a[i].x();
    double y = a[i].y();
    a[i] = QPoint(x * c - y * s,
		  x * s + y * c);
  }
}

void ItemHandle::rotateTo(double radians) {
  QPointArray pa(3);
  pa[0] = QPoint(7, 0);
  pa[1] = QPoint(-5, -5);
  pa[2] = QPoint(-5, 5);
  rotate(pa, radians);
  setPoints(pa);
}

QString ItemHandle::buildMenu(QPopupMenu *menu) {
  menu->insertItem("Disconnect &all links", this, SIGNAL(disconnectAll()));
  menu->insertItem("&Edit links...", this, SIGNAL(editLinks()));

  return ("Link between " + link->getSource()->getItemName() +
	  " and " + link->getTarget()->getItemName()
	  ).c_str();
}

void ItemHandle::editLinks() {
  link->getSource()->editLinksTo(link->getTarget());
}

void ItemHandle::disconnectAll() {
  link->getSource()->disconnectFrom(link->getTarget());
}
