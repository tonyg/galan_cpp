#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "macroview.h"

GALAN_USE_NAMESPACE

MacroView::MacroView(Macro *_macro, QWidget *parent, char const *name, WFlags f)
  : QCanvasView(0, parent, name, f),
    macro(_macro),
    c(new QCanvas(1024, 1024)),
    halo(new QCanvasRectangle(c)),
    selection(0),
    linkLine(0),
    editState(IDLE)
{
  c->setBackgroundColor(Qt::black);

  halo->setBrush(QBrush(NoBrush));
  halo->setPen(QPen(Qt::white, 3));
  halo->setZ(HALO_DEPTH);

  QCanvasPolygonalItem *r = new QCanvasRectangle(10, 10, 30, 20, c);
  r->setBrush(Qt::yellow);
  r->setPen(Qt::white);
  r->setZ(ITEM_DEPTH);
  r->show();

  setCanvas(c);
}

MacroView::~MacroView() {
  delete c;
}

QCanvasItem *MacroView::topItemAt(QPoint const &p) {
  QCanvasItemList l = c->collisions(p);
  for (QCanvasItemList::Iterator i = l.begin();
       i != l.end();
       i++) {
    if ((*i)->z() == ITEM_DEPTH) {
      // We only want items (so exclude lines and the halo)
      return (*i);
    }
  }
  return 0;
}

void MacroView::setSelection(QCanvasItem *sel) {
  selection = sel;
  updateHalo();
}

void MacroView::updateHalo() {
  if (selection) {
    QRect r = selection->boundingRect();
    halo->move(r.x() - 5, r.y() - 5);
    halo->setSize(r.width() + 9, r.height() + 9);
    halo->show();
  } else {
    halo->hide();
  }
  c->update();
}

void MacroView::createLink(QMouseEvent *evt) {
  QCanvasItem *endpoint = topItemAt(evt->pos());

  if (endpoint == 0) {
    // Dropped in space. This is a cancel.
    delete linkLine;
  } else if (endpoint == selection) {
    // Dropped on originating object!
    IFDEBUG(cerr << "Ignoring link from thing to itself." << endl);
    delete linkLine;
  } else {
    // Dropped on something else.
    cerr << "Would connect to " << (void *) endpoint << " if we could." << endl;
    // %%% Must use up linkLine here!
    delete linkLine;
  }

  // If we removed the line, we need to update here to refresh the display.
  c->update();
}

void MacroView::moveItem(QMouseEvent *evt) {
  selection->move(evt->x() - moveOffset.x(),
		  evt->y() - moveOffset.y());
  updateHalo();
}

void MacroView::popupMenu(QMouseEvent *evt) {
  cerr << "UNIMPLEMENTED: MacroView::popupMenu" << endl;
}

void MacroView::contentsMousePressEvent(QMouseEvent *evt) {
  switch (editState) {
    case IDLE: {
      Qt::ButtonState button = evt->button();
      Qt::ButtonState state = evt->state();

      setSelection(topItemAt(evt->pos()));

      if (button == MidButton) {
	// Drag the view around.
	moveOffset = QPoint(contentsX() + evt->globalX(),
			    contentsY() + evt->globalY());
	editState = DRAGGING_VIEW;
      } else if ((button == LeftButton) && (state & ShiftButton)) {
	// Connect objects - either basic or detailed (without/with ctrl)
	if (selection) {
	  QPoint startPoint = selection->boundingRect().center();
	  linkLine = new QCanvasLine(c);
	  linkLine->setPoints(startPoint.x(), startPoint.y(), evt->x(), evt->y());
	  linkLine->setPen(Qt::white);
	  linkLine->setZ(LINK_DEPTH);
	  linkLine->show();
	  editState = DRAGGING_LINK;
	}
      } else if (((button == LeftButton) && (state & ControlButton)) ||
		 (button == RightButton)) {
	// Popup menu. (C-left or ANY-right)
	popupMenu(evt);
      } else if (button == LeftButton) {
	// Plain old left-click (shift and control are filtered out
	// above).
	if (selection) {
	  moveOffset = QPoint(evt->x() - selection->x(),
			      evt->y() - selection->y());
	  editState = DRAGGING_ITEM;
	}
      }
      break;
    }

    case DRAGGING_LINK:
    case DRAGGING_ITEM:
    case DRAGGING_VIEW:
      // Ignore further mousedowns.
      break;

    default:
      g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
	    "Unknown MacroView EditState! Ignoring press event...");
      break;
  }
}

void MacroView::contentsMouseMoveEvent(QMouseEvent *evt) {
  switch (editState) {
    case IDLE:
      // Ignore.
      break;

    case DRAGGING_LINK:
      linkLine->setPoints(linkLine->startPoint().x(),
			  linkLine->startPoint().y(),
			  evt->x(),
			  evt->y());
      c->update();
      break;

    case DRAGGING_ITEM:
      moveItem(evt);
      break;

    case DRAGGING_VIEW:
      setContentsPos(moveOffset.x() - evt->globalX(),
		     moveOffset.y() - evt->globalY());
      break;

    default:
      g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
	    "Unknown MacroView EditState! Ignoring move event...");
      break;
  }
}

void MacroView::contentsMouseReleaseEvent(QMouseEvent *evt) {
  switch (editState) {
    case IDLE:
      break;

    case DRAGGING_LINK:
      createLink(evt);
      editState = IDLE;
      break;

    case DRAGGING_ITEM:
      moveItem(evt);
      editState = IDLE;
      break;

    case DRAGGING_VIEW:
      editState = IDLE;
      break;

    default:
      g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
	    "Unknown MacroView EditState! Ignoring release event...");
      break;
  }
}
