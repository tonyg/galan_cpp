#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <exception>

#include "regsel.h"

GALAN_USE_NAMESPACE

RegistrySelectionMenu::RegistrySelectionMenu(Registry const *base,
					     QWidget *parent,
					     char const *name)
  : QPopupMenu(parent, name)
{
  for (Registry::const_iterator i = base->begin();
       i != base->end();
       i++) {
    Registrable *r = (*i).second;
    if (r->isRegistry()) {
      RegistrySelectionMenu *subMenu = new RegistrySelectionMenu(r->toRegistry(), this);
      insertItem(r->getLocalname().c_str(), subMenu);
      connect(subMenu, SIGNAL(itemSelected(Galan::Registrable const *)),
	      this, SLOT(subItem(Galan::Registrable const *)));
    } else {
      itemMap[insertItem(r->getLocalname().c_str(), this, SLOT(itemSelected(int)))] = r;
    }
  }
}

RegistrySelectionMenu::~RegistrySelectionMenu() {
}

void RegistrySelectionMenu::subItem(Registrable const *item) {
  //IFDEBUG(cerr << "Node Item selected, fullpath = " << item->getFullpath() << endl);
  emit itemSelected(item);
}

void RegistrySelectionMenu::itemSelected(int id) {
  Registrable const *item = itemMap[id];

  if (item != 0) {
    //IFDEBUG(cerr << "Leaf Item selected, fullpath = " << item->getFullpath() << endl);
    emit itemSelected(item);
  } else {
    qWarning("Null item selected in RegistrySelectionMenu! Strange, but no big deal.");
  }
}
