#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <vector>

#include "config.h"

#include "mainwin.h"
#include "macroview.h"
#include "controlpanel.h"
#include "helpwindow.h"

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
#include <qstatusbar.h>
#include <qtabwidget.h>

#include "galan/global.h"
#include "galan/clock.h"
#include "galan/plugin.h"

GALAN_USE_NAMESPACE
using namespace std;

MainWin *MainWin::instance = 0;

QStatusBar *MainWin::StatusBar() {
  return instance->statusBar();
}

MainWin::MainWin()
  : QMainWindow(0, 0, WDestructiveClose),
    root(Macro::create(true))
{
  if (instance != 0) {
    throw std::logic_error("Already created instance of MainWin!");
  }
  instance = this;

  QPopupMenu *fileMenu = new QPopupMenu(this);
  fileMenu->insertItem("&New workspace", 0, ""); // %%% clear main
	// macroview, delete aux. windows and controls - check if
	// modified first though!
  fileMenu->insertItem("&Open workspace...", 0, "", CTRL+Key_O);	// %%% need unpickler: xml?
  fileMenu->insertSeparator();
  fileMenu->insertItem("&Save workspace", 0, "", CTRL+Key_S);		// %%% need pickler: xml?
  fileMenu->insertItem("Save workspace &as...", 0, "");			// %%% need pickler: xml?
  fileMenu->insertSeparator();
  fileMenu->insertItem("&Close", this, SLOT(close()), CTRL+Key_W);
  fileMenu->insertItem("E&xit", qApp, SLOT(quit()), CTRL+Key_Q);

  QPopupMenu *editMenu = new QPopupMenu(this);
  editMenu->insertItem("&Undo", 0, "", CTRL+Key_Z);	// %%% depends on selection. Multi-undo?
  editMenu->insertSeparator();
  editMenu->insertItem("Cu&t", 0, "", CTRL+Key_X);	// %%% depends on selection.
  editMenu->insertItem("&Copy", 0, "", CTRL+Key_C);	// %%% depends on selection.
  editMenu->insertItem("&Paste", 0, "", CTRL+Key_V);	// %%% depends on selection.
  editMenu->insertItem("De&lete", 0, "", Key_Delete);	// %%% depends on selection.
  editMenu->insertSeparator();
  editMenu->insertItem("&Preferences...", 0, "");	// %%% new class for Prefs editing

  QPopupMenu *windowMenu = new QPopupMenu(this);
  windowMenu->insertItem("Show &Pattern Panel", 0, "");	// %%% new class for pattern panel (?)
  windowMenu->insertItem("Hide Patter&n Panel", 0, "");

  QPopupMenu *timingMenu = new QPopupMenu(this);
  timingMenu->insertItem("&Select master clock...", this, SLOT(selectClock()));

  QPopupMenu *helpMenu = new QPopupMenu(this);
  helpMenu->insertItem("&Contents...", this, SLOT(helpContents()), Key_F1);
  helpMenu->insertSeparator();
  helpMenu->insertItem("&About", this, SLOT(about()));
  helpMenu->insertItem("About &plugins...", this, SLOT(aboutPlugins()));

  menuBar()->insertItem("&File", fileMenu);
  menuBar()->insertItem("&Edit", editMenu);
  menuBar()->insertItem("&Window", windowMenu);
  menuBar()->insertItem("&Timing", timingMenu);
  menuBar()->insertSeparator();
  menuBar()->insertItem("&Help", helpMenu);	// %%% need more and better help and tooltips

  QTabWidget *tabs = new QTabWidget(this);
  setCentralWidget(tabs);

  MacroView *macroView = new MacroView(root, tabs);
  tabs->addTab(macroView, "&Graph");

  ControlPanel *cp = new ControlPanel(tabs);
  tabs->addTab(cp, "&Controls");

  connect(macroView, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));

  setMinimumSize(400, 400);

  StatusBar()->message("Ready.");
}

MainWin::~MainWin() {
  // Don't delete root here. This causes the generators inside the
  // macro to be removed, and then when it comes time to delete the
  // main QCanvas, and all the ItemIcon objects contained within it,
  // they complain (fatally) that the Generator they represent is no
  // longer present. And fair enough, too.
  //
  //delete root;
}

void MainWin::closeEvent(QCloseEvent *evt) {
  // %%% Should check to see if file is saved here.
  evt->accept();
}

void MainWin::helpContents() {
  (new HelpWindow())->show();
}

void MainWin::about() {
  QString msg;
  msg.sprintf("gAlan %s\n"
	      "Copyright Tony Garnock-Jones (C) 1999-2002\n"
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

void MainWin::selectionChanged() {
  cerr << "Selection changed!" << endl;	// %%% need to do something sensible here
}
