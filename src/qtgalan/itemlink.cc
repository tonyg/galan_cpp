#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "macroview.h"
#include "itemlink.h"
#include "itemicon.h"
#include "itemhandle.h"

GALAN_USE_NAMESPACE

ItemLink::ItemLink(ItemIcon *_source,
		   ItemIcon *_target,
		   QCanvas *_canvas)
  : QCanvasLine(_canvas),
    handle(new ItemHandle(this, _canvas)),
    source(_source),
    target(_target)
{
  setPen(Qt::white);
  source->links.insert(this);
  target->links.insert(this);
  setZ(MacroView::LINK_HEIGHT);

  reposition();
  show();
}

ItemLink::~ItemLink() {
  delete handle;
  source->links.erase(this);
  target->links.erase(this);
}

void ItemLink::refresh() {
  Generator *s = source->getGenerator();
  Generator *t = target->getGenerator();

  GeneratorClass &sc(s->getClass());
  GeneratorClass &tc(t->getClass());

  std::vector<OutputDescriptor *> outputs = sc.getOutputs();
  for (unsigned int i = 0; i < outputs.size(); i++) {
    Generator::conduitlist_t const &lst(s->outboundLinks(*outputs[i]));
    for (Generator::conduitlist_t::const_iterator j = lst.begin();
	 j != lst.end();
	 j++) {
      Conduit *link = *j;

      if (link->dst == t) {
	// Our source has at least one link targetting our target. Stay alive.
	return;
      }
    }
  }

  // Well, no outbound links point at target anymore. Die.
  delete this;
}

void ItemLink::reposition() {
  QPoint p1 = source->boundingRect().center();
  QPoint p2 = target->boundingRect().center();
  setPoints(p1.x(), p1.y(),
	    p2.x(), p2.y());

  QPoint v = p2 - p1;
  QPoint handleLoc = p1 + v / 2;

  handle->rotateTo(atan2(v.y(), v.x()));
  handle->move(handleLoc.x(), handleLoc.y());
}

#if 0
ItemIcon *ItemLink::remoteEnd(ItemIcon *nearEnd) {
  if (source == nearEnd) return target;
  if (target == nearEnd) return source;

  qWarning("Hmm: remoteEnd's nearEnd (%p) was neither source (%p) nor target (%p)!",
	   nearEnd,
	   source,
	   target);
  return 0;
}
#endif
