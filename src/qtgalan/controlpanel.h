/* -*- C++ -*- */
#ifndef ControlPanel_H
#define ControlPanel_H

#include "galan/controller.h"

#include <qscrollview.h>

class ControlPanel: public QWidget {
  Q_OBJECT
public:
  ControlPanel(QWidget *parent = 0);
  virtual ~ControlPanel();

private:
  QScrollView *scrollView;
};

#endif
