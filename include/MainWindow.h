#ifndef MainWindow_H /* -*- c++ -*- */
#define MainWindow_H

#include <gtk--.h>

class MainWindow: public Gtk::Window {
private:
  Gtk::MenuBar menu;
  Gtk::Statusbar status;
  Gtk::Fixed fixed;

  void shutdown_check();	// Checks that we can quit OK, and if so, quits.

public:
  MainWindow();

  // Event handlers.
  virtual int delete_event_impl(GdkEventAny *event);
};

#endif
