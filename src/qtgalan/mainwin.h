/* -*- C++ -*- */
#ifndef MainWin_H
#define MainWin_H

#include "galan/macro.h"

#include <qmainwindow.h>
#include <qmenubar.h>
#include <qstatusbar.h>

class MainWin: public QMainWindow {
  Q_OBJECT
public:
  static QStatusBar *StatusBar();

  MainWin();
  virtual ~MainWin();

protected:
  void closeEvent(QCloseEvent *evt);

private slots:
  void about();
  void aboutPlugins();
  void selectClock();
  void selectionChanged();

private:
  static MainWin *instance;

  Galan::Macro *root;
};

#endif
