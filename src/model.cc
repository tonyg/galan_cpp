#include "model.h"

GALAN_USE_NAMESPACE

void Model::addDependent(View &v) {
  dependents.insert(&v);
}

void Model::removeDependent(View &v) {
  dependents.erase(&v);
}

bool Model::hasDependent(View &v) const {
  return (dependents.find(&v) != dependents.end());
}

Model::dependents_t const &Model::getDependents() const {
  return dependents;
}

void Model::notifyViews() {
  dependents_t::iterator i = dependents.begin();
  while (i != dependents.end()) {
    (*i)->modelChanged();
    i++;
  }
}

View::View(Model &_model): model(_model) {
  model.addDependent(*this);
}

View::~View() {
  model.removeDependent(*this);
}
