#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "global.h"
#include "gmodule.h"
#include "msgbox.h"
#include "registry.h"
#include "plugin.h"

/* %%% Win32: dirent.h seems to conflict with glib-1.3, so ignore dirent.h */
#ifndef NATIVE_WIN32
#include <dirent.h>
#endif

/* On Win32, these headers seem to need to follow glib.h */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define HOMEDIR_SUFFIX	"/.galan/plugins"
#define INITFUNC_PREFIX "init_plugin_"

Registry *Plugin::registry = new Registry();

void Plugin::registerPlugin(string const &_author, string const &_title,
			    string const &_version, string const &_description)
{
  author = _author;
  title = _title;
  version = _version;
  description = _description;

  IFDEBUG(cerr << "Registering plugin " << title << " (version " << version << ") by "
	  << author <<endl);
  if (!Plugin::registry->bind(title, this)) {
    IFDEBUG(cerr << "Could not register plugin under title " << title << "!" << endl);
  }
}

void Plugin::loadPlugin(loaderResult_t &result, string const &filename, string const &leafname) {
  GModule *handle = g_module_open(filename.c_str(), (enum GModuleFlags) 0);
  Plugin *plugin;
  plugin_callback_t initializer;
  string initstr;
  string pluginname;

  if (handle == NULL) {
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG,
	  "g_module_open(%s, 0) failed: %s", filename.c_str(), g_module_error());
    return;
  }

  /* Strip file suffix - .so or .dll (or, on HPUX .sl, or ...) */
  pluginname = leafname;
  pluginname.erase(pluginname.rfind('.'));

  initstr = string(INITFUNC_PREFIX) + pluginname;

  if (!g_module_symbol(handle, initstr.c_str(), (gpointer *) &initializer)) {
    result.push_back(faultyPlugin(filename, pluginname,
				  "Plugin is missing initializer (" + initstr + ")"));
    g_message("Error finding initializer for plugin %s", leafname.c_str());
    g_module_close(handle);
    return;
  }

  plugin = new Plugin(handle, filename, pluginname);
  initializer(*plugin);

  /* Don't g_module_close(handle) because it will unload the .so */
}

bool Plugin::checkPluginValidity(loaderResult_t &result, string const &name) {
  struct stat sb;

  if (stat(name.c_str(), &sb) == -1)
    return false;

  if (S_ISDIR(sb.st_mode))
    loadAllPlugins(result, name);

  return S_ISREG(sb.st_mode);
}

void Plugin::loadAllPlugins(loaderResult_t &result, string const &dir) {
  DIR *d = opendir(dir.c_str());
  struct dirent *de;

  if (d == NULL) {
    /* the plugin directory cannot be read */
    return;
  }

  while ((de = readdir(d)) != NULL) {
    string fullname;

    if (de->d_name[0] == '.')
      /* Don't load 'hidden' files or directories */
      continue;

    fullname = dir + G_DIR_SEPARATOR_S + de->d_name;

    if (checkPluginValidity(result, fullname))
      loadPlugin(result, fullname, string(de->d_name));
  }

  closedir(d);
}

Plugin::loaderResult_t Plugin::loadPlugins(void) {
  char *home;
  char *plugindir = getenv("GALAN_PLUGIN_DIR");
  loaderResult_t result;

  if (plugindir != NULL)
    loadAllPlugins(result, string(plugindir));
  else
    loadAllPlugins(result, string(SITE_PKGLIB_DIR G_DIR_SEPARATOR_S "plugins"));

  home = getenv("HOME");
  if (home != NULL) {
    loadAllPlugins(result, string(home) + HOMEDIR_SUFFIX);
  }

  return result;
}
