#ifndef Macro_H	/* -*- c++ -*- */
#define Macro_H

///////////////////////////////////////////////////////////////////////////
// DESCRIPTION
// Interface

#include <memory>
#include <string>

#include "galan/global.h"
#include "galan/generator.h"
#include "galan/registry.h"
#include "galan/model.h"
#include "galan/event.h"

GALAN_BEGIN_NAMESPACE

class MacroClass;
class Macro;
class MacroInputProxy;

///////////////////////////////////////////////////////////////////////////

/**
 * A specialisation of GeneratorClass that implements (recursive)
 * sub-graphs.
 **/
class MacroClass: public GeneratorClass {
private:
  friend class Macro;

  MacroClass(): GeneratorClass(0) {}

  // These are here because GeneratorClass doesn't trust Macro with its
  // protected methods, so we redefined them here, in a proper subclass
  // of GeneratorClass.
  void unregister_desc(InputDescriptor *input) { GeneratorClass::unregister_desc(input); }
  void unregister_desc(OutputDescriptor *output) { GeneratorClass::unregister_desc(output); }
};

/**
 * A particular Generator sub-graph.
 **/
class Macro: public Generator {
public:
  /**
   * Creates our proxy GeneratorClass instances and calls
   * MacroInputProxy::initialise().
   **/
  static void initialise();

  /**
   * Create a new Macro Generator with the specified polyphony.
   **/
  Macro(bool _polyphonic, int _nvoices = DEFAULT_POLYPHONY);
  virtual ~Macro();

  /// Override virtual-constructor here to clone the whole mesh.
  virtual Generator *clone();

  /**
   * Insert a child Generator node into this Macro.
   *
   * @param name the name for the new child node
   * @param child the child node to insert
   *
   * @return true if the insertion was successful; false if the name
   * was already taken
   **/
  bool addChild(std::string const &name, Generator *child);

  /**
   * Find a child Generator node by name.
   * @param name the name to search for
   * @return a Generator pointer, or 0 if not found
   **/
  Generator *findChild(std::string const &name);

  /**
   * Remove a child Generator node (by name).
   * @param name name of the child to remove
   *
   * @return true if the child existed and was removed; false if there
   * is no child with that name
   **/
  bool removeChild(std::string const &name);

  // These next four methods return 0 if they fail.
  // Remove the resulting generators using removeChild() above.

  /**
   * Adds and returns a realtime input proxy for this Macro.
   *
   * @param name a name for the input
   * @return 0 if there's a node named the same as 'name'; a
   * Generator pointer that can be removed using removeChild()
   * otherwise
   **/
  Generator *addRealtimeInput(std::string const &name);

  /**
   * Adds and returns a randomaccess input proxy for this Macro.
   *
   * @param name a name for the input
   * @return 0 if there's a node named the same as 'name'; a
   * Generator pointer that can be removed using removeChild()
   * otherwise
   **/
  Generator *addRandomaccessInput(std::string const &name);

  /**
   * Adds and returns a realtime output proxy for this Macro.
   *
   * @param name a name for the output
   * @return 0 if there's a node named the same as 'name'; a
   * Generator pointer that can be removed using removeChild()
   * otherwise
   **/
  Generator *addRealtimeOutput(std::string const &name);

  /**
   * Adds and returns a random-access output proxy for this Macro.
   *
   * @param name a name for the output
   * @return 0 if there's a node named the same as 'name'; a
   * Generator pointer that can be removed using removeChild()
   * otherwise
   **/
  Generator *addRandomaccessOutput(std::string const &name);

  /// Overridden to proxy through our proxy-outputs.
  virtual bool read_output(RealtimeOutputDescriptor const &q, int voice, SampleBuf *buffer);

  /// Overridden to proxy through our proxy-outputs.
  virtual bool read_output(RandomaccessOutputDescriptor const &q, int voice,
			   sampletime_t offset, SampleBuf *buffer);

  /// Overridden to proxy through our proxy-outputs.
  virtual sampletime_t get_output_range(RandomaccessOutputDescriptor const &q, int voice);

  /// Passes on the command to all our child nodes.
  virtual void setPolyphony(int nvoices);

private:
  friend class MacroInputProxy;

  /**
   * The generator-class of all nodes in the graph that act as proxies
   * for one of the realtime inputs to a Macro.
   **/
  static GeneratorClass *realtimeOutputProxyClass;

  /**
   * The generator-class of all nodes in the graph that act as proxies
   * for one of the random-access inputs to a Macro.
   **/
  static GeneratorClass *randomaccessOutputProxyClass;

  MacroClass cls;	///< GeneratorClass specific to this Macro

  Registry children;	///< Our children; all Generators

  void register_desc(InputDescriptor *input);		///< Internal: adds an input to this
  void register_desc(OutputDescriptor *output);		///< Internal: adds an output to this
  void unregister_desc(InputDescriptor *input);		///< Internal: removes an input
  void unregister_desc(OutputDescriptor *output);	///< Internal: removes an output
};

/**
 * Generator specialization that implements a subgraph's view of an
 * input channel.
 **/
class MacroInputProxy: public Generator, public Model {
public:
  /// Sets up our static GeneratorClass instances.
  static void initialise();

  /// Handles an incoming IntEvent (if trackable, eventually)
  //virtual void handle_event(IntEvent &event);

private:
  friend class Macro;

  static GeneratorClass *realtimeClass;		///< Used for realtime instances
  static GeneratorClass *randomaccessClass;	///< Used for randomaccess instances

  Macro &macro;					///< Reference to our Macro
  RealtimeInputDescriptor const *rt;		///< The rt InputDescriptor we proxy for
  RandomaccessInputDescriptor const *ra;	///< The ra InputDescriptor we proxy for

  /// Called from Macro::addInput().
  MacroInputProxy(GeneratorClass &cls,
		  Macro &_macro,
		  RealtimeInputDescriptor const *_rt,
		  RandomaccessInputDescriptor const *_ra)
    : Generator(cls, false),
      macro(_macro),
      rt(_rt),
      ra(_ra)
  {}

  /// Overloaded to pull directly from our Macro's input.
  virtual bool read_output(RealtimeOutputDescriptor const &q, int voice, SampleBuf *buffer);

  /// Overloaded to pull directly from our Macro's input.
  virtual bool read_output(RandomaccessOutputDescriptor const &q, int voice,
			   sampletime_t offset, SampleBuf *buffer);

  /// Overloaded to pull directly from our Macro's input.
  virtual sampletime_t get_output_range(RandomaccessOutputDescriptor const &q, int voice);
};

///////////////////////////////////////////////////////////////////////////

GALAN_END_NAMESPACE

#endif
