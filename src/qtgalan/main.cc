#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

#include "galan/global.h"

#include "galan/model.h"
#include "galan/generator.h"
#include "galan/clock.h"
#include "galan/event.h"
#include "galan/plugin.h"
#include "galan/macro.h"

#include <qapplication.h>
#include <qtimer.h>

#include "mainwin.h"
#include "qtio.h"
#include "defaultclock.h"

GALAN_USE_NAMESPACE

#include "controlpanel.h"

class MyWidget: public QWidget {
public:
  MyWidget(QWidget *parent, Galan::RealtimeController *_c)
    : QWidget(parent),
      c(_c),
      freq(110)
  {
    resize(sizeHint());
  }

  virtual QSize sizeHint() { return QSize(128, 32); }
  virtual QSizePolicy sizePolicy() {
    return QSizePolicy(QSizePolicy::Fixed,
		       QSizePolicy::Fixed);
  }

  virtual void mousePressEvent(QMouseEvent *evt) {
    mouseMoveEvent(evt);
  }

  virtual void mouseMoveEvent(QMouseEvent *evt) {
    freq = ((double) evt->x() / width());
    Galan::SampleEvent *e = new Galan::SampleEvent(c, Clock::now(), freq);
    e->post();
  }

  virtual void paintEvent(QPaintEvent *evt) {
    QPainter p(this);

    p.setPen(Qt::NoPen);
    p.setBrush(Qt::blue);
    p.drawRect(rect());

    p.setBrush(Qt::NoBrush);
    p.setPen(Qt::white);
    p.drawText(rect(), AlignCenter, "whee");
  }

private:
  Galan::RealtimeController *c;
  double freq;
};

class MyFactory: public ControlFactory {
public:
  MyFactory()
    : ControlFactory("fizzbot")
  {}

  virtual void construct(QWidget *parent,
			 std::string const &name,
			 Galan::Controller *&controller,
			 QWidget *&buddy) const
  {
    Galan::RealtimeController *rtc = new RealtimeController(name);
    controller = rtc;
    buddy = new MyWidget(parent, rtc);
  }
};

int main(int argc, char *argv[]) {
  try {
    printf("gAlan version " VERSION ", Copyright (C) 1999-2003 Tony Garnock-Jones\n"
	   "gAlan comes with ABSOLUTELY NO WARRANTY; for details, see the file\n"
	   "\"COPYING\" that came with the gAlan distribution.\n"
	   "This is free software, distributed under the GNU General Public\n"
	   "License. Please see \"COPYING\" or http://www.gnu.org/copyleft/gpl.txt\n\n");

    QApplication app(argc, argv);

    DefaultClock defaultClock;

    Macro::initialise();
    QtIOManager::initialise();
    Plugin::loadPlugins();

    new MyFactory();

    // Must be a pointer here, as WDestructiveClose is set in
    // MainWin's constructor, which causes delete to be called on
    // window close.
    MainWin *mainwin = new MainWin();
    mainwin->setCaption("QtGalan (" PACKAGE " " VERSION ")");
    mainwin->show();

    app.connect(mainwin, SIGNAL(destroyed()), &app, SLOT(closeAllWindows()));
    app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));

    return app.exec();

  } catch (std::runtime_error &e) {
    cerr << "Uncaught std::runtime_error: " << e.what() << endl;
  } catch (std::logic_error &e) {
    cerr << "Uncaught std::logic_error: " << e.what() << endl;
  } catch (std::exception &e) {
    cerr << "Uncaught std::exception: " << e.what() << endl;
  } catch (...) {
    cerr << "Uncaught unknown exception!" << endl;
  }
}
