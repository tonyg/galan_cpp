// -*- C++ -*-
#ifndef galan_ITEMICON_H
#define galan_ITEMICON_H

#include <string>
#include <vector>

#include <qcanvas.h>
#include <qpoint.h>
#include <qpainter.h>

#include "galan/macro.h"
#include "galan/generator.h"

#include "itemlink.h"

class ItemIcon: public QObject, public QCanvasRectangle {
  Q_OBJECT
  friend class ItemLink;

public:
  ItemIcon(Galan::Macro *_macro,
	   QPoint pos,
	   std::string const &_name,
	   QCanvas *_canvas);
  virtual ~ItemIcon();

  std::string const &getItemName() const { return name; }
  Galan::Generator *getGenerator() const { return macro->findChild(name); }

  virtual void moveBy(double dx, double dy);

  static int const RTTI = 14641;
  virtual int rtti() const { return RTTI; }

  void editLinksTo(ItemIcon *target);
  void disconnectFrom(ItemIcon *target);
  ItemLink *findLinkTo(ItemIcon *target, bool create);

  QString buildMenu(QPopupMenu *menu);

protected:
  virtual void drawShape(QPainter &p);

private slots:
  void deleteIcon();

private:
  void refreshLinks();
  
  Galan::Macro *macro;
  std::string name;

  typedef std::set<ItemLink *> links_t;
  links_t links;
};

#endif
