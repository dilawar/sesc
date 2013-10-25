#if !(defined _Checkpoint_hpp_)
#define _Checkpoint_hpp_

#include "ExecutionFlow.h"
#include "Epoch.h"

namespace tls{

  class Checkpoint{
    // This checkpoint captures the state just before myClock
    ClockValue myClock;
    // Number of epoch that have started but not completed
    // their merge into this checkpoint
    size_t mergingEpochs;
    // List of all active checkpoints, most recent at front
    typedef std::list<class Checkpoint *> CheckpointList;
    static CheckpointList allCheckpoints;
    // Position of this checkpoint in allCheckpoints
    CheckpointList::iterator myPos;
    // Constructs a new checkpoint at time just before myClock
    Checkpoint(ClockValue myClock);

    // Execution sequences of all threads
    class ExeOrder{
      class ThreadExeOrder;
      typedef std::vector<ThreadExeOrder *> ThreadExeOrders;
      ThreadExeOrders threadExeOrders;
      ThreadExeOrder *lookupThreadExeOrder(ThreadID tid);
    public:
      ~ExeOrder(void);
      bool hasNextEpoch(ThreadID tid, ClockValue parClk);
      void getNextEpoch(ThreadID tid, ClockValue parClk,
			ClockValue &myClk, size_t &iCount, SysCallLog &myLog,
			ThreadContext *myContext);
      void mergeEpochInit(ThreadID tid, ClockValue parClk,
			  ClockValue myClk, const ThreadContext *myContext);
      void mergeEpochDone(ThreadID tid, ClockValue myClk, size_t iCount, SysCallLog &myLog);
      void spliceIntoPrevious(ExeOrder &prev);
      void rewind(void);
      bool empty(void) const;
    };

    ExeOrder exeOrder;
    
    // A block of memory for keeping the checkpoint state
    enum MemoryBlockConstantsEnum{
      logBlockSize=5,
      blockSize=(1<<logBlockSize),
      blockAddrMask=(blockSize-1),
      blockBaseMask=0xffffffff-blockAddrMask
    };
    class BlockData;
    // Blocks of pre-checkpoint data that were
    // modified after this checkpoint but before the next one
    typedef HASH_MAP<Address,BlockData *> BlocksMap;
    BlocksMap myBlocks;
  public:
    static void staticConstructor(void);
    static void staticDestructor(void);

    // Prepares for merging the epoch into the appropriate checkpoint
    // Returns the checkpoint into which the epoch will be merged
    static Checkpoint *mergeInit(Epoch *epoch);

    // Lets the checkpoint know that a merging epoch is done merging
    void mergeDone(Epoch *epoch);

    // Must be called before memory block is modified
    // Logs the original content of the block so it can be overwritten
    void write(Address addr);
    // Find the checkpoint into which to merge an epoch with given clock
    static Checkpoint *getCheckpoint(ClockValue epochClock);
    // The checkpoint is merged with the preceding checkpoint and destroyed
    void merge(void);
    // Restores the state as of the latest checkpoint that is not after targClock
    // At call time currClock is the current clock value.
    // At return time it contains clock value of the restored checkpoint
    static void rollback(ClockValue &currClock, ClockValue targClock);
    // Starting from the state at the end of the checkpoint period,
    // restores the state as it was at the beginning of the period
    void rewind(void);
    // Frees the checkpoint
    ~Checkpoint(void);
  };
}
#endif // !(defined _Checkpoint_hpp_)
