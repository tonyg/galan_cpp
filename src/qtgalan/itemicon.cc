#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "macroview.h"
#include "itemicon.h"
#include "IconLinkEditor.h"

#include <qstring.h>
#include <qpopupmenu.h>

GALAN_USE_NAMESPACE

ItemIcon::ItemIcon(Macro *_macro,
		   QPoint pos,
		   std::string const &_name,
		   QCanvas *_canvas)
  : QCanvasRectangle(pos.x(), pos.y(), 40, 40, _canvas),
    macro(_macro),
    name(_name),
    links()
{
  setBrush(Qt::black);
  setPen(Qt::white);
  setZ(MacroView::ITEM_HEIGHT);
  show();
}

ItemIcon::~ItemIcon() {
  Generator *g = macro->findChild(name);
  if (g == 0) {
    qFatal("ItemIcon's name (%s) not found in macro %p", name.c_str(), macro);
  }

  if (!macro->removeChild(name)) {
    qWarning("Could not %p->removeChild(%s)", macro, name.c_str());
  }

  delete g;

  links_t linksCopy;
  links.swap(linksCopy);

  for (links_t::iterator i = linksCopy.begin();
       i != linksCopy.end();
       i++) {
    delete (*i);
  }
}

void ItemIcon::moveBy(double dx, double dy) {
  QCanvasRectangle::moveBy(dx, dy);

  for (links_t::iterator i = links.begin();
       i != links.end();
       i++) {
    (*i)->reposition();
  }
}

void ItemIcon::drawShape(QPainter &p) {
  QCanvasRectangle::drawShape(p);
  p.drawText(boundingRect(), Qt::AlignCenter, name.c_str());
}

ItemLink *ItemIcon::findLinkTo(ItemIcon *target, bool create) {
  for (links_t::iterator i = links.begin();
       i != links.end();
       i++) {
    if ((*i)->getTarget() == target)
      return *i;
  }

  ItemLink *l = new ItemLink(this, target, canvas());
  return l;
}

void ItemIcon::editLinksTo(ItemIcon *target) {
  ItemLink *link = findLinkTo(target, true);
  IconLinkEditor *dlg = new IconLinkEditorImpl(link, 0, "LinkEditor", true);
  if (dlg->exec() == QDialog::Accepted) {
    qDebug("Accepted new links (in ItemIcon::editLinks)!");
  }
  link->refresh();
}

void ItemIcon::disconnectFrom(ItemIcon *target) {
  Generator *src = getGenerator();
  Generator *dst = target->getGenerator();

  GeneratorClass &srcClass(src->getClass());

  std::vector<OutputDescriptor *> src_qs = srcClass.getOutputs();
  for (unsigned int i = 0; i < src_qs.size(); i++) {
    Generator::conduitlist_t const &linksFrom(src->outboundLinks(*src_qs[i]));
    for (Generator::conduitlist_t::const_iterator j = linksFrom.begin();
	 j != linksFrom.end();
	 j++) {
      if ((*j)->dst == dst) {
	(*j)->unlink();
      }
    }
  }
}

QString ItemIcon::buildMenu(QPopupMenu *menu) {
  menu->insertItem("Placeholder", 0, "");
  return ("Generator " + name).c_str();
}

void ItemIcon::refreshLinks() {
  links_t linksCopy = links;

  for (links_t::iterator i = linksCopy.begin();
       i != linksCopy.end();
       i++) {
    (*i)->refresh();
  }
}
