#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <iterator>
#include <algorithm>

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
    originalMap(),
    outputMap()
{
  Generator *g = link->getSource()->getGenerator();
  Generator *target = link->getTarget()->getGenerator();

  GeneratorClass &gc(g->getClass());

  // First, we build an internal map of the interconnection state of
  // the two generators. We map from the source's outputs to a set of
  // the inputs on the target that are currently connected to each
  // output.

  OutputDescriptor const *defaultOutput = gc.getDefaultOutput();
  std::vector<OutputDescriptor *> outputs = gc.getOutputs();

  for (unsigned int i = 0; i < outputs.size(); i++) {
    inputSet_t &s(originalMap[outputs[i]]);
    Generator::conduitlist_t const &links(g->outboundLinks(*outputs[i]));

    for (Generator::conduitlist_t::const_iterator i = links.begin();
	 i != links.end();
	 i++) {
      Conduit *conduit = *i;
      // We only need to check the target here; specifically don't
      // need to check for compatibility between the inputs and
      // outputs, because they're already connected, so we already
      // checked.
      if (conduit->dst == target)
	s.insert(const_cast<InputDescriptor *>(conduit->dst_q));
    }
  }

  // Need to copy originalMap into outputMap here, rather than below
  // the initialisation of OutputsList, because when we call
  // OutputsList->setSelected, the slot outputSelected is fired, which
  // scans outputMap to find out what is currently connected to that
  // output.
  outputMap = originalMap;

  for (outputMap_t::const_iterator i = originalMap.begin();
       i != originalMap.end();
       i++) {
    OutputsList->insertItem((*i).first->getName().c_str());
    if ((*i).first == defaultOutput) {
      // Select it.
      OutputsList->setSelected(OutputsList->count() - 1, true);
    }
  }
}

IconLinkEditorImpl::~IconLinkEditorImpl() {
}

void IconLinkEditorImpl::accept() {
  Generator *src = link->getSource()->getGenerator();
  Generator *dst = link->getTarget()->getGenerator();

  for (outputMap_t::iterator i = outputMap.begin(); i != outputMap.end(); i++) {
    OutputDescriptor const *src_q = (*i).first;
    inputSet_t &originalSet(originalMap[src_q]);
    inputSet_t &finalSet(outputMap[src_q]);

    inputSet_t toAdd;
    inputSet_t toRemove;

    // toAdd = finalSet - originalSet.
    set_difference(finalSet.begin(), finalSet.end(),
		   originalSet.begin(), originalSet.end(),
		   inserter(toAdd, toAdd.begin()));

    // toRemove = originalSet - finalSet.
    set_difference(originalSet.begin(), originalSet.end(),
		   finalSet.begin(), finalSet.end(),
		   inserter(toRemove, toRemove.begin()));

    for (inputSet_t::iterator i = toRemove.begin(); i != toRemove.end(); i++) {
      InputDescriptor const *dst_q = (*i);
      IFDEBUG(cerr << "Removing link from " << src_q->getName()
	      << " to " << dst_q->getName() << endl);
      src->unlink(*src_q, dst, *dst_q);
    }

    for (inputSet_t::iterator i = toAdd.begin(); i != toAdd.end(); i++) {
      InputDescriptor const *dst_q = (*i);
      IFDEBUG(cerr << "Adding link from " << src_q->getName()
	      << " to " << dst_q->getName() << endl);
      src->link(*src_q, dst, *dst_q);
    }
  }

  IconLinkEditor::accept();
}

void IconLinkEditorImpl::outputSelected(QListBoxItem *item) {
  std::string itemName(item->text());
  //IFDEBUG(cerr << "Chose output" << itemName << endl);

  OutputDescriptor const &src_q(link->getSource()->getGenerator()->getClass().getOutput(itemName));
  inputSet_t &actives(outputMap[&src_q]);

  std::vector<InputDescriptor *> dst_qs =
    link->getTarget()->getGenerator()->getClass().getInputs();

  InputsList->clear();
  for (unsigned int i = 0; i < dst_qs.size(); i++) {
    if (src_q.compatibleWith(dst_qs[i])) {
      // This input is compatible with the chosen output.
      InputsList->insertItem(dst_qs[i]->getName().c_str());
      if (actives.find(dst_qs[i]) != actives.end()) {
	// This input is currently connected.
	InputsList->setSelected(InputsList->count() - 1, true);
      }
    }
  }

  InputsList->setEnabled(InputsList->count() > 0);
}

void IconLinkEditorImpl::inputSelected(QListBoxItem *item) {
  std::string outputName((char const *) OutputsList->currentText());
  std::string itemName(item->text());
  //IFDEBUG(cerr << "Chose input " << itemName << endl);

  OutputDescriptor const &src_q
    (link->getSource()->getGenerator()->getClass().getOutput(outputName));

  InputDescriptor const &dst_q
    (link->getTarget()->getGenerator()->getClass().getInput(itemName));

  inputSet_t &actives(outputMap[&src_q]);

  if (InputsList->isSelected(item))
    actives.insert(&dst_q);
  else
    actives.erase(&dst_q);
}
