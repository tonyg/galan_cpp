#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "mainwin.h"
#include "macroview.h"
#include "itemicon.h"
#include "itemhandle.h"
#include "IconLinkEditor.h"

#include <qstring.h>
#include <qtextstream.h>
#include <qpopupmenu.h>
#include <qinputdialog.h>

GALAN_USE_NAMESPACE

///////////////////////////////////////////////////////////////////////////

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

  QString msg;
  msg.sprintf("Removed item %s.", name.c_str());
  MainWin::StatusBar()->message(msg);
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

  // %%% Perhaps make this a configuration item? (And then a member
  // variable on ItemIcon... maybe even a static member)
  QFont f("System");
  f.setPixelSize(8);
  p.setFont(f);

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

QString ItemIcon::buildMenu(QPopupMenu *menu) {
  menu->setItemChecked(menu->insertItem("&Mute", this, SLOT(muteIcon()), CTRL+Key_M),
		       macro->findChild(name)->mute());
  menu->insertItem("&Rename...", this, SLOT(renameIcon()), Key_F2);
  menu->insertItem("De&lete", this, SLOT(deleteIcon()), Key_Delete);

  if (links.empty()) {
    menu->setItemEnabled(menu->insertItem("Edit links"), false);
    menu->setItemEnabled(menu->insertItem("Delete links"), false);
  } else {
    LinksMenu *linksEdit = new LinksMenu(menu, this, links);
    linksEdit->insertLinkItems(SLOT(editLinks()));
    menu->insertItem("Edit links", linksEdit);

    LinksMenu *linksDelete = new LinksMenu(menu, this, links);
    linksDelete->insertItem("All links", this, SLOT(deleteIconLinks()));
    linksDelete->insertSeparator();
    linksDelete->insertLinkItems(SLOT(disconnectAll()));
    menu->insertItem("Delete links", linksDelete);
  }

  return ("Generator " + name).c_str();
}

QString ItemIcon::buildTip() {
  Generator *g = macro->findChild(name);

  QString tip;
  tip.sprintf("Class: %s\n"
	      "Name: %s",
	      g->getClass().getDescription().c_str(),
	      name.c_str());
  return tip;
}

void ItemIcon::editLinksTo(ItemIcon *target) {
  ItemLink *link = findLinkTo(target, true);
  IconLinkEditor *dlg = new IconLinkEditorImpl(link, 0, "LinkEditor", true);
  if (dlg->exec() == QDialog::Accepted) {
    qDebug("Accepted new links (in ItemIcon::editLinks)!");
  }
  link->refresh();
}

void ItemIcon::disconnectFrom(ItemIcon *target, ItemLink *whichLink) {
  ItemLink *link = whichLink ? whichLink : findLinkTo(target, true);

  Generator *src = getGenerator();
  Generator *dst = target->getGenerator();

  GeneratorClass &srcClass(src->getClass());

  std::vector<OutputDescriptor *> src_qs = srcClass.getOutputs();
  for (unsigned int i = 0; i < src_qs.size(); i++) {
    // Make a copy of the reference returned, since we'll probably be
    // modifying the underlying object, so we want to keep an
    // untouched copy locally.
    Generator::conduitlist_t linksFrom = src->outboundLinks(*src_qs[i]);

    for (Generator::conduitlist_t::const_iterator j = linksFrom.begin();
	 j != linksFrom.end();
	 j++) {

      if ((*j)->dst == dst) {
	(*j)->unlink();
      }

    }
  }

  link->refresh();
}

void ItemIcon::disconnectFrom(ItemIcon *target) {
  disconnectFrom(target, 0);
}

void ItemIcon::muteIcon() {
  Generator *g = macro->findChild(name);
  bool mute = !g->mute();

  g->mute(mute);

  if (mute) {
    setBrush(QColor(64, 0, 0));
  } else {
    setBrush(Qt::black);
  }

  canvas()->setChanged(boundingRect());
}

void ItemIcon::renameIcon() {
  QString prompt;
  prompt.sprintf("Please enter the new name for \"%s\":", name.c_str());

 retry:
  bool ok = false;
  QString response = QInputDialog::getText("Rename Generator",
					   prompt,
					   QLineEdit::Normal,
					   name.c_str(),
					   &ok);

  if (ok && !response.isEmpty()) {
    std::string oldName = name;
    std::string newName = response.latin1();

    if (oldName != newName) {
      if (!macro->renameChild(oldName, newName)) {
	prompt.sprintf("That name is already taken. Please try a different name.\n"
		       "Please enter the new name for \"%s\":", name.c_str());
	goto retry;
      }

      name = newName;
      canvas()->setChanged(boundingRect());

      QString msg;
      QTextOStream(&msg)
	<< "Renamed " << oldName.c_str()
	<< " to " << newName.c_str() << ".";
      MainWin::StatusBar()->message(msg);
    }
  }
}

void ItemIcon::deleteIcon() {
  delete this;
}

void ItemIcon::deleteIconLinks() {
  for (links_t::iterator i = links.begin();
       i != links.end();
       i++) {
    (*i)->getSource()->disconnectFrom((*i)->getTarget(), (*i));
  }

  QString msg;
  msg.sprintf("Disconnected all links to or from %s.",
	      getItemName().c_str());
  MainWin::StatusBar()->message(msg);
}

void ItemIcon::refreshLinks() {
  links_t linksCopy = links;

  for (links_t::iterator i = linksCopy.begin();
       i != linksCopy.end();
       i++) {
    (*i)->refresh();
  }
}

///////////////////////////////////////////////////////////////////////////

LinksMenu::LinksMenu(QWidget *parent,
		     ItemIcon *_subject,
		     std::set<ItemLink *> const &_links)
  : QPopupMenu(parent),
    subject(_subject),
    links(_links.begin(), _links.end())
{}

void LinksMenu::insertLinkItems(char const *slotName) {
  for (int i = 0; i < links.size(); i++) {
    if (links[i]->getSource() == subject)
      insertLinkItem(slotName,
		     links[i],
		     std::string("to ") + links[i]->getTarget()->getItemName());
  }

  for (int i = 0; i < links.size(); i++) {
    if (links[i]->getTarget() == subject)
      insertLinkItem(slotName,
		     links[i],
		     std::string("from ") + links[i]->getSource()->getItemName());
  }
}

void LinksMenu::insertLinkItem(char const *slotName,
			       ItemLink *link,
			       std::string const &title)
{
  insertItem(title.c_str(), link->getHandle(), slotName);
}
