#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "qtio.h"

GALAN_USE_NAMESPACE

void QtIOManager::initialise() {
  IOManager::setInstance(new QtIOManager());
}

IOManager::token_t QtIOManager::add(int fd,
				    Direction direction,
				    handler_t handler,
				    void *userdata)
{
  entry_t *e = new entry_t;
  e->fd = fd;
  e->reader = 0;
  e->writer = 0;
  e->handler = handler;
  e->userdata = userdata;

  entries[fd] = e;

  if (direction & IOManager::INPUT) {
    e->reader = new QSocketNotifier(fd, QSocketNotifier::Read);
    QObject::connect(e->reader, SIGNAL(activated(int)), this, SLOT(readAvailable(int)));
  }

  if (direction & IOManager::OUTPUT) {
    e->writer = new QSocketNotifier(fd, QSocketNotifier::Write);
    QObject::connect(e->writer, SIGNAL(activated(int)), this, SLOT(writeAvailable(int)));
  }

  return (token_t) e;
}

void QtIOManager::remove(token_t token) {
  entry_t *e = (entry_t *) token;

  delete e->reader;
  delete e->writer;

  entries.erase(e->fd);
  delete e;
}

void QtIOManager::readAvailable(int fd) {
  entry_t *e = entries[fd];
  e->handler(fd, IOManager::INPUT, (token_t) e, e->userdata);
}

void QtIOManager::writeAvailable(int fd) {
  entry_t *e = entries[fd];
  e->handler(fd, IOManager::OUTPUT, (token_t) e, e->userdata);
}
