/* Delay plugin */

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

static GeneratorClass *delayPluginClass;

class Delay: public GeneratorState {
public:
  Delay(Generator &_gen, int _voice);

  bool MainOutput(RealtimeOutputDescriptor const &desc, SampleBuf *buf);

private:
  SampleBuf delayBuf;
  int offset;
};

Delay::Delay(Generator &_gen, int _voice)
  : GeneratorState(_gen, _voice),
    delayBuf(0),
    offset(0)
{}

bool Delay::MainOutput(RealtimeOutputDescriptor const &desc, SampleBuf *buf) {
  static RealtimeInputDescriptor const &in = delayPluginClass->getRealtimeInput("Main");
  static RealtimeInputDescriptor const &delay = delayPluginClass->getRealtimeInput("Delay");
  static RealtimeInputDescriptor const &feedback = delayPluginClass->getRealtimeInput("Feedback");

  SampleBuf delayVar(buf->getLength());
  if (!read_input(delay, &delayVar)) {
    // Zero delay setting for entire buffer. Copy from our input.
    return read_input(in, buf);
  } else {
    // Non-zero delay for at least part of the buffer.
    SampleBuf tmpBuf(buf->getLength());
    if (!read_input(in, &tmpBuf))
      tmpBuf.clear();

    SampleBuf feedbackBuf(buf->getLength());
    if (!read_input(feedback, &feedbackBuf))
      feedbackBuf.clear();

    int delaySamples = 0;

    for (int i = 0; i < tmpBuf.getLength(); i++) {
      delaySamples = delayVar[i] * Sample::rate;

      if (delayBuf.getLength() < delaySamples)
	delayBuf.resize(delaySamples);

      if (offset >= delaySamples)
	offset = 0;

      (*buf)[i] = delayBuf[offset];

      Sample tmp = tmpBuf[i] + delayBuf[offset] * feedbackBuf[i];
      delayBuf[offset] = tmp; // old galan delay clipped to limit param here
      offset++;
    }

    delayBuf.resize(delaySamples);
    return true;
  }
}

PUBLIC_SYMBOL void init_plugin_Delay(Plugin &plugin) {
  plugin.registerPlugin("Tony Garnock-Jones", "Delay Plugin", "1.0",
			"This plugin is a very simple delay.");

  delayPluginClass = new GeneratorClass("Delay",
					&GeneratorStateFactory<Delay>,
					"Effects/Delay with feedback");

  delayPluginClass->register_desc(new RealtimeInputDescriptor("Main"));
  delayPluginClass->register_desc(new RealtimeInputDescriptor("Delay"));
  delayPluginClass->register_desc(new RealtimeInputDescriptor("Feedback"));
  delayPluginClass->
    register_desc(new RealtimeOutputDescriptor("Main",
					       (RealtimeOutputDescriptor::samplefn_t)
					       &Delay::MainOutput));
}
