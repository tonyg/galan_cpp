#include "SelectClock.h"

#include <qlistbox.h>

GALAN_USE_NAMESPACE

using std::cerr;
using std::endl;

/* 
 *  Constructs a SelectClockImpl which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
SelectClockImpl::SelectClockImpl( QWidget* parent,  const char* name, bool modal, WFlags fl )
  : SelectClock( parent, name, modal, fl ),
    allClocks(ClockManager::instance()->begin(),
	      ClockManager::instance()->end())
{
  for (unsigned int i = 0; i < allClocks.size(); i++) {
    ClockList->insertItem(allClocks[i]->getName().c_str());
    if (ClockManager::instance()->getSelected() == allClocks[i]) {
      ClockList->setSelected(ClockList->count() - 1, true);
    }
  }
}

/*  
 *  Destroys the object and frees any allocated resources
 */
SelectClockImpl::~SelectClockImpl()
{
    // no need to delete child widgets, Qt does it all for us
}

void SelectClockImpl::accept() {
  int item = ClockList->currentItem();
  if (item != -1) {
    IFDEBUG(cerr << "Accepting new clock " << allClocks[item]->getName() << endl);
    ClockManager::instance()->select_clock(allClocks[item]);
  }
  SelectClock::accept();
}
