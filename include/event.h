#ifndef Event_H	/* -*- c++ -*- */
#define Event_H

///////////////////////////////////////////////////////////////////////////
// DESCRIPTION
// Interface

#include <queue>

#include "global.h"
#include "sample.h"

///////////////////////////////////////////////////////////////////////////

class Event;

class EventHandler {
public:
  virtual void handle_event(Event &event) {};
};

class Event {
private:
  static priority_queue<Event> event_q;

public:
  static sampletime_t mainloop();
  static void purge_references_to(EventHandler *handler);

private:
  EventHandler *target;
  sampletime_t time;

  Event() {}

public:
  Event(EventHandler *_target, sampletime_t _time)
    : target(_target),
      time(_time) {
  }
  virtual ~Event() {}

  EventHandler *getTarget() const { return target; }
  sampletime_t getTime() const { return time; }

  void post();
  virtual void send();

  bool operator <(Event const &right) const { return time >= right.time; }

  friend ostream &operator<<(ostream &o, Event &e);
};

inline ostream &operator<<(ostream &o, Event &e) {
  return o << "Event(" << e.target << "@" << e.time << ")";
}

///////////////////////////////////////////////////////////////////////////

class IntEvent;

class IntEventHandler: public EventHandler {
public:
  virtual void handle_event(IntEvent &event) {}
};

class IntEvent: public Event {
private:
  int value;

public:
  IntEvent(EventHandler *_target, sampletime_t _time, int _value): Event(_target, _time) {
    value = _value;
  }

  int getValue() const { return value; }

  virtual void send();
};

#endif
