// -*- C++ -*-
#ifndef galan_ITEMICON_H
#define galan_ITEMICON_H

#include <string>
#include <vector>

#include <qcanvas.h>
#include <qpoint.h>
#include <qpainter.h>
#include <qpopupmenu.h>

#include "galan/macro.h"
#include "galan/generator.h"

#include "itemlink.h"

// Defined in this header: ItemIcon, LinksMenu

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

  bool mute() const { return getGenerator()->mute(); }

  virtual void moveBy(double dx, double dy);

  static int const RTTI = 14641;
  virtual int rtti() const { return RTTI; }

  ItemLink *findLinkTo(ItemIcon *target, bool create);

  QString buildMenu(QPopupMenu *menu);
  QString buildTip();

  void disconnectFrom(ItemIcon *target, ItemLink *whichLink);
  void disconnectFrom(ItemIcon *target);
  void editLinksTo(ItemIcon *target);

public slots:
  void muteIcon();
  void renameIcon();
  void deleteIcon();
  void deleteIconLinks();

protected:
  virtual void drawShape(QPainter &p);

private:
  void refreshLinks();
  
  Galan::Macro *macro;
  std::string name;

  typedef std::set<ItemLink *> links_t;
  links_t links;
};

///////////////////////////////////////////////////////////////////////////

class LinksMenu: public QPopupMenu {
public:
  LinksMenu(QWidget *parent,
	    ItemIcon *_subject,
	    std::set<ItemLink *> const &_links);

  void insertLinkItems(char const *slotName);

private:
  void insertLinkItem(char const *slotName, ItemLink *link, std::string const &title);

  ItemIcon *subject;
  std::vector<ItemLink *> links;
};

#endif
