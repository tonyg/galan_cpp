#ifndef Generator_H	/* -*- c++ -*- */
#define Generator_H

///////////////////////////////////////////////////////////////////////////
// DESCRIPTION
// Interface

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <list>

#include "global.h"
#include "model.h"
#include "sample.h"
#include "clock.h"
#include "registry.h"
#include "ref.h"

GALAN_BEGIN_NAMESPACE

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

/**
 * Describes one of the connections available for passing data to a
 * Generator. Essentially, describes a single input to or output from
 * a Generator.
 **/
class ConnectionDescriptor {
public:
  /**
   * Create a ConnectionDescriptor with the passed-in human-readable name.
   *
   * @param _name the name of this connection, to be displayed to the user
   **/
  ConnectionDescriptor(std::string const &_name);
  virtual ~ConnectionDescriptor() {}

  /// Retrieve the display name for this ConnectionDescriptor
  std::string const &getName() const { return name; }

  /// Retrieve the GeneratorClass that this ConnectionDescriptor is tied to
  GeneratorClass *getGeneratorClass() const { return cls; }

  // For internal use only
  int const &getInternalIndex() const { return index; }

private:
  friend class GeneratorClass;
  std::string name;	///< name of the connection
  GeneratorClass *cls;	///< for internal use - set by GeneratorClass on registration
  int index;		///< for internal use - set by GeneratorClass on registration

  ConnectionDescriptor() {}
  ConnectionDescriptor(ConnectionDescriptor const &from);		//unimpl
  ConnectionDescriptor &operator =(ConnectionDescriptor const &from);	//unimpl

  /// Should only be called by GeneratorClass::register_desc.
  void setInternal(GeneratorClass *_cls, int _index) { cls = _cls; index = _index; }
};

/**
 * Describes an inbound connection to a Generator.
 **/
class InputDescriptor: public ConnectionDescriptor {
public:
  InputDescriptor(std::string const &_name): ConnectionDescriptor(_name) {}
};

/**
 * Describes an inbound realtime (streaming) connection to a
 * Generator.
 **/
class RealtimeInputDescriptor: public InputDescriptor {
public:
  RealtimeInputDescriptor(std::string const &_name): InputDescriptor(_name) {}
};

/**
 * Describes an inbound random-access (precomputed) connection to a
 * Generator.
 **/
class RandomaccessInputDescriptor: public InputDescriptor {
public:
  RandomaccessInputDescriptor(std::string const &_name): InputDescriptor(_name) {}
};

/**
 * Describes an outbound connection from a Generator.
 **/
class OutputDescriptor: public ConnectionDescriptor {
public:
  OutputDescriptor(std::string const &_name): ConnectionDescriptor(_name) {}

  /**
   * Subclasses override this function to allow the GUI to allow or
   * disallow a connection between a given pair of
   * output/inputdescriptors based on the descriptor type. For
   * instance, RealtimeOutputDescriptor only returns true from this
   * method if it is passed an argument of type
   * RealtimeInputDescriptor. RandomaccessOutputDescriptor is similar.
   *
   * @param other the InputDescriptor to check for compatibility with
   * @return true if a connection could be made between this and other
   **/
  virtual bool compatibleWith(InputDescriptor const *other) const {
    return false;
  }
};

/**
 * Describes an outbound realtime (streaming) connection from a
 * Generator.
 **/
class RealtimeOutputDescriptor: public OutputDescriptor {
public:
  /**
   * Functions of this signature are used to produce a sample buffer's
   * worth of audio samples.
   *
   * @param desc the descriptor being asked to generate samples
   * @param buffer the buffer to fill with samples
   *
   * @return false if the buffer remains untouched, but would have
   * been filled with zeros (ie. "silence"); true if the buffer was
   * filled with samples as requested.
   **/
  typedef bool (GeneratorState::*samplefn_t)(RealtimeOutputDescriptor const &desc,
					     SampleBuf *buffer);

public:
  RealtimeOutputDescriptor(std::string const &_name, samplefn_t _fn)
    : OutputDescriptor(_name),
      fn(_fn) {
  }

  /// Retrieve the sample generator for this descriptor
  samplefn_t getSampleFn() const { return fn; }

  virtual bool compatibleWith(InputDescriptor const *other) const {
    return dynamic_cast<RealtimeInputDescriptor const *>(other) != 0;
  }

private:
  samplefn_t fn;	///< The sample-generator associated with this output
};

