/* -*- C++ -*- */
#ifndef MainWin_H
#define MainWin_H

#include "galan/macro.h"

#include <qmainwindow.h>
#include <qmenubar.h>
#include <qstatusbar.h>

class QListViewItem;	// in qlistview.h

class MacroView;	// in macroview.h

class MainWin: public QMainWindow {
  Q_OBJECT
public:
  static QStatusBar *StatusBar();

  MainWin();
  virtual ~MainWin();

  static void allowNewControl(bool allow);

protected:
  void closeEvent(QCloseEvent *evt);

private slots:
  void helpContents();
  void about();
  void aboutPlugins();
  void selectClock();
  void selectionChanged();
  void tabChanged(QWidget*);

private:
  static MainWin *instance;

  Galan::Macro *root;

  MacroView *macroView;
  QPopupMenu *editMenu;
  int cutMenuItem;
  int copyMenuItem;
  int pasteMenuItem;
  int muteMenuItem;
  int renameMenuItem;
  int deleteMenuItem;
  int newControlMenuItem;
};

#endif
