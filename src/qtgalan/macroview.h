// -*- C++ -*-
#ifndef galan_MACROVIEW_H
#define galan_MACROVIEW_H

#include "galan/macro.h"

#include <qcanvas.h>
#include <qscrollview.h>
#include <qevent.h>

class MacroView: public QCanvasView {
  Q_OBJECT
public:
  MacroView(Galan::Macro *_macro, QWidget *parent = 0, char const *name = 0, WFlags f = 0);
  virtual ~MacroView();

  /**@name Mouse Event Handlers
   *
   * These functions implement the user interaction with a view of a
   * Galan::Macro.
   *
   * left drag			-->	move item
   * right click		-->	popup menu (for item, link, or entire macro)
   * shift + left click		-->	connect objects
   * ctrl + left click		-->	popup menu
   * ctrl + shift + left click	-->	non-default connection (with detail dialog)
   *
   **/
  //@{
  virtual void contentsMousePressEvent(QMouseEvent *evt);
  virtual void contentsMouseMoveEvent(QMouseEvent *evt);
  virtual void contentsMouseReleaseEvent(QMouseEvent *evt);
  //@}

  enum ItemDepth {
    LINK_HEIGHT = 0,
    ITEM_HEIGHT = 1,
    HALO_HEIGHT = 2,
    SELECTION_HEIGHT = 3
  };

  QCanvasItem *getSelection() const { return selection; }

signals:
  void selectionChanged();

public slots:
  void muteSelectedIcon();
  void renameSelectedIcon();
  void deleteSelectedIcon();
  void newControl();

private slots:
  void createPrimitive(Galan::Registrable const *generatorClass);

private:
  class DynamicTip;	// defined in macroview.cc.
  friend class DynamicTip;

  QCanvasItemList itemsAt(QPoint p);
  QCanvasItem *topItemAt(QPoint p);

  void setSelection(QCanvasItem *sel);
  void updateHalo();

  /**@name Operators
   * These functions perform the actual actions corresponding to user
   * actions.
   **/
  //@{
  void createLink(QMouseEvent *evt);
  void moveItem(QMouseEvent *evt);
  void popupMenu(QMouseEvent *evt);
  //@}

  void createPrimitive(Galan::Registrable const *generatorClass,
		       std::string const &name);

  enum EditState {
    IDLE = 0,		///< User is not interacting with this object
    DRAGGING_LINK,	///< User is dragging out a new connection
    DRAGGING_ITEM,	///< User is moving an item around
    DRAGGING_VIEW	///< User is scrolling the whole view
  };

  Galan::Macro *macro;
  QCanvas *c;

  QCanvasRectangle *halo;	///< Halo around selection
  QCanvasItem *selection;	///< Item currently selected

  QPoint moveOffset;		///< How far off (x,y) are we dragging at?
  QCanvasLine *linkLine;	///< Item representing a link being built

  EditState editState;

  QPoint focusPos;		///< Position (local coords) of the mouse focus
};

#endif
