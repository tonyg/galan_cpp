#ifndef Event_H	/* -*- c++ -*- */
#define Event_H

///////////////////////////////////////////////////////////////////////////
// DESCRIPTION
// Interface

#include <queue>
#include <iostream>

#include "galan/global.h"
#include "galan/sample.h"

GALAN_BEGIN_NAMESPACE

class Event;
class EventHandler;

class SampleEvent;
class SampleEventHandler;

///////////////////////////////////////////////////////////////////////////

/**
 * Internal event representation. Used to represent messages passed
 * between Generator instances etc. More generally, Events can be
 * targetted at instances of EventHandler.
 **/
class Event: public Destructable {
public:
  /**
   * The master event dispatcher. Processes all events in the event_q
   * until the next event in the queue is due to be processed in the
   * future. Returns the number of sample ticks until that next event
   * is due to occur. If the next event in the future is more than
   * Clock::max_step() sample ticks away, returns Clock::max_step().
   * Expects to be called again when the returned number of ticks have
   * elapsed (according to the currently-selected realtime clock).
   *
   * @see Clock::advance
   * @see Clock
   * @see ClockManager
   *
   * @return the number of sample ticks to advance our realtime clock by
   **/
  static sampletime_t mainloop();

  /**
   * Filters the event queue, removing all events targetted at the
   * passed-in event handler.  Rebuilds the entire pending-event queue
   * (which is a heap)!
   *
   * @param handler the EventHandler that is now an unperson
   **/
  static void purge_references_to(EventHandler *handler);

public:
  /**
   * Create an event targetted at _target, and due to be delivered at
   * time _time (if put on the global queue with post(), that is).
   **/
  Event(EventHandler *_target, sampletime_t _time)
    : target(_target),
      time(_time) {
  }
  virtual Event *clone() const = 0;	///< Subclasses must override

  /// Retrieve the target of this event.
  EventHandler *getTarget() const { return target; }

  /// Retrieve the scheduled delivery-time for this event.
  sampletime_t getTime() const { return time; }

  /// Enqueue this event on the global queue.
  void post();

  /// Deliver this event *now* - note, doesn't dequeue it if it's already been post()ed!
  virtual void send();

  /// Ordering predicate for use by event_q.
  bool operator <(Event const &right) const { return time >= right.time; }

private:
  EventHandler *target;		///< Which EventHandler this event is destined for
  sampletime_t time;		///< Time at which the event should be delivered by mainloop()

  Event();

  // No copying.
  Event(Event const &other);
  Event &operator=(Event const &other);

private:
  static std::priority_queue<Event *> event_q;	///< The global event queue
};

inline std::ostream &operator<<(std::ostream &o, Event const &e) {
  return o << "Event(" << e.getTarget() << "@" << e.getTime() << ")";
}

/**
 * Classes which implement this interface can receive Event instances
 * via Event::post()/Event::mainloop() or Event::send().
 **/
class EventHandler {
public:
  /**
   * Implement this to handle incoming events.
   **/
  virtual void handle_event(Event const &event);
};

///////////////////////////////////////////////////////////////////////////

/**
 * Subclass of Event that carries a single sample as payload.
 **/
class SampleEvent: public Event {
private:
  Sample value;		///< Our payload

public:
  virtual SampleEvent *clone() const { return new SampleEvent(getTarget(), getTime(), value); }

  SampleEvent(EventHandler *_target, sampletime_t _time, Sample _value): Event(_target, _time) {
    value = _value;
  }

  Sample getValue() const { return value; }

  /// Overridden to support instances of SampleEventHandler specially.
  virtual void send();
};

inline std::ostream &operator<<(std::ostream &o, SampleEvent const &e) {
  return o << "SampleEvent(" << e.getTarget() << "@" << e.getTime() << ":" << e.getValue() << ")";
}

/**
 * Implement this interface if you'll be taking delivery of SampleEvent
 * instances often.
 **/
class SampleEventHandler: public EventHandler {
public:
  /// Overloaded method specialized for SampleEvent processing.
  virtual void handle_event(SampleEvent const &event);
};

GALAN_END_NAMESPACE

#endif
