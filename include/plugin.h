#ifndef Plugin_H	/* -*- c++ -*- */
#define Plugin_H

#include "registry.h"
#include "ref.h"

#include <gmodule.h>

class Plugin: public Registrable {
public:
  typedef void (*plugin_callback_t)(Plugin &p);

private:
  ref< c_auto_ptr<GModule> > handle;	// handle to the loaded plugin
  string filename;			// filename the plugin was loaded from
  string pluginname;			// shortname of the plugin

  string author;	// supplied by the plugin
  string title;		// supplied by the plugin
  string version;	// supplied by the plugin

  plugin_callback_t aboutCallback;	// supplied by the plugin

  Plugin() {}
  Plugin(GModule *h, string const &fn, string const &pn)
    : handle(new c_auto_ptr<GModule>(h)),
      filename(fn),
      pluginname(pn)
  {}

  static void loadPlugin(string const &plugin, string const &leafname);
  static void loadAllPlugins(string const &dir);
  static bool checkPluginValidity(string const &name);

  void assign(Plugin const &from) {
    handle = from.handle;
    filename = from.filename;
    pluginname = from.pluginname;
    author = from.author;
    title = from.title;
    version = from.version;
    aboutCallback = from.aboutCallback;
  }

public:
  static Registry *registry;

  Plugin(Plugin const &from) { assign(from); }
  Plugin &operator=(Plugin const &from) { assign(from); }

  ~Plugin() {
    unbind();
    g_module_close(handle->get());
  }

  string const &getFilename() const { return filename; }
  string const &getPluginname() const { return pluginname; }
  string const &getAuthor() const { return author; }
  string const &getTitle() const { return title; }
  string const &getVersion() const { return version; }

  void about() { if (aboutCallback) aboutCallback(*this); }

  void registerPlugin(string const &_author, string const &_title,
		      string const &_version, plugin_callback_t const about = 0);

  static void loadPlugins();
};

#endif
