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

int main(int argc, char *argv[]) {
  printf("gAlan version " VERSION ", Copyright (C) 1999-2001 Tony Garnock-Jones\n"
	 "gAlan comes with ABSOLUTELY NO WARRANTY; for details, see the file\n"
	 "\"COPYING\" that came with the gAlan distribution.\n"
	 "This is free software, distributed under the GNU General Public\n"
	 "License. Please see \"COPYING\" or http://www.gnu.org/copyleft/gpl.txt\n\n");

  QApplication app(argc, argv);

  DefaultClock defaultClock;

  Macro::initialise();
  QtIOManager::initialise();
  Plugin::loadPlugins();

  MainWin mainwin;
  mainwin.setCaption("QtGalan (" PACKAGE " " VERSION ")");
  mainwin.show();

  app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
  return app.exec();
}
