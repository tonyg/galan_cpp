#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <vector>

#include "config.h"

#include "mainwin.h"
#include "macroview.h"
#include "itemicon.h"
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
#include <qaccel.h>

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

  QTabWidget *tabs = new QTabWidget(this);
  connect(tabs, SIGNAL(currentChanged(QWidget*)), this, SLOT(tabChanged(QWidget*)));

  macroView = new MacroView(root, tabs);

  QPopupMenu *fileMenu = new QPopupMenu(this);
  fileMenu->insertItem("&New workspace", 0, ""); // %%% clear main
	// macroview, delete aux. windows and controls - check if
	// modified first though!
  fileMenu->insertItem("&Open workspace...", 0, "", CTRL+Key_O);	// %%% need unpickler: xml?
  fileMenu->insertSeparator();
  fileMenu->insertItem("&Save workspace", 0, "", CTRL+Key_S);		// %%% need pickler: xml?
  fileMenu->insertItem("Save workspace &as...", 0, "");			// %%% need pickler: xml?
  fileMenu->insertSeparator();
  fileMenu->insertItem("&Import bundle...", 0, "");
  fileMenu->insertItem("&Export bundle...", 0, "");
  fileMenu->insertSeparator();
  fileMenu->insertItem("E&xit", qApp, SLOT(quit()), CTRL+Key_Q);

  editMenu = new QPopupMenu(this);
  editMenu->insertItem("&Undo", 0, "", CTRL+Key_Z);	// %%% depends on selection. Multi-undo?
  editMenu->insertSeparator();
  cutMenuItem = editMenu->insertItem("Cu&t", 0, "", CTRL+Key_X);	//%%%
  copyMenuItem = editMenu->insertItem("&Copy", 0, "", CTRL+Key_C);	//%%%
  pasteMenuItem = editMenu->insertItem("&Paste", 0, "", CTRL+Key_V);	//%%%
  editMenu->insertSeparator();
  muteMenuItem = editMenu->insertItem("&Mute", macroView, SLOT(muteSelectedIcon()),
				      CTRL+Key_M);
  renameMenuItem = editMenu->insertItem("&Rename...", macroView, SLOT(renameSelectedIcon()),
					Key_F2);
  deleteMenuItem = editMenu->insertItem("De&lete", macroView, SLOT(deleteSelectedIcon()),
					Key_Delete);
  editMenu->insertSeparator();
  newControlMenuItem = editMenu->insertItem("New control", macroView, SLOT(newControl()),
					    CTRL+Key_N);
  editMenu->setItemEnabled(newControlMenuItem, false);
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

  setCentralWidget(tabs);

  tabs->addTab(macroView, "&Graph");

  ControlPanel *cp = new ControlPanel(tabs);
  tabs->addTab(cp, "&Controls");

  connect(macroView, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
  selectionChanged();

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

void MainWin::allowNewControl(bool allow) {
  // There's almost certainly a better way of achieving this. Signals/slots...?
  instance->editMenu->setItemEnabled(instance->newControlMenuItem, allow);
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
	      "Copyright Tony Garnock-Jones (C) 1999-2003\n"
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
  ItemIcon *selection = dynamic_cast<ItemIcon *>(macroView->getSelection());
  bool enabled = (selection != 0);

  editMenu->setItemEnabled(cutMenuItem, false);		//%%%
  editMenu->setItemEnabled(copyMenuItem, false);	//%%%
  editMenu->setItemEnabled(pasteMenuItem, false);	//%%%

  editMenu->setItemEnabled(muteMenuItem, enabled);
  editMenu->setItemChecked(muteMenuItem, enabled && selection->mute());

  editMenu->setItemEnabled(renameMenuItem, enabled);
  editMenu->setItemEnabled(deleteMenuItem, enabled);
}

void MainWin::tabChanged(QWidget *w) {
  if (w == macroView) {
    selectionChanged();
    editMenu->setItemEnabled(newControlMenuItem, Controller::have_active_instance()); //%%%
  } else {
    editMenu->setItemEnabled(muteMenuItem, false);
    editMenu->setItemEnabled(renameMenuItem, false);
    editMenu->setItemEnabled(deleteMenuItem, false);
    editMenu->setItemEnabled(newControlMenuItem, false);
  }
}
