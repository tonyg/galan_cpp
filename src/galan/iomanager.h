#ifndef Iomanager_H	/* -*- c++ -*- */
#define Iomanager_H

///////////////////////////////////////////////////////////////////////////
// I/O Manager for galan

#include "galan/global.h"

GALAN_BEGIN_NAMESPACE

class IOManager;

///////////////////////////////////////////////////////////////////////////

/**
 * Allows plugins and library-side code access to
 * read/write-availability events on O/S file descriptors. Users of
 * the library should implement a concrete subclass of this class, and
 * register a single instance of it using IOManager::setInstance().
 **/
class IOManager {
public:
  /**
   * Direction of I/O a client is interested in.
   **/
  enum Direction {
    INPUT = 0x01,
    OUTPUT = 0x02
  };

  /**
   * Retrieve the global instance of IOManager. There must have been
   * an instance previously registered via setInstance().
   **/
  static IOManager *instance();

  /**
   * Register the global instance of IOManager. This must only be
   * called once! Throws std::logic_error if called more than once.
   * @param inst the instance to return from calls to instance()
   **/
  static void setInstance(IOManager *inst);

  /**
   * Variables of this type are used to represent registrations with
   * an IOManager.
   * @see addReader()
   * @see addWriter()
   **/
  typedef void *token_t;

  /**
   * Functions of this type act as callbacks from the IOManager to
   * interested parties.
   *
   * @param fd the file descriptor in question
   * @param direction SINGLE EVENT (not bitmask) that has occurred on fd
   * @param token the token representing this handler
   * @param userdata the datum passed in when add() was called
   **/
  typedef void (*handler_t)(int fd, Direction direction, token_t token, void *userdata);

  virtual ~IOManager();

  /**
   * Adds an interested client to this IOManager.
   *
   * @param fd the file descriptor we are interested in
   * @param direction mask of events we're interested in - see enum Direction
   * @param handler handler function to call on I/O availability
   * @param userdata datum to pass in to the handler when we call it
   * @return a token for use in deregistering or recognising this addition
   **/
  virtual token_t add(int fd, Direction direction, handler_t handler, void *userdata) = 0;

  /**
   * Uninstalls a previously registered client.
   * @param token a token as returned by add()
   **/
  virtual void remove(token_t token) = 0;

private:
  static IOManager *_instance;
};

GALAN_END_NAMESPACE

#endif
