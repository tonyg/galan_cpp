// -*- C++ -*-
#ifndef galan_DefaultClock_H
#define galan_DefaultClock_H

#include "galan/clock.h"

#include <qtimer.h>

class DefaultClock: public QObject, Galan::Clock {
  Q_OBJECT
public:
  DefaultClock();
  virtual ~DefaultClock();

  virtual std::string const &getName() const;
  virtual void disable();
  virtual void enable();

private slots:
  void timedOut();

private:
  QTimer timer;
};

#endif
