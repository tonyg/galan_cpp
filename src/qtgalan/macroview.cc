#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <strstream>

#include "mainwin.h"
#include "macroview.h"
#include "regsel.h"
#include "itemicon.h"
#include "itemhandle.h"

#include "galan/controller.h"

#include <qpopupmenu.h>
#include <qtooltip.h>

GALAN_USE_NAMESPACE

///////////////////////////////////////////////////////////////////////////

class MacroView::DynamicTip: public QToolTip {
public:
  DynamicTip(MacroView *m);
protected:
  void maybeTip(QPoint const &p);
private:
  MacroView *macroView;
};

MacroView::DynamicTip::DynamicTip(MacroView *m)
  : QToolTip(m),
    macroView(m)
{}

void MacroView::DynamicTip::maybeTip(QPoint const &p) {
  QCanvasItem *item = macroView->topItemAt(p);

  if (!item)
    return;

  ItemIcon *ii = dynamic_cast<ItemIcon *>(item);

  tip(ii->boundingRect(), ii->buildTip());
}

///////////////////////////////////////////////////////////////////////////

MacroView::MacroView(Macro *_macro, QWidget *parent, char const *name, WFlags f)
  : QCanvasView(0, parent, name, f),
    macro(_macro),
    c(new QCanvas(2048, 2048)),
    halo(new QCanvasRectangle(c)),
    selection(0),
    linkLine(0),
    editState(IDLE),
    focusPos()
{
  c->setBackgroundColor(Qt::black);

  halo->setBrush(QBrush(NoBrush));
  halo->setPen(QPen(Qt::white, 3));
  halo->setZ(HALO_HEIGHT);
  // Keep the halo hidden until a selection comes along.

  setCanvas(c);
  viewport()->setMouseTracking(true);

  // We want a tooltip manager.
  new DynamicTip(this);
}

MacroView::~MacroView() {
  // First, clear out all the ItemIcon instances. Otherwise things get
  // deleted in the wrong order, and iterators get invalidated,
  // causing SEGV deep inside ~QCanvas.
  //
  QCanvasItemList l = c->allItems();
  for (QCanvasItemList::Iterator i = l.begin(); i != l.end(); i++) {
    // We only want items (so exclude lines and the halo)
    if ((*i)->rtti() == ItemIcon::RTTI) {
      delete (*i);
    }
  }

  delete c;
}

QCanvasItemList MacroView::itemsAt(QPoint p) {
  QCanvasItemList l = c->collisions(p);
  QCanvasItemList r;
  for (QCanvasItemList::Iterator i = l.begin(); i != l.end(); i++) {
    // We only want items and handles (so exclude lines and the halo)
    switch ((*i)->rtti()) {
      case ItemIcon::RTTI:
      case ItemHandle::RTTI:
	r.append(*i);
	break;

      default:
	break;
    }
  }
  return r;
}

QCanvasItem *MacroView::topItemAt(QPoint p) {
  QCanvasItemList l = c->collisions(p);
  for (QCanvasItemList::Iterator i = l.begin(); i != l.end(); i++) {
    // We only want items (so exclude lines and the halo)
    if ((*i)->rtti() == ItemIcon::RTTI) {
      return (*i);
    }
  }
  return 0;
}

