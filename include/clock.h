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

class Clock;
class RealtimeHandler;
class ClockManager;

/**
 * Keeps track of "real time" in the system; provides an external
 * source of synchronisation, so that the playback of sound etc.\ is
 * tied in to the real world.
 *
 * @see ClockManager
 * @see RealtimeHandler
 **/
class Clock {
public:
  /**
   * Installs a RealtimeHandler in the global handler list, to be
   * invoked whenever we advance our internal clock.
   **/
  static void register_realtime_fn(RealtimeHandler *handler);

  /**
   * Remove a RealtimeHandler previously registered with
   * register_realtime_fn().
   **/
  static void deregister_realtime_fn(RealtimeHandler *handler);

  /**
   * Retrieve the current setting of the internal clock, in sample
   * ticks since the clock was first started.
   **/
  static sampletime_t now();

  /**
   * Returns the maximum number of sample ticks that the internal
   * clock will be advanced by in one go.
   *
   * @note If Clock::advance() is given a value larger than this, it
   * breaks it up into smaller steps, calling Event::mainloop() in
   * between each step. This is an artifact of Clock::advance() and
   * Event::mainloop() working in tandem.
   *
   * @return biggest allowed number of samples to be processed at once
   **/
  static sampletime_t max_step();

  /**
   * Advance our internal clock by delta, calling Event::mainloop() as
   * necessary (if delta is larger than either Clock::max_step() or
   * the time to the next registered event in the event queue).
   **/
  static void advance(sampletime_t delta);

public:
  Clock() {}
  virtual ~Clock() {}

  /**
   * Specific clocks must implement this method to provide a
   * human-readable name for their clock instance.
   * @return a human-readable string
   **/
  virtual std::string const &getName() const = 0;

  /**
   * Called by the ClockManager when this clock is deactivated, just
   * before a new (different) clock is activated. Subclasses must
   * implement this to clean up or destroy their link to an outside
   * source of synchronisation.
   **/
  virtual void disable() = 0;

  /**
   * Called by the ClockManager when this clock is activated, just
   * after the previously-selected clock was disactivated. Subclasses
   * must implement this to initialise their link to an outside source
   * of synchronisation.
   **/
  virtual void enable() = 0;

private:
  // Clocks are pure reference types, and may not be copied.
  Clock(Clock const &from);
  Clock &operator =(Clock const &from);

private:
  static sampletime_t time_now;			///< The current setting of our internal clock
  static sampletime_t max_realtime_step;	///< How many samples we can move by in one step
  static std::list<RealtimeHandler *> realtime_fns;	///< All registered RealtimeHandlers
};

/**
 * Interface for objects which need to be told when some wall-clock
 * time has elapsed, from the synthesiser's point of view.
 *
 * For an example of usage, see OssOutput's realtime_elapsed() method,
 * and check out the interaction between Clock::advance() and
 * Event::mainloop().
 **/
class RealtimeHandler {
public:
  /**
   * This virtual function is called by Clock::advance() whenever
   * 'delta' ticks of realtime have elapsed.
   *
   * @param delta number of sample ticks that have elapsed since the
   * last call
   **/
  virtual void realtime_elapsed(sampletime_t delta) = 0;
};

/**
 * Singleton - Manages all clocks that the system makes available,
 * either in the main binary or in any plugins that choose to register
 * a clock with the ClockManager.
 **/
class ClockManager: public Model {
public:
  /**
   * Retrieve the global instance of ClockManager.
   * @return a pointer to a singleton ClockManager object
   **/
  static ClockManager *instance();

public:
  typedef std::list<Clock *> clocklist_t;

  /**
   * Register a Clock instance with the system.
   * @param clock the Clock to register
   **/
  void register_clock(Clock *clock);

  /**
   * Deregister a Clock instance previously registered with
   * register_clock().
   * @param clock the instance to deregister
   **/
  void deregister_clock(Clock *clock);

  /// Constant Iterator type, as returned by begin() and end()
  typedef clocklist_t::const_iterator const_iterator;
  const_iterator begin() const { return all_clocks.begin(); }
  const_iterator end() const { return all_clocks.end(); }

  /**
   * Returns the currently selected/active Clock instance. There can
   * be at most one active instance at any given time.
   * @return null if there is no active clock right now, otherwise a Clock pointer
   **/
  Clock *getSelected() { return active; }

  /**
   * Causes the passed-in clock to become the currently selected clock
   * instance, calling Clock::disable() and Clock::enable() as
   * appropriate.
   * @param clock the clock to make active
   **/
  void select_clock(Clock *clock);

  /**
   * Sets which clock instance to fall back on, if the
   * currently-selected clock is deregistered.
   **/
  void set_default_clock(Clock *clock);

  /**
   * Stops the clock, by passing in NULL to select_clock(), which
   * causes the current clock to be disable()d without enable()ing any
   * other clock.
   **/
  void stop_clock();

private:
  ClockManager() {
    active = NULL;
    default_clock = NULL;
  }

  clocklist_t all_clocks;
  Clock *active;		///< The currently active clock
  Clock *default_clock;		///< The fallback clock

  // Copying the ClockManager is strictly forbidden...
  ClockManager(ClockManager const &from);
  ClockManager &operator =(ClockManager const &from);

private:
  static ClockManager *_instance;	///< Our global/singleton instance
};

#endif
