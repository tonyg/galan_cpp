#ifndef Clock_H	/* -*- c++ -*- */
#define Clock_H

///////////////////////////////////////////////////////////////////////////
// DESCRIPTION
// Interface

#include <list>
#include <string>

#include "global.h"
#include "model.h"
#include "sample.h"
#include "event.h"

class RealtimeHandler {
public:
  virtual void realtime_elapsed(sampletime_t delta) = 0;
};

class Clock {
private:
  static sampletime_t time_now;
  static sampletime_t max_realtime_step;
  static list<RealtimeHandler *> realtime_fns;

public:
  static void register_realtime_fn(RealtimeHandler *handler);
  static void deregister_realtime_fn(RealtimeHandler *handler);
  static sampletime_t now();
  static sampletime_t max_step();
  static void advance(sampletime_t delta);

private:
  Clock(Clock const &from) {}
  Clock &operator =(Clock const &from) {}

public:
  Clock() {}
  virtual ~Clock() {}

  virtual string const &getName() const = 0;
  virtual void disable() = 0;
  virtual void enable() = 0;
};

class ClockManager: public Model {
private:
  typedef list<Clock *> clocklist_t;

  clocklist_t all_clocks;
  Clock *active;
  Clock *default_clock;

  ClockManager(ClockManager const &from) {}
  ClockManager &operator =(ClockManager const &from) {}

public:
  ClockManager() {
    active = NULL;
    default_clock = NULL;
  }

  void register_clock(Clock *clock);
  void deregister_clock(Clock *clock);

  clocklist_t::iterator begin() { return all_clocks.begin(); }
  clocklist_t::iterator end() { return all_clocks.end(); }
  Clock *getSelected() { return active; }

  void select_clock(Clock *clock);
  void set_default_clock(Clock *clock);
  void stop_clock();
};

extern ClockManager clockManager;

#endif
