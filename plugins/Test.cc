/* Template gAlan plugin file. Please distribute your plugins according to
   the GNU General Public License! (You don't have to, but I'd prefer it.) */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>

#include "global.h"
#include "generator.h"
#include "sample.h"
#include "plugin.h"

static GeneratorClass *pluginClass;

class Test: public GeneratorState {
public:
  Test(Generator &_gen, int _voice);
  virtual ~Test();

  bool MainOutput(RealtimeOutputDescriptor const &desc, SampleBuf *buf);

  static GeneratorState *factory(Generator &_gen, int _voice) {
    return new Test(_gen, _voice);
  }
};

Test::Test(Generator &_gen, int _voice): GeneratorState(_gen, _voice) {
}

Test::~Test() {
}

bool Test::MainOutput(RealtimeOutputDescriptor const &desc, SampleBuf *buf) {
  static RealtimeInputDescriptor const &main = pluginClass->getRealtimeInput("Main");

  return read_input(main, buf);
}

PUBLIC_SYMBOL void init_plugin_Test(Plugin &plugin) {
  plugin.registerPlugin("Author's Name", "Test Plugin", "1.0",
			"This plugin is a stub, do-nothing plugin.");

  pluginClass = new GeneratorClass(&Test::factory, "Main/Test Plugin");

  pluginClass->register_desc(new RealtimeInputDescriptor("Main"));
  pluginClass->register_desc(new RealtimeOutputDescriptor("Main",
							  (RealtimeOutputDescriptor::samplefn_t)
							  &Test::MainOutput));

  cout << "Test.cc: ClockManager is " << (void *) ClockManager::instance() << endl;
  cout << "Test.cc: &Registry::root is " << (void *) &Registry::root << endl;
}
