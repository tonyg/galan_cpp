/* MacOS X CoreAudio Output Plugin */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <assert.h>

#include <CoreAudio/AudioHardware.h>

#include "galan/global.h"
#include "galan/generator.h"
#include "galan/sample.h"
#include "galan/plugin.h"
#include "galan/clock.h"
#include "galan/iomanager.h"

#include <set>

GALAN_USE_NAMESPACE
using namespace std;

class MacOutputClock;
class MacOutput;

typedef set<Generator *> generatorSet_t;

///////////////////////////////////////////////////////////////////////////

static GeneratorClass *pluginClass;
static MacOutputClock *outputClock = 0;
static generatorSet_t allOutputs;

///////////////////////////////////////////////////////////////////////////

class MacOutputClock: public Clock, public RealtimeHandler {
public:
  MacOutputClock();
  virtual ~MacOutputClock();

  virtual string const &getName() const {
    static string const name = "Macintosh CoreAudio Output Clock";
    return name;
  }

  virtual void disable();
  virtual void enable();

  virtual void realtime_elapsed(sampletime_t delta);

  OSStatus io_callback(AudioDeviceID inDevice, AudioTimeStamp const *inNow,
		       AudioBufferList const *inInputData, AudioTimeStamp const *inInputTime,
		       AudioBufferList *outOutputData, AudioTimeStamp const *inOutputTime);

private:
  bool initialised;
  AudioDeviceID device;
  UInt32 deviceBufferSize;
  AudioStreamBasicDescription deviceFormat;

  bool registered;
  float *out_buffer;

  void play_fragment(SampleBuf const &left, SampleBuf const &right);
};

MacOutputClock::MacOutputClock()
  : Clock(),
    initialised(false),
    device(kAudioDeviceUnknown),
    deviceBufferSize(0),
    deviceFormat(),
    registered(false),
    out_buffer(0)
{
  size_t count;
  OSStatus err = kAudioHardwareNoError;

  count = sizeof(device);
  err = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice,
				 &count, (void *) &device);
  if (kAudioHardwareNoError != err) {
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
	  "AudioHardwareGetProperty returned error code %ld",
	  err);
  } else {
    // Have probed successfully for default output device.

    count = sizeof(deviceBufferSize);
    err = AudioDeviceGetProperty(device, 0, false, kAudioDevicePropertyBufferSize,
				 &count, &deviceBufferSize);

    if (kAudioHardwareNoError != err) {
      g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
	    "AudioDeviceGetProperty for buffer size returned error code %ld",
	    err);
    } else {
      // Have probed the device for its buffer size successfully.

      count = sizeof(deviceFormat);
      err = AudioDeviceGetProperty(device, 0, false, kAudioDevicePropertyStreamFormat,
				   &count, &deviceFormat);
      if (kAudioHardwareNoError != err) {
	g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
	      "AudioDeviceGetProperty for stream format returned error code %ld",
	      err);
      } else {
	// Have probed the device for its stream format successfully.

	if (deviceFormat.mFormatID != kAudioFormatLinearPCM ||
	    !(deviceFormat.mFormatFlags & kLinearPCMFormatFlagIsFloat)) {
	  g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
		"Audio stream is in wrong format");
	} else {
	  // Initialised successfully.
	  initialised = true;
	}
      }
    }
  }

  if (initialised) {
    ClockManager::instance()->register_clock(this);
    Clock::register_realtime_fn(this);
  }
}

MacOutputClock::~MacOutputClock() {
  if (initialised) {
    Clock::deregister_realtime_fn(this);
    ClockManager::instance()->deregister_clock(this);
    initialised = false;
  }
}

static OSStatus io_proc(AudioDeviceID inDevice, AudioTimeStamp const *inNow,
			AudioBufferList const *inInputData, AudioTimeStamp const *inInputTime,
			AudioBufferList *outOutputData, AudioTimeStamp const *inOutputTime,
			void *userdata)
{
  return ((MacOutputClock *) userdata)->io_callback(inDevice, inNow,
						    inInputData, inInputTime,
						    outOutputData, inOutputTime);
}

void MacOutputClock::disable() {
  if (!initialised) return;
  if (!registered) return;

  OSStatus err = kAudioHardwareNoError;

  err = AudioDeviceStop(device, io_proc);
  if (err != kAudioHardwareNoError) {
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
	  "Could not stop audio device: %ld\n",
	  err);
    return;
  }

  err = AudioDeviceRemoveIOProc(device, io_proc);
  if (err != kAudioHardwareNoError) {
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
	  "Could not remove IO proc: %ld\n",
	  err);
    return;
  }

  registered = false;
}

void MacOutputClock::enable() {
  if (!initialised) return;
  if (registered) return;

  OSStatus err = kAudioHardwareNoError;

  err = AudioDeviceAddIOProc(device, io_proc, (void *) this);
  if (err != kAudioHardwareNoError) {
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
	  "Could not install IO proc: %ld\n",
	  err);
    return;
  }

  err = AudioDeviceStart(device, io_proc);
  if (err != kAudioHardwareNoError) {
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL,
	  "Could not start audio device: %ld\n",
	  err);
    return;
  }

  registered = true;
}

void MacOutputClock::realtime_elapsed(sampletime_t delta) {
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

    if (gen->mute())
      continue;

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

void MacOutputClock::play_fragment(SampleBuf const &left, SampleBuf const &right) {
  int length = left.getLength();
  assert(length == right.getLength());

  for (int i = 0; i < length; i++) {
    *(out_buffer++) = (float) ((Sample::value_t) left[i]);
    *(out_buffer++) = (float) ((Sample::value_t) right[i]);
  }
}

OSStatus MacOutputClock::io_callback(AudioDeviceID inDevice,
				     AudioTimeStamp const *inNow,
				     AudioBufferList const *inInputData,
				     AudioTimeStamp const *inInputTime,
				     AudioBufferList *outOutputData,
				     AudioTimeStamp const *inOutputTime)
{
  int nSamples = deviceBufferSize / deviceFormat.mBytesPerFrame;

  out_buffer = (float *) outOutputData->mBuffers[0].mData;
  Clock::advance(nSamples);

  return kAudioHardwareNoError;
}

///////////////////////////////////////////////////////////////////////////

class MacOutput: public GeneratorState {
public:
  MacOutput(Generator &_gen, int _voice);
  virtual ~MacOutput();
};

MacOutput::MacOutput(Generator &_gen, int _voice): GeneratorState(_gen, _voice) {
  if (outputClock == 0) {
    outputClock = new MacOutputClock();
  }

  allOutputs.insert(&getInstance());
}

MacOutput::~MacOutput() {
  allOutputs.erase(&getInstance());

  if (allOutputs.empty()) {
    delete outputClock;
    outputClock = 0;
  }
}

///////////////////////////////////////////////////////////////////////////

PUBLIC_SYMBOL void init_plugin_MacOutput(Plugin &plugin) {
  plugin.registerPlugin("Tony Garnock-Jones", "Macintosh CoreAudio Output Plugin", "1.0",
			"Plays sound via the Carbon CoreAudio interface");

  pluginClass = new GeneratorClass("CoreAudio Output",
				   &GeneratorStateFactory<MacOutput>,
				   "Output/CoreAudio");

  pluginClass->register_desc(new RealtimeInputDescriptor("Main"));	// mono output
  pluginClass->register_desc(new RealtimeInputDescriptor("Left"));
  pluginClass->register_desc(new RealtimeInputDescriptor("Right"));
}
