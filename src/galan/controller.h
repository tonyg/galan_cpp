#ifndef Galan_Controller_H	/* -*- c++ -*- */
#define Galan_Controller_H

#include <string>

#include "galan/global.h"
#include "galan/registry.h"
#include "galan/event.h"
#include "galan/sample.h"
#include "galan/model.h"

GALAN_BEGIN_NAMESPACE

class Controller;

///////////////////////////////////////////////////////////////////////////

/**
 * Base class for links between the synth mesh and the user
 * interface. The UI should send events (SampleEvents for
 * RealtimeController instances, others for RandomaccessController
 * instances) through to Controllers, which are then read out by
 * Generator instances.
 **/
class Controller: public Registrable, public Model {
public:
  // Static protocol for safely constructing instances of class
  // Controller and passing them reliably over to GeneratorState
  // instances responsible for transmitting their signals further down
  // the processing chain.

  /**
   * Retrieves the "current instance" of class Controller. Use in
   * conjunction with activate() and deactivate(). Throws
   * std::logic_error if there's no currently active instance.
   **/
  static Controller *active_instance();

  /**
   * Returns true if there is a currently active instance; false
   * otherwise.
   **/
  static bool have_active_instance();

  /**
   * Call this to activate this instance of Controller. There can only
   * be one active at a time. Throws std::logic_error if there's
   * already an active object.
   **/
  void activate();

  /**
   * Call this to deactivate this instance of Controller. There can
   * only be one active at a time. Throws std::logic_error if instance
   * is not equal to this.
   **/
  void deactivate();

  /**
   * Unbinds this in addition to normal duties.
   **/
  virtual ~Controller();

protected:
  /**
   * Abstract class - not directly instantiable. Binds this
   * registrable, in the root Registry, to
   * "Controller/{style}/{name}".
   **/
  Controller(std::string const &style,
	     std::string const &name);

private:
  static Controller *_active_instance;	///< Current active instance
};

/**
 * Stores a single Sample variable - used for realtime control of the
 * output of a synth mesh channel. Send instances instances of
 * SampleEvent to control the value contained.
 **/
class RealtimeController: public Controller, public SampleEventHandler {
public:
  RealtimeController(std::string const &name);

  /**
   * Receives change notifications from the UI. (Set the value of the
   * controller by sending it events.)
   **/
  virtual void handle_event(SampleEvent const &event);

  Sample value();		///< Retrieve current controller value

private:
  Sample current_value;
};

/**
 * Stores a SampleBuf containing samples for a generator to make use
 * of.
 **/
class RandomaccessController: public Controller {
public:
  RandomaccessController(std::string const &name);
  virtual ~RandomaccessController();

  /**
   * Retrieve the sample buffer contained, for further operations.
   **/
  SampleBuf &buffer() const;

private:
  SampleBuf *buf;	///< The contents of the randomaccess-controller.
};

GALAN_END_NAMESPACE

#endif
