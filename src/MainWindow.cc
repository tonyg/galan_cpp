#include "config.h"
#include "MainWindow.h"

using namespace Gtk;

MainWindow::MainWindow()
  : Window(GTK_WINDOW_TOPLEVEL),
    menu(),
    status(),
    fixed()
{
  set_title("gAlan " VERSION);
  set_position(GTK_WIN_POS_CENTER);
  set_usize(500, 400);
  set_policy(true, true, false);
  set_wmclass("gAlan_Controls", "gAlan");

  {
    using namespace Menu_Helpers;

    Menu *menu_file = manage(new Menu());
    MenuList &list_file = menu_file->items();

    list_file.push_back(MenuElem("E_xit", "<control>q"));
    list_file.back()->activate.connect(slot(*this, &MainWindow::shutdown_check));

    menu.items().push_front(MenuElem("_File", "<alt>f", *menu_file));
  }

  HandleBox *hb = manage(new HandleBox());
  hb->add(menu);

  ScrolledWindow *sw = manage(new ScrolledWindow());
  sw->add_with_viewport(fixed);

  VPaned *vp = manage(new VPaned());
  vp->add2(*sw);

  VBox *vb = manage(new VBox());
  vb->pack_start(*hb, false, true);
  vb->pack_start(*vp, true, true);
  vb->pack_end(status, false, false);

  add(*vb);

  show_all();

  if (!fixed.is_realized())
    fixed.realize();
  fixed.get_window().set_events(GDK_ALL_EVENTS_MASK);
}

void MainWindow::shutdown_check() {
  // Check that we can quit OK, and if so, quit.
  // %%% should check that the file is saved here
  Gtk::Main::quit();
}

int MainWindow::delete_event_impl(GdkEventAny *event) {
  shutdown_check();
  return true;
}
