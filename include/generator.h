#ifndef Generator_H	/* -*- c++ -*- */
#define Generator_H

///////////////////////////////////////////////////////////////////////////
// DESCRIPTION
// Interface

#include <memory>
#include <string>
#include <hash_map>
#include <vector>

#include "global.h"
#include "model.h"
#include "sample.h"
#include "clock.h"
#include "registry.h"
#include "ref.h"

class	GeneratorClass;
class	Generator;
class	GeneratorState;

class	ConnectionDescriptor;
class		InputDescriptor;
class   		RealtimeInputDescriptor;
class			RandomaccessInputDescriptor;
class		OutputDescriptor;
class			RealtimeOutputDescriptor;
class			RandomaccessOutputDescriptor;

#define DEFAULT_POLYPHONY	4

///////////////////////////////////////////////////////////////////////////

class ConnectionDescriptor {
private:
  friend class GeneratorClass;
  string name;		// name of the connection
  GeneratorClass *cls;	// for internal use - set by GeneratorClass on registration
  int index;		// for internal use - set by GeneratorClass on registration

  ConnectionDescriptor() {}
  ConnectionDescriptor(ConnectionDescriptor const &from) {}
  ConnectionDescriptor const &operator =(ConnectionDescriptor const &from) {}

  // Should only be called by GeneratorClass::register_desc.
  void setInternal(GeneratorClass *_cls, int _index) { cls = _cls; index = _index; }

public:
  ConnectionDescriptor(string const &_name);
  virtual ~ConnectionDescriptor() {}

  string const &getName() const { return name; }
  GeneratorClass *getGeneratorClass() const { return cls; }
  int const &getInternalIndex() const { return index; }
};

class InputDescriptor: public ConnectionDescriptor {
public:
  InputDescriptor(string const &_name): ConnectionDescriptor(_name) {}
};

class RealtimeInputDescriptor: public InputDescriptor {
public:
  RealtimeInputDescriptor(string const &_name): InputDescriptor(_name) {}
};

class RandomaccessInputDescriptor: public InputDescriptor {
public:
  RandomaccessInputDescriptor(string const &_name): InputDescriptor(_name) {}
};

class OutputDescriptor: public ConnectionDescriptor {
public:
  OutputDescriptor(string const &_name): ConnectionDescriptor(_name) {}

  virtual bool compatibleWith(InputDescriptor const *other) const {
    return false;
  }
};

class RealtimeOutputDescriptor: public OutputDescriptor {
public:
  typedef bool (GeneratorState::*samplefn_t)(RealtimeOutputDescriptor const &desc,
					     SampleBuf *buffer);

private:
  samplefn_t fn;

public:
  RealtimeOutputDescriptor(string const &_name, samplefn_t _fn)
    : OutputDescriptor(_name),
      fn(_fn) {
  }

  samplefn_t getSampleFn() const { return fn; }

  virtual bool compatibleWith(InputDescriptor const *other) const {
    return dynamic_cast<RealtimeInputDescriptor const *>(other) != 0;
  }
};

class RandomaccessOutputDescriptor: public OutputDescriptor {
public:
  typedef sampletime_t (GeneratorState::*rangefn_t)(RandomaccessOutputDescriptor const &desc);
  typedef bool (GeneratorState::*samplefn_t)(RandomaccessOutputDescriptor const &desc,
					     sampletime_t offset, SampleBuf *buffer);

private:
  rangefn_t range;
  samplefn_t samples;

public:
  RandomaccessOutputDescriptor(string const &_name, rangefn_t _range, samplefn_t _samples)
    : OutputDescriptor(_name),
      range(_range),
      samples(_samples) {
  }

  rangefn_t getRangeFn() const { return range; }
  samplefn_t getSampleFn() const { return samples; }

  virtual bool compatibleWith(InputDescriptor const *other) const {
    return dynamic_cast<RandomaccessInputDescriptor const *>(other) != 0;
  }
};

///////////////////////////////////////////////////////////////////////////

class GeneratorClass: public Registrable {
  friend class Generator;

private:
  typedef class GeneratorState *(*factory_fn_t)(Generator &_gen, int _voice);

  factory_fn_t factory_fn;

protected:
  typedef hash_map< string, ref<ConnectionDescriptor> > descriptormap_t;

  descriptormap_t inputs;
  descriptormap_t outputs;

  void unregister_desc(InputDescriptor *input);
  void unregister_desc(OutputDescriptor *output);

public:
  GeneratorClass(factory_fn_t _fn) : factory_fn(_fn) {}
  GeneratorClass(factory_fn_t _fn, string const &path, bool preferred = false)
    : factory_fn(_fn)
  {
    Registry::root->bind(string("Generator/") + path, this, preferred);
  }

  void register_desc(InputDescriptor *input);
  void register_desc(OutputDescriptor *output);

  int getNumInputs() const { return inputs.size(); }
  int getNumOutputs() const { return outputs.size(); }

  InputDescriptor const &getInput(string const &name) const;
  OutputDescriptor const &getOutput(string const &name) const;

  RealtimeInputDescriptor const &getRealtimeInput(string const &name) const {
    return dynamic_cast<RealtimeInputDescriptor const &>(getInput(name));
  }

  RandomaccessInputDescriptor const &getRandomaccessInput(string const &name) const {
    return dynamic_cast<RandomaccessInputDescriptor const &>(getInput(name));
  }