/**
 * Describes an outbound random-access (precomputed) connection from a
 * Generator.
 **/
class RandomaccessOutputDescriptor: public OutputDescriptor {
public:
  /**
   * Functions of this signature are used to query the number of
   * samples available via this output descriptor.
   **/
  typedef sampletime_t (GeneratorState::*rangefn_t)(RandomaccessOutputDescriptor const &desc);

  /**
   * Functions of this signature are used to produce a sample buffer's
   * worth of audio samples at the given offset.
   *
   * @param desc the descriptor being asked to generate samples
   * @param offset number of samples to "skip" before filling
   * @param buffer the buffer to fill with samples
   *
   * @return false if the buffer remains untouched, but would have
   * been filled with zeros (ie. "silence"); true if the buffer was
   * filled with samples as requested.
   **/
  typedef bool (GeneratorState::*samplefn_t)(RandomaccessOutputDescriptor const &desc,
					     sampletime_t offset, SampleBuf *buffer);

public:
  RandomaccessOutputDescriptor(std::string const &_name, rangefn_t _range, samplefn_t _samples)
    : OutputDescriptor(_name),
      range(_range),
      samples(_samples) {
  }

  /// Retrieve the ranging function for this descriptor
  rangefn_t getRangeFn() const { return range; }
  /// Retrieve the sample generator for this descriptor
  samplefn_t getSampleFn() const { return samples; }

  virtual bool compatibleWith(InputDescriptor const *other) const {
    return dynamic_cast<RandomaccessInputDescriptor const *>(other) != 0;
  }

private:
  rangefn_t range;		///< Our range-determiner
  samplefn_t samples;		///< Our sample-generator
};

///////////////////////////////////////////////////////////////////////////

/**
 * Describes a class of Generators. One plugin usually implements a
 * GeneratorState subclass, with associated factory function
 * (GeneratorClass::factory_fn_t), and then creates an instance of
 * GeneratorClass. Instances of Generator are built by GeneratorClass
 * instances via the instantiate() method.
 *
 * @see GeneratorState
 * @see Generator
 **/
class GeneratorClass: public Registrable {
public:
  /**
   * Functions of this signature create internal GeneratorState
   * instances, which represent a single voices' worth of plugin
   * state.
   *
   * @see GeneratorStateFactory
   *
   * @param _gen the Generator instance we are creating state for
   * @param _voice the voice number we are creating state for %%%
   **/
  typedef class GeneratorState *(*factory_fn_t)(Generator &_gen, int _voice);

  /// Create an anonymous GeneratorClass, based on the supplied factory function
  GeneratorClass(factory_fn_t _fn) : factory_fn(_fn) {}

  /**
   * Create a named (and listed in the GUI) GeneratorClass based on
   * the supplied factory function.
   *
   * @see Registry
   *
   * @param _fn the factory function to use
   * @param path the Registry path to bind this GeneratorClass under
   *
   * @param preferred set this to true to replace any existing entry
   * in the registry under our chosen name. This can be used to choose
   * between multiple plugins implementing the same general interface.
   **/
  GeneratorClass(factory_fn_t _fn, std::string const &path, bool preferred = false)
    : factory_fn(_fn)
  {
    Registry::root->bind(std::string("Generator/") + path, this, preferred);
  }

  /**
   * Called after construction to install an inbound
   * connection-descriptor on this GeneratorClass. Should be called
   * before instantiate()ing any Generators from this GeneratorClass.
   **/
  void register_desc(InputDescriptor *input);

  /**
   * Called after construction to install an outbound
   * connection-descriptor on this GeneratorClass. Should be called
   * before instantiate()ing any Generators from this GeneratorClass.
   **/
  void register_desc(OutputDescriptor *output);

  /// Counts the number of input connections instances will have.
  int getNumInputs() const { return inputs.size(); }
  /// Counts the number of output connections instances will have.
  int getNumOutputs() const { return outputs.size(); }

  /// Retrieve all the input descriptors for this class.
  std::vector<InputDescriptor *> getInputs() const;
  /// Retrieve all the output descriptors for this class.
  std::vector<OutputDescriptor *> getOutputs() const;

  /// Retrieves an InputDescriptor by name
  InputDescriptor const &getInput(std::string const &name) const;
  /// Retrieves an OutputDescriptor by name
  OutputDescriptor const &getOutput(std::string const &name) const;

