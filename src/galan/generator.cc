#include <string.h>
#include "galan/generator.h"

GALAN_USE_NAMESPACE

ConnectionDescriptor::ConnectionDescriptor(string const &_name)
  : name(_name)
{
  cls = 0;
  index = -1;
}

void GeneratorClass::unregister_desc(InputDescriptor *input) {
  int myIndex = input->getInternalIndex();

  inputs.erase(inputs.find(input->getName()));

  for (descriptormap_t::iterator i = inputs.begin(); i != inputs.end(); i++) {
    InputDescriptor *desc = dynamic_cast<InputDescriptor *>((*i).second);
    int descIndex = desc->getInternalIndex();

    if (descIndex >= myIndex)
      desc->setInternal(this, descIndex - 1);
  }
}

void GeneratorClass::unregister_desc(OutputDescriptor *output) {
  int myIndex = output->getInternalIndex();

  outputs.erase(outputs.find(output->getName()));

  for (descriptormap_t::iterator i = outputs.begin(); i != outputs.end(); i++) {
    OutputDescriptor *desc = dynamic_cast<OutputDescriptor *>((*i).second);
    int descIndex = desc->getInternalIndex();

    if (descIndex >= myIndex)
      desc->setInternal(this, descIndex - 1);
  }
}

void GeneratorClass::register_desc(InputDescriptor *input) {
  input->setInternal(this, inputs.size());
  inputs[input->name] = input;
}

void GeneratorClass::register_desc(OutputDescriptor *output) {
  output->setInternal(this, outputs.size());
  outputs[output->name] = output;
}

vector<InputDescriptor *> GeneratorClass::getInputs() const {
  vector<InputDescriptor *> result;
  for (descriptormap_t::const_iterator i = inputs.begin(); i != inputs.end(); i++) {
    result.push_back(dynamic_cast<InputDescriptor *>((*i).second));
  }
  return result;
}

vector<OutputDescriptor *> GeneratorClass::getOutputs() const {
  vector<OutputDescriptor *> result;
  for (descriptormap_t::const_iterator i = outputs.begin(); i != outputs.end(); i++) {
    result.push_back(dynamic_cast<OutputDescriptor *>((*i).second));
  }
  return result;
}

InputDescriptor const &GeneratorClass::getInput(string const &name) const {
  descriptormap_t::const_iterator i = inputs.find(name);
  if (i == inputs.end()) {
    throw std::logic_error("GeneratorClass::getInput: not found: " + name);
  }
  return *dynamic_cast<InputDescriptor *>((*i).second);
}

OutputDescriptor const &GeneratorClass::getOutput(string const &name) const {
  descriptormap_t::const_iterator i = outputs.find(name);
  if (i == outputs.end()) {
    throw std::logic_error("GeneratorClass::getOutput: not found: " + name);
  }
  return *dynamic_cast<OutputDescriptor *>((*i).second);
}

bool SampleCache::read(GeneratorState *voice, RealtimeOutputDescriptor const &output,
		       SampleBuf *buffer) {
  sampletime_t now = Clock::now();
  int last_end = last_sampletime + (last_buffer == 0 ? 0 : last_buffer->getLength());

  if (last_buffer == 0 ||
      last_sampletime == -1 ||
      (last_sampletime < now && last_end <= now)) {

    // no cache, or
    // first call to read, or
    // cache completely expired

    if (last_buffer != 0)
      delete last_buffer;

    last_sampletime = now;
    last_buffer = new SampleBuf(buffer->getLength());
    last_result = (voice->*(output.getSampleFn()))(output, last_buffer);

  } else {

    // we have a cache, and
    // it has been used before, and
    // it may be partially expired, or
    //		 all valid but too short, or
    //		 all valid and just right.

    if (last_sampletime < now) {
      // partially expired.
      last_buffer->delete_front(now - last_sampletime);
      last_sampletime = now;
    }

    // Now the first sample in the cache is valid.

    int oldlen = last_buffer->getLength();
    int newlen = buffer->getLength();

    if (oldlen < newlen) {
      // ... but the cache is too short. Fill up the remainder.

      if (!last_result)
	last_buffer->clear();

      last_buffer->resize(newlen);

      last_buffer->wind(oldlen);	// urrk
      last_result = (voice->*(output.getSampleFn()))(output, last_buffer);
      last_buffer->wind(-oldlen);
    }

  }

  // At this point, last_buffer contains a valid set of cached
  // samples.

  if (last_result)
    *buffer = *last_buffer;

  return last_result;
}