  GeneratorState *instantiate(Generator &_gen, int _voice);
};

///////////////////////////////////////////////////////////////////////////

struct Conduit {
  Generator *src;
  OutputDescriptor const *src_q;
  Generator *dst;
  InputDescriptor const *dst_q;

  Conduit(Generator *_src, OutputDescriptor const *_src_q,
	  Generator *_dst, InputDescriptor const *_dst_q)
    : src(_src), src_q(_src_q),
      dst(_dst), dst_q(_dst_q)
  {}

  void unlink();

  bool operator==(Conduit const &other) const {
    return
      other.src == src && other.src_q == src_q &&
      other.dst == dst && other.dst_q == dst_q;
  }
};

class SampleCache {
private:
  sampletime_t last_sampletime;		// first sample in cache buffer is for this time
  SampleBuf *last_buffer;		// cache buffer
  bool last_result;

  void assign(SampleCache const &other) {
    last_sampletime = other.last_sampletime;
    if (last_buffer)
      delete last_buffer;
    last_buffer = (other.last_buffer ? new SampleBuf(*other.last_buffer) : 0);
    last_result = other.last_result;
  }

public:
  SampleCache(): last_sampletime(-1), last_buffer(0), last_result(false) {}
  SampleCache(SampleCache const &other) { assign(other); }
  SampleCache const &operator=(SampleCache const &other) { assign(other); return *this; }

  ~SampleCache() {
    if (last_buffer) delete last_buffer;
  }

  bool read(GeneratorState *voice, RealtimeOutputDescriptor const &output, SampleBuf *buffer);
};

class Generator: public Registrable {
  friend class GeneratorState;
  friend class Macro;

public:
  typedef vector<Conduit *> conduitvec_t;
  typedef vector<GeneratorState *> statevec_t;
  typedef vector< vector<SampleCache> > cachevec_t;

private:
  GeneratorClass &cls;			// where we find info about how we communicate
  vector<conduitvec_t> inputs;		// all conduits connected to our inputs
  vector<conduitvec_t> outputs;		// all conduits connected to our outputs

  bool polyphonic;			// If false, there is only ever going to be a single voice.
  statevec_t voices;			// All voices in this instance. degree of polyphony
					// is implicit in the length of the vector.
  cachevec_t caches;			// Matrix of cached outputs, one array per voice.

  Generator(Generator const &from): cls(from.cls) {}
  Generator const &operator =(Generator const &from) {}

  Conduit *find_link(OutputDescriptor const *src_q, Generator *dst, InputDescriptor const *dst_q);

  bool aggregate_input(RealtimeInputDescriptor const &q, int voice, SampleBuf *buffer);

protected:
  bool read_input(RealtimeInputDescriptor const &q, int voice, SampleBuf *buffer);
  bool read_input(RandomaccessInputDescriptor const &q, int voice,
			  sampletime_t offset, SampleBuf *buffer);
  sampletime_t get_input_range(RandomaccessInputDescriptor const &q, int voice);

  void addInput();
  void addOutput();
  void removeInput(int index);
  void removeOutput(int index);

public:
  Generator(GeneratorClass &_cls, bool _polyphonic, int _nvoices = DEFAULT_POLYPHONY);
  virtual ~Generator();

  GeneratorClass &getClass() const { return cls; }
  bool isPolyphonic() const { return polyphonic; }
  int getNumVoices() const { return voices.size(); }

  void link(OutputDescriptor const &src_q, Generator *dst, InputDescriptor const &dst_q);
  void unlink(OutputDescriptor const &src_q, Generator *dst, InputDescriptor const &dst_q);

  virtual bool read_output(RealtimeOutputDescriptor const &q, int voice, SampleBuf *buffer);
  virtual bool read_output(RandomaccessOutputDescriptor const &q, int voice,
			   sampletime_t offset, SampleBuf *buffer);
  virtual sampletime_t get_output_range(RandomaccessOutputDescriptor const &q, int voice);

  virtual void setPolyphony(int nvoices);
};

///////////////////////////////////////////////////////////////////////////

class GeneratorState {
private:
  Generator &gen;			// the generator instance we hold state for a voice of.
  int voice;				// which voice in my Generator do I represent?

  GeneratorState(GeneratorState const &from): gen(from.gen) {}
  GeneratorState const &operator =(GeneratorState const &from) {}

protected:
  bool read_input(RealtimeInputDescriptor const &q, SampleBuf *buffer) {
    return getInstance().read_input(q, voice, buffer);
  }

  bool read_input(RandomaccessInputDescriptor const &q, sampletime_t offset, SampleBuf *buffer) {
    return getInstance().read_input(q, voice, offset, buffer);
  }

  sampletime_t get_input_range(RandomaccessInputDescriptor const &q) {
    return getInstance().get_input_range(q, voice);
  }
  
public:
  GeneratorState(Generator &_gen, int _voice): gen(_gen), voice(_voice) {}
  virtual ~GeneratorState() {}

  Generator &getInstance() const { return gen; }
  int getVoice() const { return voice; }
};

///////////////////////////////////////////////////////////////////////////

inline void Conduit::unlink() {
  src->unlink(*src_q, dst, *dst_q);
}

inline GeneratorState *GeneratorClass::instantiate(Generator &_gen, int _voice) {
  return factory_fn(_gen, _voice);
}

#endif
