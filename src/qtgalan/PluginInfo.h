// -*- C++ -*-
#ifndef PLUGININFOIMPL_H
#define PLUGININFOIMPL_H
#include "PluginInfo_base.h"

#include "galan/plugin.h"
#include <vector>

class PluginInfoImpl : public PluginInfo
{ 
    Q_OBJECT

public:
    PluginInfoImpl( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~PluginInfoImpl();

protected slots:
    void selectionChanged();

private:
    std::vector<Galan::Plugin *> allPlugins;
};

#endif // PLUGININFOIMPL_H
