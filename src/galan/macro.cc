#include <string.h>
#include "galan/macro.h"

GALAN_USE_NAMESPACE

GeneratorClass *Macro::realtimeOutputProxyClass;
GeneratorClass *Macro::randomaccessOutputProxyClass;

GeneratorClass *MacroInputProxy::realtimeClass;
GeneratorClass *MacroInputProxy::randomaccessClass;

void Macro::initialise() {
  realtimeOutputProxyClass = new GeneratorClass("Realtime Macro Output Proxy", 0);
  randomaccessOutputProxyClass = new GeneratorClass("Randomaccess Macro Output Proxy", 0);

  realtimeOutputProxyClass->register_desc(new RealtimeInputDescriptor("Main"));
  randomaccessOutputProxyClass->register_desc(new RandomaccessInputDescriptor("Main"));

  MacroInputProxy::initialise();
}

void MacroInputProxy::initialise() {
  realtimeClass = new GeneratorClass("Realtime Macro Input Proxy", 0);
  randomaccessClass = new GeneratorClass("Randomaccess Macro Input Proxy", 0);

  realtimeClass->register_desc(new RealtimeOutputDescriptor("Main", 0));
  randomaccessClass->register_desc(new RandomaccessOutputDescriptor("Main", 0, 0));
}

static GeneratorState *nullStateFactory(Generator &_gen, int _voice) {
  return 0;
}

MacroClass::MacroClass()
  : GeneratorClass("Macro", nullStateFactory)
{}

Macro *Macro::create(bool _polyphonic, int _nvoices) {
  MacroClass *cls = new MacroClass();
  return new Macro(cls, _polyphonic, _nvoices);
}

Macro::Macro(MacroClass *_cls, bool _polyphonic, int _nvoices = DEFAULT_POLYPHONY)
  : Generator(*_cls, _polyphonic, _nvoices),
    cls(_cls)
{
}

Macro::~Macro() {
  delete cls;
}

Generator *Macro::clone() {
  Macro *m = Macro::create(isPolyphonic(), getNumVoices());

  // First copy the children themselves, then copy the links between
  // them.

  for (Registry::iterator i = children.begin();
       i != children.end();
       i++) {
    string name = (*i).first;
    Generator *child = dynamic_cast<Generator *>((*i).second);

    // paranoia
    if (child == 0) {
      throw std::logic_error(std::string("Discovered non-Generator child of Macro at path") +
			     getFullpath());
    }

    MacroInputProxy *proxy = dynamic_cast<MacroInputProxy *>(child);
    if (proxy != 0) {
      // It's a MacroInputProxy.
      if (proxy->rt != 0) m->addRealtimeInput(name);
      if (proxy->ra != 0) m->addRandomaccessInput(name);
    } else {
      // It's a normal Generator (includes Macros, of course), or an output. Clone it!
      m->addChild(name, child->clone());
    }
  }

  // Now the links.

  for (Registry::iterator i = children.begin();
       i != children.end();
       i++) {
    string name = (*i).first;
    Generator *child = dynamic_cast<Generator *>((*i).second);

    vector<OutputDescriptor *> outputDescs = child->getClass().getOutputs();

    for (unsigned int i = 0; i < outputDescs.size(); i++) {
      conduitlist_t const &outs(child->outboundLinks(*outputDescs[i]));

      for (conduitlist_t::const_iterator link = outs.begin();
	   link != outs.end();
	   link++) {
	Conduit *c = (*link);

	Generator *head = m->findChild(name);
	Generator *tail = m->findChild(c->dst->getLocalname());

	head->link(head->getClass().getOutput(c->src_q->getName()),
		   tail,
		   tail->getClass().getInput(c->dst_q->getName()));
      }
    }
  }

  return m;
}

void Macro::register_desc(InputDescriptor *input) {
  cls->register_desc(input);
  Generator::addInput();
}

void Macro::register_desc(OutputDescriptor *output) {
  cls->register_desc(output);
  addOutput();
}

void Macro::unregister_desc(InputDescriptor *input) {
  removeInput(input->getInternalIndex());
  cls->unregister_desc(input);
}

void Macro::unregister_desc(OutputDescriptor *output) {
  removeOutput(output->getInternalIndex());
  cls->unregister_desc(output);
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

bool Macro::renameChild(string const &name,
			string const &newName)
{
  if (children.lookup(newName) != 0) {
    // New name already taken.
    return false;
  }

  Generator *child = findChild(name);
  if (child == 0) {
    // Old name not present.
    return false;
  }

  child->unbind();
  return children.bind(newName, child, false);
}

Generator *Macro::addRealtimeInput(string const &name) {
  if (findChild(name))
    return 0;

  RealtimeInputDescriptor *desc = new RealtimeInputDescriptor(name);
  register_desc(desc);

  Generator *result = new MacroInputProxy(*MacroInputProxy::realtimeClass,
					  *this, desc, 0);
  addChild(name, result);
  return result;
}

Generator *Macro::addRandomaccessInput(string const &name) {
  if (findChild(name))
    return 0;

  RandomaccessInputDescriptor *desc = new RandomaccessInputDescriptor(name);
  register_desc(desc);

  Generator *result = new MacroInputProxy(*MacroInputProxy::randomaccessClass,
					  *this, 0, desc);
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

void Macro::setPolyphony(bool poly) {
  Generator::setPolyphony(poly);

  for (Registry::iterator i = children.begin(); i != children.end(); ++i) {
    Generator *child = dynamic_cast<Generator *>((*i).second);
    child->setNumVoices(poly ? getNumVoices() : 1);
    child->setPolyphony(true);
  }
}

void Macro::setNumVoices(int nvoices) {
  Generator::setNumVoices(nvoices);

  for (Registry::iterator i = children.begin(); i != children.end(); ++i) {
    Generator *child = dynamic_cast<Generator *>((*i).second);
    child->setNumVoices(nvoices);
  }
}

bool MacroInputProxy::read_output(RealtimeOutputDescriptor const &q,
				  int voice, SampleBuf *buffer) {
  bool result = macro.read_input(*rt, voice, buffer);

#if 1
  return result;
#else
  if (value == 0) {
    return result;
  }

  if (!result)
    buffer->clear();

  buffer += value;
  return true;
#endif
}

bool MacroInputProxy::read_output(RandomaccessOutputDescriptor const &q, int voice,
			sampletime_t offset, SampleBuf *buffer) {
  return macro.read_input(*ra, voice, offset, buffer);
}

sampletime_t MacroInputProxy::get_output_range(RandomaccessOutputDescriptor const &q, int voice) {
  return macro.get_input_range(*ra, voice);
}
