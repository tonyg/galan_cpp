#include <stdexcept>
#include <strstream>

#include "galan/controller.h"

GALAN_USE_NAMESPACE

///////////////////////////////////////////////////////////////////////////

Controller *Controller::_active_instance = 0;

Controller::Controller(std::string const &style,
		       std::string const &name)
{
  std::string fullname = "Controller/" + style + "/" + name;

  if (!Registry::root->bind(fullname, this)) {
    throw std::range_error(std::string(style) + " name \"" + name + "\" already in use");
  }
}

Controller::~Controller() {
  unbind();
}

Controller *Controller::active_instance() {
  if (_active_instance == 0) {
    throw std::logic_error("Controller::active_instance called without active instance!");
  }

  return _active_instance;
}

bool Controller::have_active_instance() {
  return (_active_instance != 0);
}

void Controller::activate() {
  if (_active_instance != 0) {
    throw std::logic_error("Controller::activate called with instance already active!");
  }

  _active_instance = this;
}

void Controller::deactivate() {
  if (_active_instance != this) {
    std::ostrstream s;
    s << "Eeek! deactivate called on " << this
      << " when instance == " << _active_instance << "!" << std::ends;
    throw std::logic_error(s.str());
  }

  _active_instance = 0;
}

///////////////////////////////////////////////////////////////////////////

RealtimeController::RealtimeController(std::string const &name)
  : Controller("Realtime", name),
    current_value()
{}

void RealtimeController::handle_event(SampleEvent const &event) {
  //IFDEBUG(cerr << "RealtimeController " << this << " received " << event << endl);
  current_value = event.getValue();
}

Sample RealtimeController::value() {
  return current_value;
}

///////////////////////////////////////////////////////////////////////////

RandomaccessController::RandomaccessController(std::string const &name)
  : Controller("Randomaccess", name),
    buf(new SampleBuf())
{}

RandomaccessController::~RandomaccessController() {
  delete buf;
}

SampleBuf &RandomaccessController::buffer() const {
  return *buf;
}
