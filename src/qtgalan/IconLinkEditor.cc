#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "IconLinkEditor.h"

#include <qlistbox.h>

GALAN_USE_NAMESPACE

IconLinkEditorImpl::IconLinkEditorImpl(ItemLink *_link,
				       QWidget* parent,
				       const char* name,
				       bool modal,
				       WFlags fl)
  : IconLinkEditor(parent, name, modal, fl),
    link(_link),
    outputMap()
{
  Generator *g = link->getSource()->getGenerator();
  Generator *target = link->getTarget()->getGenerator();

  GeneratorClass &gc(g->getClass());

  std::vector<OutputDescriptor *> outputs = gc.getOutputs();
  for (unsigned int i = 0; i < outputs.size(); i++) {
    inputVec_t &v(outputMap[outputs[i]]);
    Generator::conduitlist_t const &links(g->outboundLinks(*outputs[i]));

    for (Generator::conduitlist_t::const_iterator i = links.begin();
	 i != links.end();
	 i++) {
      Conduit *conduit = *i;
      if (conduit->dst == target)
	v.push_back(const_cast<InputDescriptor *>(conduit->dst_q));
    }
  }

  for (outputMap_t::const_iterator i = outputMap.begin();
       i != outputMap.end();
       i++) {
    OutputsList->insertItem((*i).first->getName().c_str());
  }
}

IconLinkEditorImpl::~IconLinkEditorImpl() {
}

void IconLinkEditorImpl::accept() {
  qDebug("Okay! Accepted.");
  IconLinkEditor::accept();
}

void IconLinkEditorImpl::outputSelected(const QString&str) {
  qDebug("Chose output %s", (char const *) str);
}

void IconLinkEditorImpl::inputSelected(const QString&str) {
  qDebug("Chose input %s", (char const *) str);
}
