#ifndef Model_H	/* -*- c++ -*- */
#define Model_H

///////////////////////////////////////////////////////////////////////////
// Model-View in C++
// Interface

#include <set>
#include <utility>

#include "galan/global.h"

GALAN_BEGIN_NAMESPACE

class Model;
class View;
class ModelLink;

///////////////////////////////////////////////////////////////////////////

/**
 * Observer type. May depend on one or more Model instances. Notified
 * of changes to Models through calls to Model::notifyViews(), which
 * call modelChanged() with the changed model.
 **/
class View: public Destructable {
 public:
  View();

  virtual void modelChanged(Model &model);

 private:
  View(View const &from);		//unimpl
  View &operator =(View const &from);	//unimpl
};

/**
 * Observable object. Has dependent View instances, which are notified
 * of (abstract) changes in thie object's state by calls to
 * notifyViews().
 **/
class Model: public Destructable {
 public:
  typedef std::set<View *> dependents_t;

  Model();

  void addDependent(View &v);			///< Install a new dependent
  void removeDependent(View &v);		///< Remove a dependent
  bool hasDependent(View &v) const;		///< "Is v a dependent of this?"
  bool hasDependents() const;			///< "Does this have *any* deps?"
  dependents_t const &getDependents() const;	///< Retrieve all deps of this

  virtual void notifyViews();			///< Call all dependent callbacks.

 protected:
  int freeze();
  int thaw();

 private:
  dependents_t dependents;		///< All dependents of this.
  int freezeCount;  ///< Number of times freeze() has been called without a corresponding thaw()
  bool frozenNotify;///< If frozen and notifyViews called, this is set to true for the thaw

  Model(Model const &from);		//unimpl
  Model &operator =(Model const &from);	//unimpl
};

/**
 * Manages the link between a Model and View. On construction,
 * registers the dependency; on destruction, deregisters the
 * dependency.
 **/
class ModelLink {
public:
  ModelLink(Model &m, View &v);
  ~ModelLink();

private:
  Model &model;
  View &view;
};

GALAN_END_NAMESPACE

#endif
