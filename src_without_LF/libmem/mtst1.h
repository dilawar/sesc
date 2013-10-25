#ifndef SESCTEST1_H
#define SESCTEST1_H

#include "nanassert.h"

#include "MemRequest.h"
#include "MemObj.h"

// This is an example of Backend utilization. The results are slightly
// different for fread and read. The timing is the same, the only
// difference is the order in which the instructions are scheduled. In
// fread, the memory operations have a higher priority

class DCache : public MemoryObj {
 private:
 protected:

  bool fread() {

    // return false; if you don't want to use fread
    // interface. Then everything would go through read()

    ack(2);
    return true;            // true == I don't need to be called in thread
    // mode through the read() interface
  };

  void read() {
    sleep(1);               // whould sleep the current thread 1 cycle

    // getVaddr(); // to get the virtual address accessed

    // Thread::getTime(); the current task local time

    ack(1);                 // Acknowledge in two cycles
  };

  bool fwrite() {
    ack(1);

    return true;
  };

  void write() {
    ack(0);
  };

  void ResetCounters() {};

 public:
};

#endif   // SESCTEST1_H
