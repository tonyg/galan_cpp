#ifndef Model_H	/* -*- c++ -*- */
#define Model_H

///////////////////////////////////////////////////////////////////////////
// Model-View in C++
// Interface

#include <set.h>

#include "global.h"

class View;	// forward

class Model {
 private:
  typedef set<View *> dependents_t;
  dependents_t dependents;

  Model(Model const &from) {}
  Model &operator =(Model const &from) {}

 public:
  Model() {}
  virtual ~Model() {}

  virtual void addDependent(View &v);
  virtual void removeDependent(View &v);
  virtual bool hasDependent(View &v) const;
  virtual dependents_t const &getDependents() const;

  virtual void notifyViews();
};

class View {
 private:
  Model &model;

  View(View const &from): model(from.model) {}
  View &operator =(View const &from) {}

 public:
  View(Model &_model);
  virtual ~View();

  virtual void modelChanged() = 0;
};

#endif
