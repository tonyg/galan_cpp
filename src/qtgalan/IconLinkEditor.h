// -*- C++ -*-
#ifndef ICONLINKEDITORIMPL_H
#define ICONLINKEDITORIMPL_H
#include "IconLinkEditor_base.h"

#include <map>
#include <set>

#include "galan/generator.h"

#include "itemicon.h"

class IconLinkEditorImpl : public IconLinkEditor
{ 
  Q_OBJECT

public:
  IconLinkEditorImpl(ItemLink *_link,
		     QWidget* parent = 0,
		     const char* name = 0,
		     bool modal = FALSE,
		     WFlags fl = 0);
  ~IconLinkEditorImpl();

protected slots:
  virtual void inputSelected(QListBoxItem*);
  virtual void outputSelected(QListBoxItem*);
  virtual void accept();

private:
  ItemLink *link;

  typedef std::set<Galan::InputDescriptor const *> inputSet_t;
  typedef std::map<Galan::OutputDescriptor const *, inputSet_t> outputMap_t;

  outputMap_t originalMap;
  outputMap_t outputMap;
};

#endif // ICONLINKEDITORIMPL_H
