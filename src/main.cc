#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "global.h"

#include "model.h"
#include "generator.h"
#include "clock.h"
#include "event.h"
#include "plugin.h"
#include "macro.h"

GALAN_USE_NAMESPACE

class DefaultClock: public Clock {
private:
  guint timeout_tag;

public:
  DefaultClock(): Clock(), timeout_tag(0) {
    ClockManager *cm = ClockManager::instance();
    cm->register_clock(this);
    cm->set_default_clock(this);
    cm->select_clock(this);
    cout << "main.cc: ClockManager is " << (void *) cm << endl;
    cout << "main.cc: &Registry::root is " << (void *) &Registry::root << endl;
  }

  virtual ~DefaultClock() {
    ClockManager *cm = ClockManager::instance();
    cm->set_default_clock(NULL);
    cm->stop_clock();
  }

  virtual string const &getName() const {
    static string result("Default Clock");
    return result;
  }

  virtual void disable();
  virtual void enable();
};

void DefaultClock::disable() {
  IFDEBUG(cerr << "Disabling DefaultClock" << endl);
//    gtk_timeout_remove(timeout_tag);
}

static gint timeout_handler(gpointer data) {
  //  IFDEBUG(cerr << "DefaultClock timeout_handler running" << endl);
  Clock::advance(Event::mainloop());
  return TRUE;	// keep this timer running...
}

void DefaultClock::enable() {
  IFDEBUG(cerr << "Enabling DefaultClock" << endl);
//    timeout_tag = gtk_timeout_add(1000 / (Sample::rate / Clock::max_step()),
//  				timeout_handler,
//  				NULL);
}

int main(int argc, char *argv[]) {
  DefaultClock defaultClock;

  Macro::initialise();
  Plugin::loadPlugins();

  return EXIT_SUCCESS;
}
