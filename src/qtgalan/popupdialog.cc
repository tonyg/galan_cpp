#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <iostream>
#include "galan/global.h"

#include "popupdialog.h"

#include <qpushbutton.h>
#include <qtimer.h>
#include <qlayout.h>
#include <qhbox.h>

PopupDialog::PopupDialog(char const *title, int buttons,
			 int timeout_millis, Response def,
			 QWidget *contents)
  : QDialog(0, title, true)
{
  IFDEBUG(cerr << "Creating PopupDialog called '" << title << "'" << endl);
  setCaption(title);

  this->def = def;
  this->result = PopupDialog::NONE;

  QBoxLayout *vbox = new QVBoxLayout(this);

  if (contents) {
    QWidget *w = new QWidget(this);
    contents->reparent(w, 0, QPoint(), TRUE);
    vbox->addWidget(w, 10);
  }

  QHBox *hbox = new QHBox(this);
  vbox->addWidget(hbox);

  QPushButton *b;

#define BUILDBUTTON(_e_,_n_,_a_)					\
  if (buttons & PopupDialog::_e_) {					\
    b = new QPushButton(_n_, hbox);					\
    connect(b, SIGNAL(clicked()), this, SLOT(_a_ ## Clicked()));	\
  }

  BUILDBUTTON(OK,"Ok",ok);
  BUILDBUTTON(ACCEPT,"Accept",accept);
  BUILDBUTTON(CANCEL,"Cancel",cancel);
  BUILDBUTTON(DISMISS,"Dismiss",dismiss);
  BUILDBUTTON(YES,"Yes",yes);
  BUILDBUTTON(NO,"No",no);

  if (timeout_millis > 0) {
    QTimer::singleShot(timeout_millis, this, SLOT(timedOut()));
  }

  vbox->activate();
}

PopupDialog::Response PopupDialog::runPopup() {
  //show();
  exec();
  return result;
}

#define DEFHANDLER(_a_,_e_)			\
  void PopupDialog::_a_ ## Clicked() {		\
    result = PopupDialog::_e_;			\
    accept();					\
  }

DEFHANDLER(ok,OK);
DEFHANDLER(accept,ACCEPT);
DEFHANDLER(cancel,CANCEL);
DEFHANDLER(dismiss,DISMISS);
DEFHANDLER(yes,YES);
DEFHANDLER(no,NO);

void PopupDialog::timedOut() {
  result = def;
  accept();
}
