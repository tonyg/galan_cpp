/* -*- C++ -*- */
#ifndef MainWin_H
#define MainWin_H

#include "galan/macro.h"

#include <qmainwindow.h>
#include <qmenubar.h>

class MainWin: public QMainWindow {
  Q_OBJECT
public:
  MainWin();
  virtual ~MainWin();

protected:
  void closeEvent(QCloseEvent *evt);

private slots:
  void about();
  void aboutPlugins();
  void selectClock();

private:
  Galan::Macro *root;
};

#endif