void MacroView::setSelection(QCanvasItem *sel) {
  QCanvasItem *oldSel = selection;

  if (selection) selection->setZ(ITEM_HEIGHT);
  selection = sel;
  if (selection) selection->setZ(SELECTION_HEIGHT);

  if (oldSel != selection) {
    selectionChanged();
  }

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

  delete linkLine;
  c->update();

  if (endpoint == 0) {
    // Dropped in space. This is a cancel.
  } else if (endpoint == selection) {
    // Dropped on originating object!
    IFDEBUG(cerr << "Ignoring link from thing to itself." << endl);
  } else {
    // Dropped on something else.
    ItemIcon *source = dynamic_cast<ItemIcon *>(selection);
    ItemIcon *target = dynamic_cast<ItemIcon *>(endpoint);

    if (source == 0) qFatal("Source of link is not an ItemIcon!");
    if (target == 0) qFatal("Target of link is not an ItemIcon!");

    Qt::ButtonState state = evt->state();

    // Test to see if we can shortcircuit the connection process.

    if (state & ControlButton) {
      // Want detailed connection.
      IFDEBUG(cerr << "ControlButton on link --> detailed connection" << endl);
      source->editLinksTo(target);
    } else {
      // Maybe shortcircuit.
      Generator *src = source->getGenerator();
      Generator *dst = target->getGenerator();
      OutputDescriptor const *src_q = src->getClass().getDefaultOutput();
      InputDescriptor const *dst_q = dst->getClass().getDefaultInput();

      IFDEBUG(cerr << "Link: src " << src << " dst " << dst
	      << " src_q " << src_q << " dst_q " << dst_q << endl);

      if (src_q && dst_q && src_q->compatibleWith(dst_q)) {
	QString msg;
	msg.sprintf("Connected from %s of %s to %s of %s.",
		    src_q->getName().c_str(),
		    source->getItemName().c_str(),
		    dst_q->getName().c_str(),
		    target->getItemName().c_str());
	MainWin::StatusBar()->message(msg);

	src->link(*src_q, dst, *dst_q);
	source->findLinkTo(target, true);	// we ignore result here.
      } else {
	// Not compatible, or no defaults. Use detailed connection.
	IFDEBUG(cerr << "Not compatible or no defaults --> detailed connection" << endl);
	source->editLinksTo(target);
      }
    }
  }

  // We need to update here to refresh the display, just in case we
  // added a link above.
  c->update();
}

void MacroView::moveItem(QMouseEvent *evt) {
  selection->move(evt->x() - moveOffset.x(),
		  evt->y() - moveOffset.y());
  updateHalo();
}

void MacroView::popupMenu(QMouseEvent *evt) {
  focusPos = evt->pos();

  QCanvasItemList items = itemsAt(focusPos);

  QPopupMenu menu(this);
  menu.insertItem("New macro...", 0, "");	// %%% new class for New Macro dialog etc.

  menu.setItemEnabled(menu.insertItem("New control", this, SLOT(newControl()), CTRL+Key_N),
		      Controller::have_active_instance());

  RegistrySelectionMenu *regSel = 
    new RegistrySelectionMenu(Registry::root->lookup("Generator")->toRegistry(), &menu);
  menu.insertItem("New primitive", regSel);
  connect(regSel, SIGNAL(itemSelected(Galan::Registrable const *)),
	  this, SLOT(createPrimitive(Galan::Registrable const *)));

  menu.insertSeparator();

  int counter = 0;
  for (QCanvasItemList::Iterator i = items.begin();
       i != items.end();
       i++) {
    QPopupMenu *itemMenu = new QPopupMenu(&menu);
    QString itemCaption;

    switch ((*i)->rtti()) {
      case ItemIcon::RTTI:
	itemCaption = dynamic_cast<ItemIcon *>(*i)->buildMenu(itemMenu);
	break;

      case ItemHandle::RTTI:
	itemCaption = dynamic_cast<ItemHandle *>(*i)->buildMenu(itemMenu);
	break;

      default:
	qWarning("Unknown RTTI %d in MacroView::popupMenu", (*i)->rtti());
	break;
    }

    menu.insertItem(itemCaption, itemMenu);
  }

  menu.exec(evt->globalPos(), 3);
}

void MacroView::muteSelectedIcon() {
  if (selection) {
    ItemIcon *ii = dynamic_cast<ItemIcon *>(selection);
    if (ii) {
      ii->muteIcon();
      selectionChanged();	// force update of Edit menu
      c->update();
    }
  }
}

void MacroView::renameSelectedIcon() {
  if (selection) {
    ItemIcon *ii = dynamic_cast<ItemIcon *>(selection);
    if (ii) {
      ii->renameIcon();
      c->update();
    }
  }
}

void MacroView::deleteSelectedIcon() {
  if (selection) {
    ItemIcon *ii = dynamic_cast<ItemIcon *>(selection);
    if (ii) {
      setSelection(0);
      ii->deleteIcon();
      c->update();
    }
  }
}

