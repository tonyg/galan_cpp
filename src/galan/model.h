#ifndef Model_H	/* -*- c++ -*- */
#define Model_H

///////////////////////////////////////////////////////////////////////////
// Model-View in C++
// Interface

#include <set>

#include "galan/global.h"

GALAN_BEGIN_NAMESPACE

class Model;
class View;

///////////////////////////////////////////////////////////////////////////

/**
 * Observable object. Has dependent View instances, which are notified
 * of (abstract) changes in thie object's state by calls to
 * notifyViews().
 **/
class Model {
 public:
  typedef std::set<View *> dependents_t;

  Model() {}
  virtual ~Model() {}

  virtual void addDependent(View &v);			///< Install a new dependent
  virtual void removeDependent(View &v);		///< Remove a dependent
  virtual bool hasDependent(View &v) const;		///< "Is v a dependent of this?"
  virtual dependents_t const &getDependents() const;	///< Retrieve all dependents of this

  virtual void notifyViews();				///< Call View::modelChanged() on deps.

 private:
  dependents_t dependents;		///< All dependents of this.

  Model(Model const &from);		//unimpl
  Model &operator =(Model const &from);	//unimpl
};

class View {
 public:
  View(Model &_model);
  virtual ~View();

  /**
   * Called whenever our Model has changed (indirectly, via
   * Model::notifyViews()).
   **/
  virtual void modelChanged() = 0;

 private:
  Model &model;				///< Our model.

  View(View const &from);		//unimpl
  View &operator =(View const &from);	//unimpl
};

GALAN_END_NAMESPACE

#endif
