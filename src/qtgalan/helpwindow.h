/* -*- C++ -*- */
#ifndef HelpWindow_H
#define HelpWindow_H

#include <string>

#include <qmainwindow.h>
#include <qpopupmenu.h>

class HelpWindow: public QMainWindow {
  Q_OBJECT
public:
  HelpWindow(std::string const &startLocation = std::string(),
	     QWidget *parent = 0);

private slots:
  void setBackEnabled(bool on);
  void setForwardEnabled(bool on);

private:
  QPopupMenu *goMenu;
  int backItemId;
  int forwardItemId;
};

#endif
