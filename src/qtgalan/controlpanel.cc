#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "controlpanel.h"
#include "mainwin.h"
#include "regsel.h"

#include "galan/global.h"
#include "galan/controller.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qinputdialog.h>
#include <qpainter.h>

GALAN_USE_NAMESPACE
using namespace std;

///////////////////////////////////////////////////////////////////////////

ControlFactory::ControlFactory(std::string const &name) {
  std::string fullname = "ControlFactory/" + name;

  if (!Registry::root->bind(fullname, this)) {
    qWarning((fullname + " name already in use").c_str());
  }
};

///////////////////////////////////////////////////////////////////////////

class ControlHandle: public QWidget {
public:
  ControlHandle(QScrollView *parent,
		std::string const &_label,
		Galan::Controller *_controller,
		QWidget *_buddy,
		QPoint pos)
    : QWidget(parent->viewport()),
      label(_label),
      scrollView(parent),
      controller(_controller),
      buddy(_buddy)
  {
    scrollView->addChild(this, pos.x(), pos.y());
    scrollView->addChild(buddy, pos.x(), pos.y() + HEIGHT);
    resize(sizeHint());
    show();
    buddy->show();
  }

  void moveTo(int x, int y) {
    scrollView->moveChild(this, x, y);
    scrollView->moveChild(buddy, x, y + HEIGHT);
  }

  void moveBy(int dx, int dy) {
    moveTo(scrollView->childX(this) + dx,
	   scrollView->childY(this) + dy);
  }

  QSize sizeHint() { return QSize(buddy->width(), HEIGHT); }
  QSizePolicy sizePolicy() {
    return QSizePolicy(QSizePolicy::Fixed,
		       QSizePolicy::Fixed);
  }

  void paintEvent(QPaintEvent *evt) {
    QPainter p(this);

    p.setPen(Qt::NoPen);
    if (Controller::have_active_instance()) {
      p.setBrush((Controller::active_instance() == controller)
		 ? Qt::red
		 : Qt::black);
    } else {
      p.setBrush(Qt::blue);
    }
    p.drawRect(rect());

    p.setBrush(Qt::NoBrush);
    p.setPen(Qt::white);

    // %%% Configuration item? (see comment in ItemIcon::drawShape(QPainter&))
    QFont f("System");
    f.setPixelSize(HEIGHT - 2);
    p.setFont(f);

    p.drawText(2, 0,
	       width()  - 2, height(),
	       AlignLeft | AlignVCenter, label.c_str());
  }

  void mousePressEvent(QMouseEvent *evt) {
    if (Controller::have_active_instance())
      Controller::active_instance()->deactivate();
    controller->activate();
    update();

    dragOrigin = evt->globalPos() - QPoint(scrollView->childX(this),
					   scrollView->childY(this));
    raise();
    buddy->raise();
  }

  void mouseMoveEvent(QMouseEvent *evt) {
    QPoint g = evt->globalPos();
    QPoint tl = scrollView->mapToGlobal(scrollView->rect().topLeft());
    QPoint br = scrollView->mapToGlobal(scrollView->rect().bottomRight());

    int dx = 0;
    int dy = 0;
    if (g.x() < tl.x() + 10) dx -= 10;
    if (g.y() < tl.y() + 10) dy -= 10;
    if (g.x() > br.x() - 10) dx += 10;
    if (g.y() > br.y() - 10) dy += 10;

    if (dx || dy) {
      QPoint oldScrollLoc(scrollView->contentsX(),
			  scrollView->contentsY());
      scrollView->scrollBy(dx, dy);
      QPoint newScrollLoc(scrollView->contentsX(),
			  scrollView->contentsY());
      dragOrigin = dragOrigin - (newScrollLoc - oldScrollLoc);
    }

    QPoint d = g - dragOrigin;
    moveTo(max(0,
	       min(scrollView->contentsWidth() - HEIGHT,
		   d.x())),
	   max(0,
	       min(scrollView->contentsHeight() - HEIGHT,
		   d.y())));
  }

private:
  static int const HEIGHT = 10;

  std::string label;
  QScrollView *scrollView;
  Controller *controller;
  QWidget *buddy;

  QPoint dragOrigin;
};

///////////////////////////////////////////////////////////////////////////

class ControlScroller: public QScrollView {
public:
  ControlScroller(ControlPanel *parent)
    : QScrollView(parent),
      panel(parent)
  {}

  virtual void contentsMousePressEvent(QMouseEvent *evt) {
    Qt::ButtonState button = evt->button();
    Qt::ButtonState state = evt->state();

    if (((button == LeftButton) && (state & ControlButton)) ||
	(button == RightButton)) {
      // Popup menu. (C-left or ANY-right)
      panel->popupPanelMenu(evt);
    }
  }

private:
  ControlPanel *panel;
};

///////////////////////////////////////////////////////////////////////////

ControlPanel::ControlPanel(QWidget *parent)
  : QWidget(parent),
    scrollView(0)
{
  scrollView = new ControlScroller(this);

  QVBoxLayout *vbl = new QVBoxLayout(this);
  vbl->addWidget(scrollView);

  scrollView->resizeContents(2048, 2048);
  scrollView->enableClipper(true);
}

ControlPanel::~ControlPanel() {
}

void ControlPanel::popupPanelMenu(QMouseEvent *evt) {
  popupPos = evt->pos();

  Registrable *factoryList = Registry::root->lookup("ControlFactory");
  if (factoryList == 0) {
    qWarning("No UI control factories registered (under /ControlFactory)");
    return;
  }

  RegistrySelectionMenu menu(factoryList->toRegistry(), this);
  menu.setItemEnabled(menu.insertItem("New control", -1, 0), false);
  menu.insertSeparator(1);

  connect(&menu, SIGNAL(itemSelected(const Galan::Registrable*)),
	  this, SLOT(buildUIControl(const Galan::Registrable*)));

  menu.exec(evt->globalPos(), 2);
}

void ControlPanel::buildUIControl(const Galan::Registrable *maybeFactory) {
  ControlFactory const *factory = dynamic_cast<ControlFactory const *>(maybeFactory);

  if (factory == 0) {
    qWarning("Eeek! How did a non-ControlFactory come to be registered in /ControlFactory?");
    return;
  }

  QString prompt;
  prompt.sprintf("Please enter a name for the new control:");

 retry:
  bool ok = false;
  QString response = QInputDialog::getText("Name Control",
					   prompt,
					   QLineEdit::Normal,
					   "",
					   &ok);

  if (ok && !response.isEmpty()) {
    std::string name = response.latin1();

    Controller *c = 0;
    QWidget *b = 0;

    try {
      factory->construct(scrollView->viewport(), name, c, b);
    } catch (std::range_error &e) {
      prompt.sprintf("The name %s was already taken. Please try again.\n"
		     "Please enter a name for the new control:",
		     name.c_str());
      goto retry;
    }

    if (Controller::have_active_instance())
      Controller::active_instance()->deactivate();

    c->activate();
    MainWin::allowNewControl(true);

    new ControlHandle(scrollView, name,
		      c,
		      b,
		      popupPos);
  }
}