  /// As for getInput, but force interpretation as a RealtimeInputDescriptor.
  RealtimeInputDescriptor const &getRealtimeInput(std::string const &name) const {
    return dynamic_cast<RealtimeInputDescriptor const &>(getInput(name));
  }

  /// As for getInput, but force interpretation as a RandomaccessInputDescriptor.
  RandomaccessInputDescriptor const &getRandomaccessInput(std::string const &name) const {
    return dynamic_cast<RandomaccessInputDescriptor const &>(getInput(name));
  }

  /// Use our factory_fn to generate an instance of GeneratorState for the given voice. %%%
  GeneratorState *instantiate(Generator &_gen, int _voice);

protected:
  typedef std::map< std::string, ConnectionDescriptor * > descriptormap_t;

  descriptormap_t inputs;	///< All inbound connections
  descriptormap_t outputs;	///< All outbound connections

  // Primarily of use to class Macro. Caller must delete any unregistered instances.
  void unregister_desc(InputDescriptor *input);
  void unregister_desc(OutputDescriptor *output);

private:
  friend class Generator;

  factory_fn_t factory_fn;	///< Our state factory
};

///////////////////////////////////////////////////////////////////////////

/**
 * A directed link between two specific Generator instances. Links
 * between a specific InputDescriptor and a specific OutputDescriptor.
 **/
struct Conduit {
  Generator *src;			///< Source Generator
  OutputDescriptor const *src_q;	///< Source Generator's OutputDescriptor
  Generator *dst;			///< Destination Generator
  InputDescriptor const *dst_q;		///< Destination Generator's InputDescriptor

  Conduit(Generator *_src, OutputDescriptor const *_src_q,
	  Generator *_dst, InputDescriptor const *_dst_q)
    : src(_src), src_q(_src_q),
      dst(_dst), dst_q(_dst_q)
  {}

  void unlink();			///< Delegates to Generator::unlink().

  /**
   * Two Conduits are equal if both the source and target are
   * identical, and the two Conduits link exactly the same pair of
   * descriptors together.
   **/
  bool operator==(Conduit const &other) const {
    return
      other.src == src && other.src_q == src_q &&
      other.dst == dst && other.dst_q == dst_q;
  }
};

/**
 * Buffers a realtime sample stream so that it is not excessively recomputed.
 **/
class SampleCache {
public:
  /**
   * Construct an empty SampleCache.
   **/
  SampleCache(): last_sampletime(-1), last_buffer(0), last_result(false) {}

  /// Clone an existing SampleCache.
  SampleCache(SampleCache const &other) { assign(other); }
  /// Become a copy of an existing SampleCache.
  SampleCache const &operator=(SampleCache const &other) { assign(other); return *this; }

  ~SampleCache() {
    if (last_buffer) delete last_buffer;
  }

  /**
   * Fills 'buffer' with samples from the cache, if they're
   * valid. Uses 'voice' and 'output' (taken together) to top up or
   * refresh the cache buffer if it is not big enough or too old. Uses
   * Clock::now() to determine if the cached buffer (if any) is still
   * valid.
   *
   * @param voice the GeneratorState we're sourcing samples from
   * @param output the descriptor we're using on 'voice'
   * @param buffer the buffer to fill with samples
   * @return true if samples were filled in; false for "silence"; cf. samplefn_t
   **/
  bool read(GeneratorState *voice, RealtimeOutputDescriptor const &output, SampleBuf *buffer);

private:
  sampletime_t last_sampletime;		///< First sample in cache buffer is for this time
  SampleBuf *last_buffer;		///< Cache buffer
  bool last_result;			///< Result of call to RealtimeOutputDescriptor::samplefn_t

  /// Copies another SampleCache, duplicating its internal buffer.
  void assign(SampleCache const &other) {
    last_sampletime = other.last_sampletime;
    if (last_buffer)
      delete last_buffer;
    last_buffer = (other.last_buffer ? new SampleBuf(*other.last_buffer) : 0);
    last_result = other.last_result;
  }
};

/**
 * The core class of the system. Represents a specific instance of a
 * GeneratorClass (ie. an instance of a plugin), that is, something
 * that generates samples on one or more outputs, based on zero or
 * more inputs.
 **/
class Generator: public Registrable {
public:
  typedef std::list<Conduit *> conduitlist_t;

