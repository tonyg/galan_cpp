#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "helpwindow.h"

#include "galan/global.h"

#include <qtextbrowser.h>
#include <qtoolbar.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>

GALAN_USE_NAMESPACE
using namespace std;

static string const homeLocation(SITE_PKGDATA_DIR "/manual/index.html");

HelpWindow::HelpWindow(std::string const &startLocation,
		       QWidget *parent)
  : QMainWindow(parent)
{
  setCaption("QtGalan Help Browser");
  resize(600, 400);

  QTextBrowser *tb = new QTextBrowser(this);
  setCentralWidget(tb);

  tb->setSource(homeLocation.c_str());
  if (startLocation.size()) {
    tb->setSource(startLocation.c_str());
  }

  QToolBar *toolBar = new QToolBar("Browser Controls", this);
  QPushButton *backButton = new QPushButton("Back", toolBar);
  QPushButton *forwardButton = new QPushButton("Forward", toolBar);
  QPushButton *homeButton = new QPushButton("Home", toolBar);

  connect(backButton, SIGNAL(clicked()), tb, SLOT(backward()));
  connect(forwardButton, SIGNAL(clicked()), tb, SLOT(forward()));
  connect(homeButton, SIGNAL(clicked()), tb, SLOT(home()));

  backButton->setEnabled(false);
  forwardButton->setEnabled(false);
  connect(tb, SIGNAL(backwardAvailable(bool)), backButton, SLOT(setEnabled(bool)));
  connect(tb, SIGNAL(forwardAvailable(bool)), forwardButton, SLOT(setEnabled(bool)));

  QPopupMenu *fileMenu = new QPopupMenu(this);
  fileMenu->insertItem("&Close", this, SLOT(close()), CTRL+Key_W);

  goMenu = new QPopupMenu(this);
  backItemId = goMenu->insertItem("&Back", tb, SLOT(backward()), ALT+Key_Left);
  forwardItemId = goMenu->insertItem("&Forward", tb, SLOT(forward()), ALT+Key_Right);
  goMenu->insertSeparator();
  goMenu->insertItem("&Home", tb, SLOT(home()), ALT+Key_Home);

  goMenu->setItemEnabled(backItemId, false);
  goMenu->setItemEnabled(forwardItemId, false);
  connect(tb, SIGNAL(backwardAvailable(bool)), this, SLOT(setBackEnabled(bool)));
  connect(tb, SIGNAL(forwardAvailable(bool)), this, SLOT(setForwardEnabled(bool)));

  QPopupMenu *helpMenu = new QPopupMenu(this);
  helpMenu->insertItem("&Contents...", tb, SLOT(home()), Key_F1);

  menuBar()->insertItem("&File", fileMenu);
  menuBar()->insertItem("&Go", goMenu);
  menuBar()->insertSeparator();
  menuBar()->insertItem("&Help", helpMenu);
}

void HelpWindow::setBackEnabled(bool on) {
  goMenu->setItemEnabled(backItemId, on);
}

void HelpWindow::setForwardEnabled(bool on) {
  goMenu->setItemEnabled(forwardItemId, on);
}
