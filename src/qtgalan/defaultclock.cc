#include "defaultclock.h"

GALAN_USE_NAMESPACE

DefaultClock::DefaultClock()
  : Clock(), timer()
{
  ClockManager *cm = ClockManager::instance();
  cm->register_clock(this);
  cm->set_default_clock(this);
  cm->select_clock(this);

  connect(&timer, SIGNAL(timeout()), this, SLOT(timedOut()));
}

DefaultClock::~DefaultClock() {
  ClockManager *cm = ClockManager::instance();
  cm->set_default_clock(NULL);
  cm->stop_clock();
}

string const &DefaultClock::getName() const {
  static string result("Default Clock");
  return result;
}

void DefaultClock::disable() {
  IFDEBUG(cerr << "Disabling DefaultClock" << endl);
  timer.stop();
}

void DefaultClock::enable() {
  IFDEBUG(cerr << "Enabling DefaultClock" << endl);
  timer.start(1000 / (Sample::rate / Clock::max_step()), false);
}

void DefaultClock::timedOut() {
  //IFDEBUG(cerr << "DefaultClock timeout_handler running" << endl);
  Clock::advance(Event::mainloop());
}
