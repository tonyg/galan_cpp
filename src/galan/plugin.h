#ifndef Plugin_H	/* -*- c++ -*- */
#define Plugin_H

#include "galan/registry.h"
#include "galan/ref.h"

#include <string>
#include <vector>
#include <utility>

#include <gmodule.h>

GALAN_BEGIN_NAMESPACE

/**
 * Informative class about a plugin. Holds a reference to the plugin's
 * GModule along with some other useful information about it.
 **/
class Plugin: public Registrable {
public:
  /**
   * Instances of this struct are returned (as a loaderResult_t) by
   * Plugin::loadPlugins(), for use in indicating to the user which
   * plugins had problems loading, and what those problems were.
   **/
  struct faultyPlugin {
    std::string pathName;	///< Path to the faulty plugin
    std::string pluginName;	///< Short name for the plugin
    std::string detail;		///< Detailed error text
    faultyPlugin(std::string const &_path, std::string const &_name, std::string const &_detail)
      : pathName(_path), pluginName(_name), detail(_detail)
    {}
  };
  typedef std::vector<faultyPlugin> loaderResult_t;	///< @see Plugin::loadPlugins()

  /**
   * Function signature for plugin initializer entry points.
   **/
  typedef void (*plugin_callback_t)(Plugin &plugin);

  /**
   * Main entry point - loads all plugins, first from either
   * $GALAN_PLUGIN_DIR, if specified, otherwise $(pkglibdir)/plugins,
   * and second from $HOME/.galan/plugins (or as specified by the
   * HOMEDIR_SUFFIX macro in plugin.cc).
   *
   * To be valid and loadable, a plugin must have an exported
   * initialiser function of the form INITFUNC_PREFIX+pluginName,
   * eg. "init_plugin_foobar", which is of the form plugin_callback_t.
   **/
  static loaderResult_t loadPlugins();

  static Registry *registry;		///< Keeps a record of all loaded plugins.

  Plugin(Plugin const &from) { assign(from); }
  Plugin &operator=(Plugin const &from) { assign(from); }

  ~Plugin() {
    unbind();
    g_module_close(handle);
  }

  /** @name Accessors
   * Simple accessors. */
  //@{
  std::string const &getFilename() const { return filename; }
  std::string const &getPluginname() const { return pluginname; }
  std::string const &getAuthor() const { return author; }
  std::string const &getTitle() const { return title; }
  std::string const &getVersion() const { return version; }
  std::string const &getDescription() const { return description; }
  //@}

  /// Called by the plugin to supply needed information.
  void registerPlugin(std::string const &_author, std::string const &_title,
		      std::string const &_version, std::string const &_description);

private:
  GModule *handle;		///< handle to the loaded plugin
  std::string filename;		///< filename the plugin was loaded from
  std::string pluginname;	///< shortname of the plugin

  std::string author;		///< supplied by the plugin
  std::string title;		///< supplied by the plugin
  std::string version;		///< supplied by the plugin
  std::string description;	///< supplied by the plugin

  Plugin();	// unimpl.
  Plugin(GModule *h, std::string const &fn, std::string const &pn)
    : handle(h),
      filename(fn),
      pluginname(pn)
  {}

  /// Internal: called from loadAllPlugins
  static void loadPlugin(loaderResult_t &result,
			 std::string const &plugin,
			 std::string const &leafname);

  /**
   * Loads all plugins in the named directory and all (non-'hidden')
   * subdirectories.
   **/
  static void loadAllPlugins(loaderResult_t &result, std::string const &dir);

  /**
   * Checks a given path; if it's a file, returns true; if it's a
   * directory, recurses using loadAllPlugins and returns false.
   **/
  static bool checkPluginValidity(loaderResult_t &result, std::string const &name);

  void assign(Plugin const &from) {
    handle = from.handle;
    filename = from.filename;
    pluginname = from.pluginname;
    author = from.author;
    title = from.title;
    version = from.version;
    description = from.description;
  }
};

GALAN_END_NAMESPACE

#endif
