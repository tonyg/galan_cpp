#include "galan/model.h"

GALAN_USE_NAMESPACE

View::View() {
}

void View::modelChanged(Model &model) {
}

Model::Model() {
}

void Model::addDependent(View &v) {
  dependents.insert(&v);
}

void Model::removeDependent(View &v) {
  dependents.erase(&v);
}

bool Model::hasDependent(View &v) const {
  return (dependents.find(&v) != dependents.end());
}

bool Model::hasDependents() const {
  return !dependents.empty();
}

Model::dependents_t const &Model::getDependents() const {
  return dependents;
}

void Model::notifyViews() {
  dependents_t::iterator i = dependents.begin();
  while (i != dependents.end()) {
    (*i)->modelChanged(*this);
    i++;
  }
}

ModelLink::ModelLink(Model &m, View &v)
  : model(m),
    view(v)
{
  model.addDependent(view);
}

ModelLink::~ModelLink() {
  model.removeDependent(view);
}
