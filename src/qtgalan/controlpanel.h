/* -*- C++ -*- */
#ifndef ControlPanel_H
#define ControlPanel_H

#include "galan/controller.h"
#include "galan/registry.h"

#include <qscrollview.h>

/**
 * Subclass this if you wish to provide a way of constructing UI
 * controls.
 **/
class ControlFactory: public Galan::Registrable {
public:
  /**
   * Constructs a control factory, binding it to the given name under
   * the "ControlFactory/" registry entry.
   *
   * @param name path fragment to append to "ControlFactory/" when binding
   **/
  ControlFactory(std::string const &name);

  /**
   * Implemented in subclasses to produce both a Galan::Controller and
   * a QWidget for interfacing to it.
   *
   * @param parent the QWidget that will own the new widget
   * @param name leaf name to use for display and registration of the new controller
   * @param controller (out) the newly-created controller
   * @param buddy (out) the newly-created widget
   **/
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
