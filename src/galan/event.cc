#include "galan/event.h"
#include "galan/clock.h"

GALAN_USE_NAMESPACE

priority_queue<Event *> Event::event_q;

Event::Event() {
}

sampletime_t Event::mainloop() {
  while (event_q.size()) {
    Event *e = event_q.top();

    sampletime_t delta = e->time - Clock::now();
    if (delta > 0) {
      delta = MIN(delta, Clock::max_step());
      return delta;
    }

    IFDEBUG(cerr << "Sending event " << *e << endl);
    e->send();
    delete e;
  }

  return Clock::max_step();
}

void Event::purge_references_to(EventHandler *handler) {
  priority_queue<Event *> other;

  while (event_q.size()) {
    Event *e = event_q.top();
    event_q.pop();

    if (e->target != handler)
      other.push(e);
  }

  event_q = other;
}

void Event::post() {
  IFDEBUG(cerr << "Posting event " << *this << endl);
  event_q.push(this);
}

void Event::send() {
  if (target) {
    target->handle_event(*this);
  }
}

void EventHandler::handle_event(Event const &event) {
}

void SampleEvent::send() {
  SampleEventHandler *target = dynamic_cast<SampleEventHandler *>(getTarget());

  if (target) {
    target->handle_event(*this);
  } else {
    Event::send();
  }
}

void SampleEventHandler::handle_event(SampleEvent const &event) {
}
