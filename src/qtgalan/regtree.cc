#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <exception>

#include "regtree.h"

GALAN_USE_NAMESPACE

RegistryTreeView::RegistryTreeView(Registry *base,
				   QString const &firstColumnLabel,
				   QWidget *parent,
				   char const *name)
  : QListView(parent, name),
    regMap(),
    itemMap()
{
  addColumn(firstColumnLabel);
  setRootIsDecorated(true);

  if (base)
    buildTree(base, 0);
}

RegistryTreeView::~RegistryTreeView() {
  for (regMap_t::iterator i = regMap.begin();
       i != regMap.end();
       ++i) {
    Registrable *r = (*i).second;

    if (r->isRegistry())
      r->removeDependent(*this);
  }
}

void RegistryTreeView::buildTree(Registry *base,
				 QListViewItem *parent)
{
  base->addDependent(*this);

  for (Registry::iterator i = base->begin();
       i != base->end();
       i++) {
    Registrable *r = (*i).second;

    QListViewItem *ri;
    if (parent) {
      ri = new QListViewItem(parent, r->getLocalname().c_str());
    } else {
      ri = new QListViewItem(this, r->getLocalname().c_str());
    }
    ri->setOpen(true);

    if (r->isRegistry()) {
      buildTree(r->toRegistry(), ri);
    } else {
      r->addDependent(*this);
    }

    regMap[ri] = r;
    itemMap[r] = ri;
  }
}

void RegistryTreeView::destroyTree(Registry *base,
				   QListViewItem *parent)
{
  base->removeDependent(*this);

  while (true) {
    QListViewItem *child = (parent ? parent->firstChild() : this->firstChild());

    if (child == 0)
      break;

    Registrable *childReg = regMap[child];

    if (childReg->isRegistry()) {
      destroyTree(childReg->toRegistry(), child);
    } else {
      childReg->removeDependent(*this);
    }

    regMap.erase(child);
    itemMap.erase(childReg);
    delete child;
  }
}

Registrable *RegistryTreeView::lookupItem(QListViewItem *item) const {
  regMap_t::const_iterator i = regMap.find(item);
  if (i == regMap.end()) {
    return 0;
  } else {
    return (*i).second;
  }
}

QListViewItem *RegistryTreeView::lookupItem(Registrable *item) const {
  itemMap_t::const_iterator i = itemMap.find(item);
  if (i == itemMap.end()) {
    return 0;
  } else {
    return (*i).second;
  }
}

void RegistryTreeView::modelChanged(Model &model) {
  Registrable &r(dynamic_cast<Registrable &>(model));

  qDebug("Model changed %p %s", &model, r.getFullpath().c_str());

  if (!r.isRegistry())
    return;

  Registry *base = r.toRegistry();
  QListViewItem *parent = itemMap[&r]; // will be 0 if the changed model is the root

  destroyTree(base, parent);
  buildTree(base, parent);

  if (parent)
    parent->setText(0, r.getLocalname().c_str());
}
