#ifndef Macro_H	/* -*- c++ -*- */
#define Macro_H

///////////////////////////////////////////////////////////////////////////
// DESCRIPTION
// Interface

#include <memory>
#include <string>
#include <hash_map>
#include <vector>

#include "global.h"
#include "generator.h"
#include "registry.h"
#include "model.h"
#include "event.h"

class MacroClass;
class Macro;
class MacroInputProxy;

///////////////////////////////////////////////////////////////////////////

class MacroClass: public GeneratorClass {
  friend class Macro;

private:
  MacroClass(): GeneratorClass(0) {}

  // These are here because GeneratorClass doesn't trust Macro with its
  // protected methods, so we redefined them here, in a proper subclass
  // of GeneratorClass.
  void unregister_desc(InputDescriptor *input) { GeneratorClass::unregister_desc(input); }
  void unregister_desc(OutputDescriptor *output) { GeneratorClass::unregister_desc(output); }
};

class Macro: public Generator {
  friend class MacroInputProxy;

private:
  static GeneratorClass *realtimeOutputProxyClass;
  static GeneratorClass *randomaccessOutputProxyClass;

  MacroClass cls;

  Registry children;	// children are all Generators

  void register_desc(InputDescriptor *input);
  void register_desc(OutputDescriptor *output);
  void unregister_desc(InputDescriptor *input);
  void unregister_desc(OutputDescriptor *output);

public:
  Macro(bool _polyphonic, int _nvoices = DEFAULT_POLYPHONY);
  virtual ~Macro();

  bool addChild(string const &name, Generator *child);
  Generator *findChild(string const &name);
  bool removeChild(string const &name);

  // These next four methods return 0 if they fail.
  // Remove the resulting generators using removeChild() above.
  Generator *addInput(RealtimeInputDescriptor *input, bool trackable = true);
  Generator *addInput(RandomaccessInputDescriptor *input);
  Generator *addRealtimeOutput(string const &name);
  Generator *addRandomaccessOutput(string const &name);

  virtual bool read_output(RealtimeOutputDescriptor const &q, int voice, SampleBuf *buffer);
  virtual bool read_output(RandomaccessOutputDescriptor const &q, int voice,
			   sampletime_t offset, SampleBuf *buffer);
  virtual sampletime_t get_output_range(RandomaccessOutputDescriptor const &q, int voice);

  virtual void setPolyphony(int nvoices);

  static void initialise();
};

class MacroInputProxy: public Generator, public Model, public IntEventHandler {
  friend class Macro;

private:
  static GeneratorClass *realtimeClass;
  static GeneratorClass *randomaccessClass;

  Macro &macro;
  RealtimeInputDescriptor const *rt;
  RandomaccessInputDescriptor const *ra;
  bool trackable;
  int value;

  MacroInputProxy(GeneratorClass &cls, Macro &_macro,
		  RealtimeInputDescriptor const *_rt,
		  RandomaccessInputDescriptor const *_ra,
		  bool _trackable)
    : Generator(cls, false),
      macro(_macro),
      rt(_rt), ra(_ra),
      trackable(_trackable),
      value(0)
  {}

  virtual bool read_output(RealtimeOutputDescriptor const &q, int voice, SampleBuf *buffer);
  virtual bool read_output(RandomaccessOutputDescriptor const &q, int voice,
			   sampletime_t offset, SampleBuf *buffer);
  virtual sampletime_t get_output_range(RandomaccessOutputDescriptor const &q, int voice);

public:
  virtual void handle_event(IntEvent &event);

  static void initialise();
};

///////////////////////////////////////////////////////////////////////////

#endif
