#include "galan/model.h"

GALAN_USE_NAMESPACE

View::View() {
}

void View::modelChanged(Model &model) {
}

Model::Model()
  : dependents(),
    freezeCount(0),
    frozenNotify(false)
{
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
  if (freezeCount) {
    frozenNotify = true;
  } else {
    dependents_t::iterator i = dependents.begin();
    while (i != dependents.end()) {
      View *v = (*i);
      ++i;
      v->modelChanged(*this);
    }
  }
}

int Model::freeze() {
  return ++freezeCount;
}

int Model::thaw() {
  freezeCount--;
  if (freezeCount < 0)
    freezeCount = 0;

  if ((freezeCount == 0) && frozenNotify) {
    frozenNotify = false;
    notifyViews();
  }

  return freezeCount;
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
