/* Gain plugin */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <math.h>

#include "galan/global.h"
#include "galan/generator.h"
#include "galan/sample.h"
#include "galan/plugin.h"

GALAN_USE_NAMESPACE

static GeneratorClass *gainPluginClass;

class Gain: public GeneratorState {
public:
  Gain(Generator &_gen, int _voice);

  bool MainOutput(RealtimeOutputDescriptor const &desc, SampleBuf *buf);
};

Gain::Gain(Generator &_gen, int _voice)
  : GeneratorState(_gen, _voice)
{}

inline Sample min(Sample a, Sample b) { return (a < b) ? a : b; }
inline Sample max(Sample a, Sample b) { return (a > b) ? a : b; }

bool Gain::MainOutput(RealtimeOutputDescriptor const &desc, SampleBuf *buf) {
  static RealtimeInputDescriptor const &in = gainPluginClass->getRealtimeInput("Signal");
  static RealtimeInputDescriptor const &gain = gainPluginClass->getRealtimeInput("Gain");

  if (!read_input(in, buf)) {
    return false;
  }

  SampleBuf gainBuf(buf->getLength());
  read_input(gain, &gainBuf);

  for (int i = 0; i < buf->getLength(); i++) {
    (*buf)[i] = (*buf)[i] * pow(10, gainBuf[i]);
  }
  return true;
}

PUBLIC_SYMBOL void init_plugin_Gain(Plugin &plugin) {
  plugin.registerPlugin("Tony Garnock-Jones", "Gain Plugin", "1.0",
			"This plugin is a very simple exponential gain.");

  gainPluginClass = new GeneratorClass("Exponential Gain",
				   &GeneratorStateFactory<Gain>,
				   "Levels/Gain");

  gainPluginClass->register_desc(new RealtimeInputDescriptor("Signal"));
  gainPluginClass->register_desc(new RealtimeInputDescriptor("Gain"));
  gainPluginClass->register_desc(new RealtimeOutputDescriptor("Main",
							  (RealtimeOutputDescriptor::samplefn_t)
							  &Gain::MainOutput));
}
