// -*- C++ -*-
#ifndef galan_QTIO_H
#define galan_QTIO_H

#include "galan/iomanager.h"

#include <map>

#include <qobject.h>
#include <qsocketnotifier.h>

class QtIOManager: public QObject, public Galan::IOManager {
  Q_OBJECT
public:
  static void initialise();

  virtual token_t add(int fd, Direction direction, handler_t handler, void *userdata);
  virtual void remove(token_t token);

private slots:
  void readAvailable(int fd);
  void writeAvailable(int fd);

private:
  struct entry_t {
    int fd;
    QSocketNotifier *reader;
    QSocketNotifier *writer;
    handler_t handler;
    void *userdata;
  };

  typedef std::map<int, entry_t *> entryMap_t;
  entryMap_t entries;
};

#endif
