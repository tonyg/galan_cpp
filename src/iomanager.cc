#include <stdexcept>

#include "iomanager.h"

GALAN_USE_NAMESPACE

IOManager *IOManager::_instance = 0;

IOManager *IOManager::instance() {
  if (_instance == 0) {
    throw std::logic_error("IOManager::instance() called before IOManager::setInstance()");
  }
  return _instance;
}

void IOManager::setInstance(IOManager *inst) {
  if (_instance != 0) {
    throw std::logic_error("IOManager::setInstance() called more than once");
  }
  _instance = inst;
}