  Generator(GeneratorClass &_cls, bool _polyphonic, int _nvoices = DEFAULT_POLYPHONY);
  virtual ~Generator();

  /// "Virtual-constructor" idiom. Used for cut-and-paste, Macro implementation, etc. etc.
  virtual Generator *clone();

  GeneratorClass &getClass() const { return cls; }	///< Retrieve GeneratorClass of this
  bool isPolyphonic() const { return polyphonic; }	///< "Is this object polyphonic?"

  /**
   * Returns the number of voices that this object supports. %%%
   **/
  int getNumVoices() const { return voices.size(); }

  /**
   * Creates a link between this and 'dst', between the named output
   * and input descriptors.
   *
   * @param src_q the output descriptor to link from
   * @param dst the target generator
   * @param dst_q the input on the target generator to link to
   **/
  void link(OutputDescriptor const &src_q, Generator *dst, InputDescriptor const &dst_q);

  /**
   * Removes a link between this and 'dst'.
   * @see Generator::link()
   **/
  void unlink(OutputDescriptor const &src_q, Generator *dst, InputDescriptor const &dst_q);

  /**
   * Retrieve a list of the connections to a specific input.
   **/
  conduitlist_t const &inboundLinks(InputDescriptor const &q);

  /**
   * Retrieve a list of the connections from a specific output.
   **/
  conduitlist_t const &outboundLinks(OutputDescriptor const &q);

  /**
   * Pulls samples from a realtime output on this generator.
   *
   * @param q the output we should use to respond
   * @param voice the voice to use when responding
   * @param buffer the buffer to fill
   * @return false for silence; true if buffer was filled
   **/
  virtual bool read_output(RealtimeOutputDescriptor const &q, int voice, SampleBuf *buffer);

  /**
   * Pulls samples from a random-access output on this generator.
   *
   * @param q the output we should use to respond
   * @param voice the voice to use when responding
   * @param offset the offset within our output to start sourcing from
   * @param buffer the buffer to fill
   * @return false for silence; true if buffer was filled
   **/
  virtual bool read_output(RandomaccessOutputDescriptor const &q, int voice,
			   sampletime_t offset, SampleBuf *buffer);

  /**
   * Retrieve the random-access output range of one of our outputs.
   *
   * @param q the output to query
   * @param voice the voice to query
   * @return the number of samples available right now
   **/
  virtual sampletime_t get_output_range(RandomaccessOutputDescriptor const &q, int voice);

  /**
   * Commands this Generator to become 'n'-phonic. %%%
   *
   * @param nvoices the new number of voices this Generator should
   * support
   **/
  virtual void setPolyphony(int nvoices);

  /**
   * Internal - Pulls samples from all Generators connected to one of
   * our realtime input connectors.
   *
   * @param q the input connector to use
   * @param voice the voice (layer) to use
   * @param buffer the buffer to fill
   * @return false for silence; true if buffer was filled
   **/
  bool read_input(RealtimeInputDescriptor const &q, int voice, SampleBuf *buffer);

  /**
   * Internal - Pulls samples from the Generators connected to one of
   * our random-access input connectors.
   *
   * @param q the input connector to use
   * @param voice the voice (layer) to use
   * @param offset the offset within the remote Generator's output connector
   * @param buffer the buffer to fill
   * @return false for silence; true if buffer was filled
   **/
  bool read_input(RandomaccessInputDescriptor const &q, int voice,
			  sampletime_t offset, SampleBuf *buffer);

  /**
   * Internal - Retrieve the width of the Generator connected to one
   * of our random-access inputs, if any.
   *
   * @param q our input connector
   * @param voice the voice to use
   * @return the number of available samples (may be zero)
   **/
  sampletime_t get_input_range(RandomaccessInputDescriptor const &q, int voice);

protected:
  // Mostly for use by Macro.
  void addInput();			///< Internal: extends 'inputs' by one
  void addOutput();			///< Internal: extends 'outputs' by one
  void removeInput(int index);		///< Internal: deletes a given entry from 'inputs'
  void removeOutput(int index);		///< Internal: deletes a given entry from 'outputs'

private:
  friend class GeneratorState;
  friend class Macro;

  typedef std::vector<GeneratorState *> statevec_t;
  typedef std::vector< std::vector<SampleCache> > cachevec_t;

