/* -*- C++ -*- */
#ifndef ControlPanel_H
#define ControlPanel_H

#include "galan/controller.h"
#include "galan/registry.h"

#include <qscrollview.h>

class QPushButton;	// in qpushbutton.h

class ControlFactory: public Galan::Registrable {
public:
  ControlFactory(std::string const &name);

  virtual void construct(QWidget *parent,
			 std::string const &name,
			 Galan::Controller *&controller,
			 QWidget *&buddy) const = 0;
};

class ControlPanel: public QWidget {
  Q_OBJECT
public:
  ControlPanel(QWidget *parent = 0);
  virtual ~ControlPanel();

  void popupPanelMenu(QMouseEvent *evt);

private slots:
  void buildUIControl(Galan::Registrable const *maybeFactory);

private:
  QScrollView *scrollView;

  QPoint popupPos;
};

#endif
