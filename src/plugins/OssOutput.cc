/* OSS Output Plugin */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <fcntl.h>
#include <unistd.h>

#include "galan/global.h"
#include "galan/generator.h"
#include "galan/sample.h"
#include "galan/plugin.h"
#include "galan/clock.h"
#include "galan/iomanager.h"

#include <set>

GALAN_USE_NAMESPACE
using namespace std;

#define DEFAULT_FRAGMENT_EXPONENT	12

class OssOutputClock;
class OssOutput;

typedef set<Generator *> generatorSet_t;

///////////////////////////////////////////////////////////////////////////

static GeneratorClass *pluginClass;
static OssOutputClock *outputClock = 0;
static generatorSet_t allOutputs;
static int audio_fragment_exponent = DEFAULT_FRAGMENT_EXPONENT;

///////////////////////////////////////////////////////////////////////////

class OssOutputClock: public Clock, public RealtimeHandler {
public:
  int audiofd;

  OssOutputClock(string const &dsppath);
  virtual ~OssOutputClock();

  virtual string const &getName() const {
    static string const &name = "OSS Output Clock";
    return name;
  }

  virtual void disable();
  virtual void enable();

  void play_fragment(SampleBuf const &left, SampleBuf const &right);
  virtual void realtime_elapsed(sampletime_t delta);

private:
  bool registered;
  IOManager::token_t input_tag;

  bool open_audiofd(string const &dsppath);
};

OssOutputClock::OssOutputClock(string const &dsppath)
  : Clock(),
    audiofd(-1),
    registered(false),
    input_tag()
{
  if (!open_audiofd(dsppath)
      && audiofd != -1) {
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
	  "OssOutput::open_audiofd failed");
    close(audiofd);
    audiofd = -1;
  }

  ClockManager::instance()->register_clock(this);
}

OssOutputClock::~OssOutputClock() {
  ClockManager::instance()->deregister_clock(this);
  close(audiofd);
}

bool OssOutputClock::open_audiofd(string const &dsppath) {
  int i;

  audiofd = open(dsppath.c_str(), O_WRONLY);
  RETURN_VAL_UNLESS(audiofd != -1, false);

  i = (4 << 16) | audio_fragment_exponent;	/* 4 buffers */
  RETURN_VAL_UNLESS(ioctl(audiofd, SNDCTL_DSP_SETFRAGMENT, &i) != -1, false);

  i = AFMT_S16_LE;
  RETURN_VAL_UNLESS(ioctl(audiofd, SNDCTL_DSP_SETFMT, &i) != -1, false);

  i = 1;
  RETURN_VAL_UNLESS(ioctl(audiofd, SNDCTL_DSP_STEREO, &i) != -1, false);

  i = Sample::rate;
  RETURN_VAL_UNLESS(ioctl(audiofd, SNDCTL_DSP_SPEED, &i) != -1, false);

  return true;
}

static void oss_output_clock_handler(int fd,
				     IOManager::Direction direction,
				     IOManager::token_t token,
				     void *userdata) {
  Clock::advance(Event::mainloop());
}

void OssOutputClock::disable() {
  if (registered) {
    IOManager::instance()->remove(input_tag);
    registered = false;
  }
}

void OssOutputClock::enable() {
  if (!registered) {
    input_tag = IOManager::instance()->add(audiofd,
					   IOManager::OUTPUT,
					   &oss_output_clock_handler,
					   NULL);
    registered = true;
  }
}

void OssOutputClock::play_fragment(SampleBuf const &left, SampleBuf const &right) {
  int length = left.getLength();
  assert(length == right.getLength());

  signed short *outbuf;
  int buflen = length * sizeof(signed short) * 2;
  int i;

  outbuf = (signed short *) malloc(buflen);
  RETURN_UNLESS(outbuf != NULL);

  for (i = 0; i < length; i++) {
    outbuf[i<<1]	= (signed short) MIN(MAX((double) left[i] * 32767, -32768), 32767);
    outbuf[(i<<1) + 1]	= (signed short) MIN(MAX((double) right[i] * 32767, -32768), 32767);
  }

  write(audiofd, outbuf, buflen);
  free(outbuf);
}

void OssOutputClock::realtime_elapsed(sampletime_t delta) {
  static RealtimeInputDescriptor const &Main = pluginClass->getRealtimeInput("Main");
  static RealtimeInputDescriptor const &Left = pluginClass->getRealtimeInput("Left");
  static RealtimeInputDescriptor const &Right = pluginClass->getRealtimeInput("Right");

  SampleBuf left(delta);
  SampleBuf right(delta);
  SampleBuf temp(delta);

  for (generatorSet_t::iterator i = allOutputs.begin();
       i != allOutputs.end();
       i++) {
    Generator *gen = (*i);

    if (gen->read_input(Main, 0, &temp)) {
      left += temp;
      right += temp;
    }

    if (gen->read_input(Left, 0, &temp)) {
      left += temp;
    }

    if (gen->read_input(Right, 0, &temp)) {
      right += temp;
    }
  }

  play_fragment(left, right);
}

///////////////////////////////////////////////////////////////////////////

class OssOutput: public GeneratorState {
public:
  OssOutput(Generator &_gen, int _voice);
  virtual ~OssOutput();
};

OssOutput::OssOutput(Generator &_gen, int _voice): GeneratorState(_gen, _voice) {
  if (outputClock == 0) {
    outputClock = new OssOutputClock("/dev/dsp");
  }

  allOutputs.insert(&getInstance());
}

OssOutput::~OssOutput() {
  allOutputs.erase(&getInstance());

  if (allOutputs.empty()) {
    delete outputClock;
    outputClock = 0;
  }
}

///////////////////////////////////////////////////////////////////////////

PUBLIC_SYMBOL void init_plugin_OssOutput(Plugin &plugin) {
  plugin.registerPlugin("Tony Garnock-Jones", "OSS Output Plugin", "1.0",
			"Sends input data to /dev/dsp");

  pluginClass = new GeneratorClass(&GeneratorStateFactory<OssOutput>, "Output/OSS");

  pluginClass->register_desc(new RealtimeInputDescriptor("Main"));	// mono output
  pluginClass->register_desc(new RealtimeInputDescriptor("Left"));
  pluginClass->register_desc(new RealtimeInputDescriptor("Right"));
}
