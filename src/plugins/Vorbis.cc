#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "galan/global.h"
#include "galan/generator.h"
#include "galan/sample.h"
#include "galan/plugin.h"

GALAN_USE_NAMESPACE

static GeneratorClass *pluginClass;

class Vorbis: public GeneratorState {
public:
  Vorbis(Generator &_gen, int _voice)
    : GeneratorState(_gen, _voice)
  {}

  sampletime_t MainRange(RandomaccessOutputDescriptor const &desc) {
    return 0;
  }

  bool MainOutput(RandomaccessOutputDescriptor const &desc,
		  sampletime_t offset,
		  SampleBuf *buffer)
  {
    return false;
  }
};

PUBLIC_SYMBOL void init_plugin_Vorbis(Plugin &plugin) {
  plugin.registerPlugin("Tony Garnock-Jones", "Vorbis Plugin", "1.0",
			"Provides a random-access view of an Ogg Vorbis file");

  pluginClass = new GeneratorClass("Vorbis Decoder",
				   &GeneratorStateFactory<Vorbis>,
				   "Sources/Vorbis Decoder");

  pluginClass->
    register_desc(new RandomaccessOutputDescriptor("Main",
						   (RandomaccessOutputDescriptor::rangefn_t)
						   &Vorbis::MainRange,
						   (RandomaccessOutputDescriptor::samplefn_t)
						   &Vorbis::MainOutput));
}
