#include "variable.h"

static GeneratorClass *variableClass;

Variable::Variable()
  : Generator(),
    value(0)
{
  // %%% Register self with Instrument's list of variables
}

Variable::~Variable() {
  // %%% Unregister self from Instrument's list of variables
}

bool Variable::MainOutput(SampleBuf *buf) {
  buf->fill_with(value);
  return true;
}

GeneratorClass *Variable::getClass() {
  return variableClass;
}

Generator *Variable::factory(string const &name, Named *parent) {
  Instrument *i = dynamic_cast<Instrument *>(parent);

  RETURN_VAL_UNLESS(i != 0, NULL);
  return new Variable(name, i);
}

void Variable::initialiseClass() {
  variableClass = new GeneratorClass("Glue/Variable", Variable::factory);
  variableClass->register_desc(new OutputDescriptor("Main", SIG_FLAG_REALTIME,
						    (OutputDescriptor::realtime_samplefn_t)
						    &Variable::MainOutput));
}
