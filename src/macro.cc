#include <string.h>
#include "macro.h"

GALAN_USE_NAMESPACE

GeneratorClass *Macro::realtimeOutputProxyClass;
GeneratorClass *Macro::randomaccessOutputProxyClass;

GeneratorClass *MacroInputProxy::realtimeClass;
GeneratorClass *MacroInputProxy::randomaccessClass;

void Macro::initialise() {
  realtimeOutputProxyClass = new GeneratorClass(0);
  randomaccessOutputProxyClass = new GeneratorClass(0);

  realtimeOutputProxyClass->register_desc(new RealtimeInputDescriptor("Main"));
  randomaccessOutputProxyClass->register_desc(new RandomaccessInputDescriptor("Main"));

  MacroInputProxy::initialise();
}

void MacroInputProxy::initialise() {
  realtimeClass = new GeneratorClass(0);
  randomaccessClass = new GeneratorClass(0);

  realtimeClass->register_desc(new RealtimeOutputDescriptor("Main", 0));
  randomaccessClass->register_desc(new RandomaccessOutputDescriptor("Main", 0, 0));
}

Macro::Macro(bool _polyphonic, int _nvoices = DEFAULT_POLYPHONY)
  : cls(),
    Generator(cls, _polyphonic, _nvoices)
{
}

Macro::~Macro() {
}

void Macro::register_desc(InputDescriptor *input) {
  cls.register_desc(input);
  Generator::addInput();
}

void Macro::register_desc(OutputDescriptor *output) {
  cls.register_desc(output);
  addOutput();
}

void Macro::unregister_desc(InputDescriptor *input) {
  removeInput(input->getInternalIndex());
  cls.unregister_desc(input);
}

void Macro::unregister_desc(OutputDescriptor *output) {
  removeOutput(output->getInternalIndex());
  cls.unregister_desc(output);
}

bool Macro::addChild(string const &name, Generator *child) {
  return children.bind(name, child, false);
}

Generator *Macro::findChild(string const &name) {
  return dynamic_cast<Generator *>(children.lookup(name));
}

bool Macro::removeChild(string const &name) {
  return children.unbind(name);
}

Generator *Macro::addInput(RealtimeInputDescriptor *input, bool trackable = true) {
  string const &name = input->getName();

  if (findChild(name))
    return 0;

  register_desc(input);

  Generator *result = new MacroInputProxy(*MacroInputProxy::realtimeClass,
					  *this, input, 0, trackable);
  addChild(name, result);
  return result;
}

Generator *Macro::addInput(RandomaccessInputDescriptor *input) {
  string const &name = input->getName();

  if (findChild(name))
    return 0;

  register_desc(input);

  Generator *result = new MacroInputProxy(*MacroInputProxy::randomaccessClass,
					  *this, 0, input, false);
  addChild(name, result);
  return result;
}

Generator *Macro::addRealtimeOutput(string const &name) {
  if (findChild(name))
    return 0;

  RealtimeOutputDescriptor *desc = new RealtimeOutputDescriptor(name, 0);
  register_desc(desc);

  Generator *result = new Generator(*realtimeOutputProxyClass, isPolyphonic(), getNumVoices());
  addChild(name, result);
  return result;
}

Generator *Macro::addRandomaccessOutput(string const &name) {
  if (findChild(name))
    return 0;

  RandomaccessOutputDescriptor *desc = new RandomaccessOutputDescriptor(name, 0, 0);
  register_desc(desc);

  Generator *result = new Generator(*randomaccessOutputProxyClass, isPolyphonic(), getNumVoices());
  addChild(name, result);
  return result;
}

bool Macro::read_output(RealtimeOutputDescriptor const &q, int voice, SampleBuf *buffer) {
  static RealtimeInputDescriptor const &main = realtimeOutputProxyClass->getRealtimeInput("Main");
  return findChild(q.getName())->read_input(main, voice, buffer);
}

bool Macro::read_output(RandomaccessOutputDescriptor const &q, int voice,
			sampletime_t offset, SampleBuf *buffer) {
  static RandomaccessInputDescriptor const &main =
    randomaccessOutputProxyClass->getRandomaccessInput("Main");
  return findChild(q.getName())->read_input(main, voice, offset, buffer);
}

sampletime_t Macro::get_output_range(RandomaccessOutputDescriptor const &q, int voice) {
  static RandomaccessInputDescriptor const &main =
    randomaccessOutputProxyClass->getRandomaccessInput("Main");
  return findChild(q.getName())->get_input_range(main, voice);
}

void Macro::setPolyphony(int nvoices) {
  if (!isPolyphonic())
    nvoices = 1;

  for (RegistryIterator i = children.deep_begin(); i != children.end(); i++) {
    Generator *child = dynamic_cast<Generator *>(*i);
    child->setPolyphony(nvoices);
  }

  Generator::setPolyphony(nvoices);
}

bool MacroInputProxy::read_output(RealtimeOutputDescriptor const &q,
				  int voice, SampleBuf *buffer) {
  bool result = macro.read_input(*rt, voice, buffer);

  if (value == 0) {
    return result;
  }

  if (!result)
    buffer->clear();

  buffer += value;
  return true;
}

bool MacroInputProxy::read_output(RandomaccessOutputDescriptor const &q, int voice,
			sampletime_t offset, SampleBuf *buffer) {
  return macro.read_input(*ra, voice, offset, buffer);
}

sampletime_t MacroInputProxy::get_output_range(RandomaccessOutputDescriptor const &q, int voice) {
  return macro.get_input_range(*ra, voice);
}

void MacroInputProxy::handle_event(IntEvent &event) {
  value = event.getValue();
  notifyViews();
}
