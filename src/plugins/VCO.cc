/* VCO plugin */

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

static GeneratorClass *pluginClass;

class VCO: public GeneratorState {
public:
  VCO(Generator &_gen, int _voice);
  virtual ~VCO();

  bool MainOutput(RealtimeOutputDescriptor const &desc, SampleBuf *buf);

private:
  static void initialise_table();

  static Sample table[Sample::rate];
  static bool table_initialised;

  double phase;
};

Sample VCO::table[Sample::rate];
bool VCO::table_initialised = false;

VCO::VCO(Generator &_gen, int _voice)
  : GeneratorState(_gen, _voice),
    phase(0)
{
  initialise_table();
}

VCO::~VCO() {
}

inline Sample min(Sample a, Sample b) { return (a < b) ? a : b; }
inline Sample max(Sample a, Sample b) { return (a > b) ? a : b; }

bool VCO::MainOutput(RealtimeOutputDescriptor const &desc, SampleBuf *buf) {
  static RealtimeInputDescriptor const &freq = pluginClass->getRealtimeInput("Frequency");
  static RealtimeInputDescriptor const &trig = pluginClass->getRealtimeInput("PhaseResetTrigger");

  SampleBuf trigger(buf->getLength());
  read_input(trig, &trigger);

#if 0
  // Proper code for variable frequency.
  //
  if (read_input(freq, buf)) {
    for (int i = 0; i < buf->getLength(); i++) {
      Sample phaseDelta = min(max((*buf)[i], 0), Sample::rate >> 1);
      (*buf)[i] = table[(int) phase];

      if (trigger[i]) {
	phase = 0;
      } else {
	phase += phaseDelta;
	if (phase >= Sample::rate)
	  phase -= Sample::rate;
      }
    }
  } else {
    buf->fill_with(table[(int) phase]);
  }
#else
  // Hacky duplicate code fixed at 110Hz, 0.1 volume (for testing)
  // (before we have proper controls available)
  //
  for (int i = 0; i < buf->getLength(); i++) {
    Sample phaseDelta = 110;
    (*buf)[i] = table[(int) phase] * 0.1;

    if (trigger[i]) {
      phase = 0;
    } else {
      phase += phaseDelta;
      if (phase >= Sample::rate)
	phase -= Sample::rate;
    }
  }
#endif

  return true;
}

void VCO::initialise_table() {
  if (table_initialised)
    return;

  double const rad_per_sample = 2.0 * M_PI / Sample::rate;

  for (int i = 0; i < Sample::rate; i++)
    table[i] = sin(i * rad_per_sample);

  table_initialised = 1;
}

PUBLIC_SYMBOL void init_plugin_VCO(Plugin &plugin) {
  plugin.registerPlugin("Tony Garnock-Jones", "VCO Plugin", "1.0",
			"This plugin is a (very) simple sine-wave oscillator.");

  pluginClass = new GeneratorClass("Variable Sine Oscillator",
				   &GeneratorStateFactory<VCO>,
				   "Sources/Variable Sine Oscillator");

  pluginClass->register_desc(new RealtimeInputDescriptor("Frequency"));
  pluginClass->register_desc(new RealtimeInputDescriptor("PhaseResetTrigger"));
  pluginClass->register_desc(new RealtimeOutputDescriptor("Main",
							  (RealtimeOutputDescriptor::samplefn_t)
							  &VCO::MainOutput));
}
