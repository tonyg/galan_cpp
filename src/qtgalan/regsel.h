// -*- C++ -*-
#ifndef galan_REGSEL_H
#define galan_REGSEL_H

#include <string>
#include <map>

#include "galan/registry.h"

#include <qpopupmenu.h>
#include <qpoint.h>

class RegistrySelectionMenu: public QPopupMenu {
  Q_OBJECT
public:
  RegistrySelectionMenu(Galan::Registry const *base, QWidget *parent = 0, char const *name = 0);
  virtual ~RegistrySelectionMenu();

signals:
  void itemSelected(const Galan::Registrable *item);

private slots:
  void subItem(const Galan::Registrable *item);
  void itemSelected(int id);

private:
  std::map<int, Galan::Registrable const *> itemMap;
};

#endif
