/* Template gAlan plugin file. Please distribute your plugins according to
   the GNU General Public License! (You don't have to, but I'd prefer it.) */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>

#include "galan/global.h"
#include "galan/generator.h"
#include "galan/sample.h"
#include "galan/plugin.h"

GALAN_USE_NAMESPACE

static GeneratorClass *pluginClass;

class Test: public GeneratorState {
public:
  Test(Generator &_gen, int _voice);
  virtual ~Test();

  bool MainOutput(RealtimeOutputDescriptor const &desc, SampleBuf *buf);
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

  pluginClass = new GeneratorClass(&GeneratorStateFactory<Test>, "Main/Test Plugin");

  pluginClass->register_desc(new RealtimeInputDescriptor("Main"));
  pluginClass->register_desc(new RealtimeOutputDescriptor("Main",
							  (RealtimeOutputDescriptor::samplefn_t)
							  &Test::MainOutput));
}
