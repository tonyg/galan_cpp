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

static GeneratorClass *pluginClass;

class TEMPLATE: public Generator {
public:
  TEMPLATE();
  virtual ~TEMPLATE();

  bool MainOutput(SampleBuf *buf);

  virtual GeneratorClass *getClass() { return pluginClass; }
  static Generator *factory() {
    return new TEMPLATE();
  }
};

TEMPLATE::TEMPLATE(): Generator() {
}

TEMPLATE::~TEMPLATE() {
}

bool TEMPLATE::MainOutput(SampleBuf *buf) {
  return false;
}

PUBLIC_SYMBOL void init_plugin_TEMPLATE(void) {
  pluginClass = new GeneratorClass("Misc/TEMPLATE", TEMPLATE::factory);
  pluginClass->register_desc(new InputDescriptor("Main", SIG_FLAG_REALTIME));
  pluginClass->register_desc(new OutputDescriptor("Main", SIG_FLAG_REALTIME,
						  (OutputDescriptor::realtime_samplefn_t)
						  &TEMPLATE::MainOutput));
}
