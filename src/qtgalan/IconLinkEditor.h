// -*- C++ -*-
#ifndef ICONLINKEDITORIMPL_H
#define ICONLINKEDITORIMPL_H
#include "IconLinkEditor_base.h"

#include <map>
#include <vector>

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
  virtual void inputSelected(const QString&);
  virtual void outputSelected(const QString&);
  virtual void accept();

private:
  ItemLink *link;

  typedef std::vector<Galan::InputDescriptor *> inputVec_t;
  typedef std::map<Galan::OutputDescriptor *, inputVec_t> outputMap_t;

  outputMap_t outputMap;
};

#endif // ICONLINKEDITORIMPL_H