Generator::Generator(GeneratorClass &_cls, bool _polyphonic, int _nvoices = DEFAULT_POLYPHONY)
  : cls(_cls),
    inputs(cls.getNumInputs()),
    outputs(cls.getNumOutputs()),
    polyphonic(_polyphonic)
{
  setPolyphony(_nvoices);
}

Generator::~Generator() {
  int numIn = cls.getNumInputs();
  int numOut = cls.getNumOutputs();

  for (int i = 0; i < numIn; i++) {
    while (!inputs[i].empty()) inputs[i].front()->unlink();
  }

  for (int i = 0; i < numOut; i++) {
    while (!outputs[i].empty()) outputs[i].front()->unlink();
  }

  for (statevec_t::iterator i = voices.begin(); i != voices.end(); i++) {
    delete (*i);
  }
}

Generator *Generator::clone() {
  return new Generator(cls, polyphonic, voices.size());
}

Conduit *Generator::find_link(OutputDescriptor const *src_q, Generator *dst, InputDescriptor const *dst_q) {
  // paranoia
  assert(src_q->getGeneratorClass() == &cls);
  assert(dst_q->getGeneratorClass() == &dst->cls);
  assert(dst->cls.getNumInputs() == dst->inputs.size());
  assert(cls.getNumOutputs() == outputs.size());

  Conduit templ(this, src_q, dst, dst_q);
  conduitlist_t &v = outputs[src_q->getInternalIndex()];
  conduitlist_t::iterator i = v.begin();

  while (i != v.end()) {
    Conduit *c = *i;
    if (templ == *c) return c;
    i++;
  }

  return NULL;
}

void Generator::link(OutputDescriptor const &src_q, Generator *dst, InputDescriptor const &dst_q) {
  Conduit *c = find_link(&src_q, dst, &dst_q);

  if (c != NULL)
    return;

  if (!src_q.compatibleWith(&dst_q)) {
    // No overlap in capabilities - fail.
    return;
  }

  c = new Conduit(this, &src_q, dst, &dst_q);
  outputs[src_q.getInternalIndex()].push_back(c);
  dst->inputs[dst_q.getInternalIndex()].push_back(c);
}

void Generator::unlink(OutputDescriptor const &src_q,
		       Generator *dst, InputDescriptor const &dst_q) {
  Conduit *c = find_link(&src_q, dst, &dst_q);
  conduitlist_t::iterator i;
  int out_index = src_q.getInternalIndex();
  int in_index = dst_q.getInternalIndex();

  RETURN_UNLESS(c != NULL);

  outputs[out_index].remove(c);
  dst->inputs[in_index].remove(c);

  delete c;
}

Generator::conduitlist_t const &Generator::inboundLinks(InputDescriptor const &q) {
  assert(q.getGeneratorClass() == &cls);
  return inputs[q.getInternalIndex()];
}

Generator::conduitlist_t const &Generator::outboundLinks(OutputDescriptor const &q) {
  assert(q.getGeneratorClass() == &cls);
  return outputs[q.getInternalIndex()];
}

void Generator::addInput() {
  inputs.push_back(conduitlist_t());
}

void Generator::addOutput() {
  outputs.push_back(conduitlist_t());

  for (cachevec_t::iterator i = caches.begin(); i != caches.end(); i++)
    (*i).push_back(SampleCache());
}

void Generator::removeInput(int index) {
  inputs.erase(&inputs[index]);
}

void Generator::removeOutput(int index) {
  outputs.erase(&outputs[index]);

  for (cachevec_t::iterator i = caches.begin(); i != caches.end(); i++)
    (*i).erase(&(*i)[index]);
}

bool Generator::aggregate_input(RealtimeInputDescriptor const &q, int voice, SampleBuf *buffer) {
  int index = q.getInternalIndex();

  // paranoia
  RETURN_VAL_UNLESS(q.getGeneratorClass() == &cls, false);

  conduitlist_t &lst = inputs[index];

  // Questionable optimisation? list<>::size() is linear-time...
  if (lst.size() == 1) {
    Conduit *c = lst.front();
    RealtimeOutputDescriptor const *output =
      dynamic_cast<RealtimeOutputDescriptor const *>(c->src_q);
    return c->src->read_output(*output, voice, buffer);
  }

  SampleBuf tmp(buffer->getLength());
  bool result = false;

  for (conduitlist_t::iterator i = lst.begin(); i != lst.end(); i++) {
    Conduit *c = (*i);
    RealtimeOutputDescriptor const *output =
      dynamic_cast<RealtimeOutputDescriptor const *>(c->src_q);

    if (c->src->read_output(*output, voice, &tmp)) {
      *buffer += tmp;
      result = true;
    }
  }

  return result;
}