void MacroView::newControl() {
  if (Controller::have_active_instance()) {
    Controller *c = Controller::active_instance();
    Registry *styleReg = c->getParent();
    std::string styleName = styleReg->getLocalname();

    Registrable *pluginClassReg =
      Registry::root->lookup(std::string("ControllerFactory/") + styleName);
    if (pluginClassReg == 0) {
      qWarning("No controller plugin for style %s", styleName.c_str());
      return;
    }

    createPrimitive(pluginClassReg, c->getLocalname());
  }
}

void MacroView::createPrimitive(Registrable const *generatorClass) {
  createPrimitive(generatorClass, generatorClass->getLocalname());
}

void MacroView::createPrimitive(Registrable const *generatorClass,
				std::string const &name)
{
  IFDEBUG(cerr << "createPrimitive: fullpath " << generatorClass->getFullpath() << endl);

  GeneratorClass *cls = dynamic_cast<GeneratorClass *>(const_cast<Registrable *>(generatorClass));
  if (cls == 0) {
    qWarning("Non-GeneratorClass found in Registry! Ignoring createPrimitive request.");
    return;
  }

  Generator *prim = new Generator(*cls, true /*%%%*/, 1 /*%%%*/);

  int counter = 1;
  std::string newName = name;

 retry:
  if (!macro->addChild(newName, prim)) {
    qWarning("Name %s already taken! Eeek", newName.c_str());

    ostrstream n;
    n << name << '-' << ++counter << ends;
    newName = n.str();

    goto retry;
  }

  ItemIcon *ii = new ItemIcon(macro, focusPos, newName, c);

  QString msg;
  msg.sprintf("Created primitive %s.", newName.c_str());
  MainWin::StatusBar()->message(msg);

  setSelection(ii);
  c->update();
}

static bool canLinkFrom(QCanvasItem *item) {
  if (item->rtti() != ItemIcon::RTTI)
    return false;

  ItemIcon *ii = dynamic_cast<ItemIcon *>(item);
  if (ii->getGenerator()->getClass().getOutputs().empty()) {
    MainWin::StatusBar()->message("Can't create link from item with no outputs!");
    return false;
  }

  return true;
}

void MacroView::contentsMousePressEvent(QMouseEvent *evt) {
  switch (editState) {
    case IDLE: {
      Qt::ButtonState button = evt->button();
      Qt::ButtonState state = evt->state();

      if (button == MidButton) {
	// Drag the view around.
	moveOffset = QPoint(contentsX() + evt->globalX(),
			    contentsY() + evt->globalY());
	setCursor(SizeAllCursor);
	editState = DRAGGING_VIEW;
      } else if ((button == LeftButton) && (state & ShiftButton)) {
	// Connect objects - either basic or detailed (without/with ctrl)
	setSelection(topItemAt(evt->pos()));
	if (selection && canLinkFrom(selection)) {
	  QPoint startPoint = selection->boundingRect().center();
	  linkLine = new QCanvasLine(c);
	  linkLine->setPoints(startPoint.x(), startPoint.y(), evt->x(), evt->y());
	  linkLine->setPen(Qt::white);
	  linkLine->setZ(LINK_HEIGHT);
	  linkLine->show();
	  editState = DRAGGING_LINK;
	}
      } else if (((button == LeftButton) && (state & ControlButton)) ||
		 (button == RightButton)) {
	// Popup menu. (C-left or ANY-right)

	// Clear selection first (the selected icon may be deleted by
	// a menu action).
	setSelection(0);

	// Now run the menu.
	popupMenu(evt);

	// Update the display, since a menu action may have altered
	// it.
	c->update();

      } else if (button == LeftButton) {
	// Plain old left-click (shift and control are filtered out
	// above).
	setSelection(topItemAt(evt->pos()));
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
      focusPos = evt->pos();
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
      setCursor(ArrowCursor);
      editState = IDLE;
      break;

    default:
      g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
	    "Unknown MacroView EditState! Ignoring release event...");
      break;
  }
}
