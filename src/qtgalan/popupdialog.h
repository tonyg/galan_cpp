// -*- C++ -*-
#ifndef PopupDialog_H
#define PopupDialog_H

#include <qwidget.h>
#include <qdialog.h>

class PopupDialog: public QDialog {
  Q_OBJECT
public:
  enum Response {
    NONE = 0,
    OK = 1,
    ACCEPT = 2,
    CANCEL = 4,
    DISMISS = 8,
    YES = 16,
    NO = 32
  };

  PopupDialog(char const *title, int buttons,
	      int timeout_millis, Response def,
	      QWidget *contents);

  Response runPopup();

private slots:
  void okClicked();
  void acceptClicked();
  void cancelClicked();
  void dismissClicked();
  void yesClicked();
  void noClicked();

  void timedOut();

private:
  Response def;
  Response result;
};

#endif
