#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>

#include "galan/global.h"
#include "galan/generator.h"
#include "galan/sample.h"
#include "galan/plugin.h"
#include "galan/registry.h"
#include "galan/controller.h"

GALAN_USE_NAMESPACE

static GeneratorClass *rtClass;
static GeneratorClass *raClass;

class ControllerData: public View {
public:
  ControllerData(Controller &_c)
    : c(_c),
      ml(c, *this)
  {}

  RealtimeController &rt() { return dynamic_cast<RealtimeController &>(c); }
  RandomaccessController &ra() { return dynamic_cast<RandomaccessController &>(c); }

private:
  Controller &c;
  ModelLink ml;
};

class ControllerVoice: public GeneratorState {
public:
  ControllerData *data() {
    return (ControllerData *) getInstance().userdata();
  }

  ControllerVoice(Generator &_gen, int _voice)
    : GeneratorState(_gen, _voice)
  {
    if (!data()) {
      getInstance().userdata(new ControllerData(*Controller::active_instance()));
    }
  }
};

class rtController: public ControllerVoice {
public:
  rtController(Generator &_gen, int _voice)
    : ControllerVoice(_gen, _voice)
  {}

  bool MainOutput(RealtimeOutputDescriptor const &desc, SampleBuf *buf) {
    Sample v = data()->rt().value();

    if (v == Sample::zero)
      return false;

    buf->fill_with(v);
    return true;
  }
};

class raController: public ControllerVoice {
public:
  raController(Generator &_gen, int _voice)
    : ControllerVoice(_gen, _voice)
  {}

  sampletime_t MainRange(RandomaccessOutputDescriptor const &desc) {
    return data()->ra().buffer().getLength();
  }

  bool MainOutput(RandomaccessOutputDescriptor const &desc,
		  sampletime_t offset,
		  SampleBuf *buffer)
  {
    SampleBuf &input(data()->ra().buffer());
    buffer->assign(input, offset, buffer->getLength());
    return true;
  }
};

PUBLIC_SYMBOL void init_plugin_Controllers(Plugin &plugin) {
  plugin.registerPlugin("Tony Garnock-Jones", "Controllers", "1.0",
			"Provides glue between user-interface controls and "
			"inputs to the synthesiser mesh.");

  rtClass = new GeneratorClass("Realtime Controller", &GeneratorStateFactory<rtController>);
  raClass = new GeneratorClass("Randomaccess Controller", &GeneratorStateFactory<raController>);

  Registry::root->bind("ControllerFactory/Realtime", rtClass);
  Registry::root->bind("ControllerFactory/Randomaccess", raClass);

  rtClass->register_desc
    (new RealtimeOutputDescriptor("Main",
				  (RealtimeOutputDescriptor::samplefn_t)
				  &rtController::MainOutput));

  raClass->register_desc
    (new RandomaccessOutputDescriptor("Main",
				      (RandomaccessOutputDescriptor::rangefn_t)
				      &raController::MainRange,
				      (RandomaccessOutputDescriptor::samplefn_t)
				      &raController::MainOutput));
}
