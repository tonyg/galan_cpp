#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <vector>

#include "config.h"

#include "mainwin.h"
#include "popupdialog.h"

#include <qpopupmenu.h>
#include <qkeycode.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qlistbox.h>

#include "galan/global.h"
#include "galan/clock.h"

GALAN_USE_NAMESPACE
using namespace std;

MainWin::MainWin()
  : QMainWindow()
{
  QPopupMenu *fileMenu = new QPopupMenu(this);
  fileMenu->insertItem("E&xit", qApp, SLOT(quit()), CTRL+Key_Q);

  QPopupMenu *editMenu = new QPopupMenu(this);

  QPopupMenu *windowMenu = new QPopupMenu(this);

  QPopupMenu *timingMenu = new QPopupMenu(this);
  timingMenu->insertItem("&Select master clock...", this, SLOT(selectClock()));

  QPopupMenu *helpMenu = new QPopupMenu(this);
  helpMenu->insertItem("&About", this, SLOT(about()));

  menuBar()->insertItem("&File", fileMenu);
  menuBar()->insertItem("&Edit", editMenu);
  menuBar()->insertItem("&Window", windowMenu);
  menuBar()->insertItem("&Timing", timingMenu);
  menuBar()->insertSeparator();
  menuBar()->insertItem("&Help", helpMenu);

  setMinimumSize(400, 400);
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

void MainWin::selectClock() {
  QListBox *lb = new QListBox();

  ClockManager *clockManager = ClockManager::instance();
  vector<Clock *> v(clockManager->begin(), clockManager->end());

  for (unsigned int i = 0; i < v.size(); i++) {
    Clock *clock = v[i];
    lb->insertItem(clock->getName().c_str());
    if (clock == clockManager->getSelected()) {
      lb->setCurrentItem(i);
    }
  }

  PopupDialog dlg("Select Master Clock",
		  PopupDialog::OK | PopupDialog::CANCEL,
		  0, PopupDialog::NONE, lb);

  dlg.resize(200, 150);

  switch (dlg.runPopup()) {
    case PopupDialog::OK: {
      int item = lb->currentItem();
      if (item != -1) {
	Clock *clock = v[item];
	IFDEBUG(cerr << "New master clock: " << clock->getName() << endl);
	clockManager->select_clock(clock);
      } else {
	IFDEBUG(cerr << "No master clock selected in dialog. Ignoring." << endl);
      }
      break;
    }

    case PopupDialog::CANCEL:
      IFDEBUG(cerr << "Cancelled Master Clock selection." << endl);
      /* FALL THROUGH */

    default:
      break;
  }
}
