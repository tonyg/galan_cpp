/* Template gAlan plugin file. Please distribute your plugins according to
   the GNU General Public License! (You don't have to, but I'd prefer it.) */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "global.h"
#include "generator.h"
#include "sample.h"
#include "plugin.h"
#include "msgbox.h"

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

void aboutFn(Plugin &plugin) {
  popup_msgbox("About Test Plugin", MSGBOX_DISMISS, 0, MSGBOX_DISMISS,
	       "This plugin is a stub, do-nothing plugin.");
}

PUBLIC_SYMBOL void init_plugin_Test(Plugin &plugin) {
  plugin.registerPlugin("Author's Name", "Test Plugin", "1.0", aboutFn);

  pluginClass = new GeneratorClass(&Test::factory, "Main/Test Plugin");

  pluginClass->register_desc(new RealtimeInputDescriptor("Main"));
  pluginClass->register_desc(new RealtimeOutputDescriptor("Main",
							  (RealtimeOutputDescriptor::samplefn_t)
							  &Test::MainOutput));
}
