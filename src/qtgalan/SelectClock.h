// -*- C++ -*-
#ifndef SELECTCLOCKIMPL_H
#define SELECTCLOCKIMPL_H
#include "SelectClock_base.h"

#include "galan/clock.h"
#include <vector>

class SelectClockImpl : public SelectClock
{ 
    Q_OBJECT

public:
    SelectClockImpl( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SelectClockImpl();

protected slots:
  virtual void accept();

private:
  std::vector<Galan::Clock *> allClocks;
};

#endif // SELECTCLOCKIMPL_H
