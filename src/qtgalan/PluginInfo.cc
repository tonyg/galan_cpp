#include "PluginInfo.h"

#include <qlistbox.h>
#include <qlabel.h>
#include <qtextview.h>
#include <qstring.h>
GALAN_USE_NAMESPACE

/* 
 *  Constructs a PluginInfoImpl which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
PluginInfoImpl::PluginInfoImpl( QWidget* parent,  const char* name, bool modal, WFlags fl )
  : PluginInfo( parent, name, modal, fl ),
    allPlugins()
{
  int counter = 0;
  for (Registry::RegistryIterator i = Plugin::registry->deep_begin();
       i != Plugin::registry->end();
       i++) {
    Plugin *p = dynamic_cast<Plugin *>(*i);
    if (p == 0) continue;	// hmm.
    allPlugins.push_back(p);
    PluginList->insertItem(p->getFullpath().c_str());
    counter++;
  }

  QString msg;
  msg.sprintf("%d plugins installed.", counter);
  PluginCountLabel->setText(msg);
}

/*  
 *  Destroys the object and frees any allocated resources
 */
PluginInfoImpl::~PluginInfoImpl()
{
    // no need to delete child widgets, Qt does it all for us
}

/* 
 * protected slot
 */
void PluginInfoImpl::selectionChanged()
{
  int item = PluginList->currentItem();
  if (item == -1)
    return;

  Plugin *p = allPlugins[item];

  TitleLabel->setText(p->getTitle().c_str());
  AuthorLabel->setText(p->getAuthor().c_str());
  VersionLabel->setText(p->getVersion().c_str());
  DescriptionText->setText(p->getDescription().c_str());
}