  GeneratorClass &cls;			///< where we find info about how we communicate
  std::vector<conduitlist_t> inputs;	///< all conduits connected to our inputs
  std::vector<conduitlist_t> outputs;	///< all conduits connected to our outputs

  bool polyphonic;			///< If false, we only ever have a single voice.

  /**
   * All voices in this instance. degree of polyphony
   * is implicit in the length of the vector.
   **/
  statevec_t voices;

  cachevec_t caches;			///< Matrix of cached outputs, one array per voice.

  Generator();						// unimpl
  Generator(Generator const &from);			// unimpl
  Generator const &operator =(Generator const &from);	// unimpl

  /**
   * Searches for a link between us and 'dst' between the specified
   * descriptors.
   * @param src_q the output descriptor to search based on
   * @param dst the destination Generator
   * @param dst_q the input descriptor of the destination to search based on
   * @return an appropriate Conduit if one exists, or 0 otherwise.
   **/
  Conduit *find_link(OutputDescriptor const *src_q, Generator *dst, InputDescriptor const *dst_q);

  /**
   * Sums up ("aggregates") all sources connected to 'q' (one of our
   * inputs), for the given voice %%%, and collects the resulting
   * sample stream into 'buffer'.
   *
   * @param q the input to read
   * @param voice the voice we're dealing with %%%
   * @param buffer the output buffer
   * @return true if any samples were filled in; false for "silence"; see samplefn_t
   **/
  bool aggregate_input(RealtimeInputDescriptor const &q, int voice, SampleBuf *buffer);
};

///////////////////////////////////////////////////////////////////////////

/**
 * Base class which generates samples based on some internal
 * state. Plugins will generally subclass this and pass
 * member-function-pointers into GeneratorClass instances via
 * OutputDescriptor instances.
 *
 * @see RealtimeOutputDescriptor::samplefn_t
 * @see RandomaccessOutputDescriptor::samplefn_t
 * @see RandomaccessOutputDescriptor::rangefn_t
 **/
class GeneratorState {
private:
  Generator &gen;			///< the generator instance we hold state for a voice of.
  int voice;				///< which voice in my Generator do I represent? %%%

  GeneratorState(GeneratorState const &from);			// unimpl
  GeneratorState const &operator =(GeneratorState const &from);	// unimpl

protected:
  /**
   * Convenience method used to collect samples from one of our
   * realtime inputs.
   *
   * @param q describes which input to read
   * @param buffer the buffer to fill
   * @return false for silence; true otherwise
   **/
  bool read_input(RealtimeInputDescriptor const &q, SampleBuf *buffer) {
    return getInstance().read_input(q, voice, buffer);
  }

  /**
   * Convenience method used to collect samples from one of our
   * random-access inputs.
   *
   * @param q describes which input to read
   * @param offset the offset within the input to source from
   * @param buffer the buffer to fill
   * @return false for silence; true otherwise
   **/
  bool read_input(RandomaccessInputDescriptor const &q, sampletime_t offset, SampleBuf *buffer) {
    return getInstance().read_input(q, voice, offset, buffer);
  }

  /**
   * Convenience method to return the number of samples available from
   * a particular random-access input.
   *
   * @param q describes which input to query
   * @return the number of samples available from that particular
   * input right now
   **/
  sampletime_t get_input_range(RandomaccessInputDescriptor const &q) {
    return getInstance().get_input_range(q, voice);
  }
  
public:
  /**
   * Construct a GeneratorState for a given Generator instance and
   * voice number.
   **/
  GeneratorState(Generator &_gen, int _voice): gen(_gen), voice(_voice) {}
  virtual ~GeneratorState() {}

  Generator &getInstance() const { return gen; }	///< Get our associated Generator
  int getVoice() const { return voice; }		///< Get our associated voice number
};

/**
 * Template factory function for producing GeneratorState
 * instances. Useful for passing in to GeneratorClass constructors.
 **/
template <class S> GeneratorState *GeneratorStateFactory(Generator &_gen, int _voice) {
  return new S(_gen, _voice);
}

///////////////////////////////////////////////////////////////////////////

inline void Conduit::unlink() {
  src->unlink(*src_q, dst, *dst_q);
}

inline GeneratorState *GeneratorClass::instantiate(Generator &_gen, int _voice) {
  return factory_fn(_gen, _voice);
}

GALAN_END_NAMESPACE

#endif
