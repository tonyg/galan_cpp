#include "clock.h"

sampletime_t Clock::time_now = 0;
sampletime_t Clock::max_realtime_step = 1024;
list<RealtimeHandler *> Clock::realtime_fns;

void Clock::register_realtime_fn(RealtimeHandler *handler) {
  IFDEBUG(cerr << "Registering realtime fn " << ((void *) handler) << endl);
  realtime_fns.push_back(handler);
}

void Clock::deregister_realtime_fn(RealtimeHandler *handler) {
  IFDEBUG(cerr << "Deregistering realtime fn " << ((void *) handler) << endl);
  realtime_fns.remove(handler);
}

sampletime_t Clock::now() {
  return time_now;
}

sampletime_t Clock::max_step() {
  return max_realtime_step;
}

void Clock::advance(sampletime_t remaining) {
  sampletime_t delta;

  while (remaining > 0) {
    list<RealtimeHandler *>::iterator i = realtime_fns.begin();

    delta = MIN(remaining, Event::mainloop());
    remaining -= delta;

    // IFDEBUG(cerr << "Advancing clock by " << delta << endl);
    while (i != realtime_fns.end()) {
      (*i)->realtime_elapsed(delta);
      i++;
    }

    time_now += delta;
  }
}

///////////////////////////////////////////////////////////////////////////

ClockManager *ClockManager::_instance = 0;

ClockManager *ClockManager::instance() {
  if (_instance == 0) {
    _instance = new ClockManager();
  }
  return _instance;
}

void ClockManager::register_clock(Clock *clock) {
  IFDEBUG(cerr << "Registering clock " << clock->getName() << endl);
  all_clocks.push_back(clock);
  notifyViews();
}

void ClockManager::deregister_clock(Clock *clock) {
  IFDEBUG(cerr << "Deregistering clock " << clock->getName() << endl);
  if (active == clock)
    select_clock(default_clock);
  all_clocks.remove(clock);
  notifyViews();
}

void ClockManager::select_clock(Clock *clock) {
  if (active == clock)
    return;

  if (active != NULL)
    active->disable();

  IFDEBUG(cerr << "Clock " << (clock ? clock->getName() : "<none>") << " selected" << endl);
  active = clock;

  if (active != NULL)
    active->enable();
}

void ClockManager::set_default_clock(Clock *clock) {
  IFDEBUG(cerr << "Default clock is now " << (clock ? clock->getName() : "<none>") << endl);
  default_clock = clock;
}

void ClockManager::stop_clock() {
  select_clock(NULL);
}
