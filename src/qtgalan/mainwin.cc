#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <vector>

#include "config.h"

#include "mainwin.h"
#include "macroview.h"

#include "PluginInfo.h"
#include "SelectClock.h"

#include <qpopupmenu.h>
#include <qkeycode.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>

#include "galan/global.h"
#include "galan/clock.h"
#include "galan/plugin.h"

GALAN_USE_NAMESPACE
using namespace std;

MainWin::MainWin()
  : QMainWindow(),
    root(Macro::create(true))
{
  QPopupMenu *fileMenu = new QPopupMenu(this);
  fileMenu->insertItem("&New", 0, "");
  fileMenu->insertItem("&Open...", 0, "", CTRL+Key_O);
  fileMenu->insertSeparator();
  fileMenu->insertItem("&Save", 0, "", CTRL+Key_S);
  fileMenu->insertItem("Save &as...", 0, "");
  fileMenu->insertSeparator();
  fileMenu->insertItem("E&xit", qApp, SLOT(quit()), CTRL+Key_Q);

  QPopupMenu *editMenu = new QPopupMenu(this);
  editMenu->insertItem("&Preferences...", 0, "");

  QPopupMenu *windowMenu = new QPopupMenu(this);
  windowMenu->insertItem("&Show Control Panel", 0, "");
  windowMenu->insertItem("&Hide Control Panel", 0, "");
  windowMenu->insertItem("Show &Pattern Panel", 0, "");
  windowMenu->insertItem("Hide Patter&n Panel", 0, "");

  QPopupMenu *timingMenu = new QPopupMenu(this);
  timingMenu->insertItem("&Select master clock...", this, SLOT(selectClock()));

  QPopupMenu *helpMenu = new QPopupMenu(this);
  helpMenu->insertItem("&About", this, SLOT(about()));
  helpMenu->insertItem("About &plugins...", this, SLOT(aboutPlugins()));

  menuBar()->insertItem("&File", fileMenu);
  menuBar()->insertItem("&Edit", editMenu);
  menuBar()->insertItem("&Window", windowMenu);
  menuBar()->insertItem("&Timing", timingMenu);
  menuBar()->insertSeparator();
  menuBar()->insertItem("&Help", helpMenu);

  setCentralWidget(new MacroView(root, this));

  setMinimumSize(400, 400);
}

MainWin::~MainWin() {
  delete root;
}

void MainWin::closeEvent(QCloseEvent *evt) {
  // %%% Should check to see if file is saved here.
  evt->accept();
}

void MainWin::about() {
  QString msg;
  msg.sprintf("gAlan %s\n"
	      "Copyright Tony Garnock-Jones (C) 1999-2001\n"
	      "A modular sound-processing tool\n(Graphical Audio LANguage)\n"
	      "\n"
	      "gAlan comes with ABSOLUTELY NO WARRANTY; for details, see the file\n"
	      "\"COPYING\" that came with the gAlan distribution.\n"
	      "This is free software, distributed under the GNU General Public\n"
	      "License. Please see \"COPYING\" or http://www.gnu.org/copyleft/gpl.txt\n"
	      "\n"
	      "SITE_PKGLIB_DIR = %s\n"
	      "\n"
	      "NOTE: This is BETA software\n",
	      VERSION,
	      SITE_PKGLIB_DIR);

  QMessageBox::about(this, "QtGalan", msg);
}

void MainWin::aboutPlugins() {
  PluginInfo *dlg = new PluginInfoImpl(this, "PluginInfo", true);
  dlg->exec();
}

void MainWin::selectClock() {
  SelectClock *dlg = new SelectClockImpl(this, "SelectClock", true);
  dlg->exec();
}
