// -*- C++ -*-
#ifndef galan_REGTREE_H
#define galan_REGTREE_H

#include <string>
#include <map>

#include "galan/registry.h"
#include "galan/model.h"

#include <qlistview.h>

class RegistryTreeView: public QListView, public Galan::View {
  Q_OBJECT
public:
  RegistryTreeView(Galan::Registry *base,
		   QString const &firstColumnLabel,
		   QWidget *parent = 0,
		   char const *name = 0);
  virtual ~RegistryTreeView();

  QListViewItem *lookupItem(Galan::Registrable *item) const;
  Galan::Registrable *lookupItem(QListViewItem *item) const;

  virtual void modelChanged(Galan::Model &model);

private:
  void buildTree(Galan::Registry *base,
		 QListViewItem *parent);
  void destroyTree(Galan::Registry *base,
		   QListViewItem *parent);

  typedef map<QListViewItem *, Galan::Registrable *> regMap_t;
  typedef map<Galan::Registrable *, QListViewItem *> itemMap_t;

  regMap_t regMap;
  itemMap_t itemMap;
};

#endif