bool Generator::read_input(RealtimeInputDescriptor const &q, int voice, SampleBuf *buffer) {
  if (polyphonic) {
    return aggregate_input(q, voice, buffer);
  } else {
    SampleBuf tmp(buffer->getLength());
    bool result = false;

    for (int i = 0; i < voices.size(); i++) {
      if (aggregate_input(q, i, &tmp)) {
	*buffer += tmp;
	result = true;
      }
    }

    return result;
  }
}

sampletime_t Generator::get_input_range(RandomaccessInputDescriptor const &q, int voice) {
  Conduit *c;
  int index = q.getInternalIndex();

  // paranoia
  RETURN_VAL_UNLESS(q.getGeneratorClass() == &cls, 0);

  if (!polyphonic) {
    // %%% Err, "it doesn't make sense to ask for a range for an input
    // to a monophonic device, when the input is potentially coming
    // from a polyphonic device."  I know, I know, I'm going to have
    // to revisit the whole idea of linkage between generators.
    return 0;
  }

  if (inputs[index].empty())
    return 0;

  c = inputs[index].front();
  RandomaccessOutputDescriptor const *output =
    dynamic_cast<RandomaccessOutputDescriptor const *>(c->src_q);
  return c->src->get_output_range(*output, voice);
}

bool Generator::read_input(RandomaccessInputDescriptor const &q, int voice,
			   sampletime_t offset, SampleBuf *buffer) {
  // paranoia
  RETURN_VAL_UNLESS(q.getGeneratorClass() == &cls, false);

  int index = q.getInternalIndex();

  if (inputs[index].empty())
    return false;

  Conduit *c = inputs[index].front();
  RandomaccessOutputDescriptor const *output =
    dynamic_cast<RandomaccessOutputDescriptor const *>(c->src_q);

  return read_output(*output, voice, offset, buffer);
}

bool Generator::read_output(RealtimeOutputDescriptor const &q, int voice, SampleBuf *buffer) {
  // paranoia
  RETURN_VAL_UNLESS(q.getGeneratorClass() == &cls, false);

  int index = q.getInternalIndex();

  if (!polyphonic) {
    // If we're monophonic, always return the output of voice zero, the only voice active.
    voice = 0;
  }

  // paranoia
  assert(voice >= 0 && voice < voices.size());
  assert(!outputs[index].empty());

  if (outputs[index].size() == 1) {
    // Don't bother caching the only output.
    return (voices[voice]->*(q.getSampleFn()))(q, buffer);
  } else {
    return caches[voice][index].read(voices[voice], q, buffer);
  }
}

bool Generator::read_output(RandomaccessOutputDescriptor const &q, int voice,
			    sampletime_t offset, SampleBuf *buffer) {
  // paranoia
  RETURN_VAL_UNLESS(q.getGeneratorClass() == &cls, false);

  RandomaccessOutputDescriptor::samplefn_t fn = q.getSampleFn();

  return (voices[voice]->*fn)(q, offset, buffer);
}

sampletime_t Generator::get_output_range(RandomaccessOutputDescriptor const &q, int voice) {
  int index = q.getInternalIndex();
  RandomaccessOutputDescriptor::rangefn_t fn;

  // paranoia
  RETURN_VAL_UNLESS(q.getGeneratorClass() == &cls, 0);

  fn = q.getRangeFn();

  if (fn != 0)
    return (voices[voice]->*fn)(q);
  else {
    g_warning("Generator %s does not implement get_range", cls.getFullpath().c_str());
    return 0;
  }
}

void Generator::setPolyphony(int nvoices) {
  if (!polyphonic)
    nvoices = 1;

  int numOut = cls.getNumOutputs();

  for (int i = voices.size(); i < nvoices; i++) {
    voices.push_back(cls.instantiate(*this, i));
    caches.push_back(vector<SampleCache>(numOut));
  }

  while (voices.size() > nvoices) {
    int i = voices.size();
    delete voices[i];
    voices.pop_back();
    caches.pop_back();
  }
}
