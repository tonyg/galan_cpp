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

private:
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

  enum EditState {
    IDLE = 0,		///< User is not interacting with this object
    DRAGGING_LINK,	///< User is dragging out a new connection
    DRAGGING_ITEM,	///< User is moving an item around
    DRAGGING_VIEW	///< User is scrolling the whole view
  };

  enum ItemDepth {
    LINK_HEIGHT = 0,
    ITEM_HEIGHT = 1,
    HALO_HEIGHT = 2,
    SELECTION_HEIGHT = 3
  };

  Galan::Macro *macro;
  QCanvas *c;

  QCanvasRectangle *halo;	///< Halo around selection
  QCanvasItem *selection;	///< Item currently selected

  QPoint moveOffset;		///< How far off (x,y) are we dragging at?
  QCanvasLine *linkLine;	///< Item representing a link being built

  EditState editState;
};

#endif
