#if !(defined _Epoch_hpp_)
#define _Epoch_hpp_

#include "estl.h"

#include "SysCall.h"
#include "OSSim.h"
#include "nanassert.h"
#include "Snippets.h"
#include "ReportGen.h"
#include <map>
#include <list>
#include <set>

#include <iostream>
#include <algorithm>
#include "mintapi.h"
#include "ThreadContext.h"
#include "MemRequest.h"
#include "callback.h"
#include "GStats.h"

class SysCallLog : public std::list<SysCall *>{
 public:
  // Destructor must call done method of each SysCall in correct order
  ~SysCallLog(void){
    while(!empty()){
      SysCall *sysCall=front();
      pop_front();
      sysCall->done();
      delete sysCall;
    }
  }
};

void callSysCallExit(ThreadContext *context,icode_ptr picode);

namespace tls{

  // Number of dynamic instructions
  typedef long long InstrCount;

  typedef Pid_t ThreadID;
  const ThreadID invalidThreadID=-1;
  
  typedef std::set<class Epoch *> EpochSet;
  typedef std::list<class Epoch *> EpochList;

  // Type for a scalar value of a clock
  typedef long int32_t ClockValue;
  // Too-small value for a clock field (all actual values are larger)
  const ClockValue smallClockValue=0x00000000l;
  // Initial value of the clock field (smallest actual clock value)
  const ClockValue startClockValue=0x00000001l;
  // Too-large value for a clock field (all actual values are smaller)
  const ClockValue largeClockValue=0x7fffffffl;

  typedef std::vector<ClockValue> ClockValueVector;

  // The type for a Lamport clock
  class LClock{
  private:
    ClockValue myValue;
    static ClockValue syncDelta;
  public:
    static void staticConstructor(void);
    LClock(void)
      : myValue(smallClockValue){
    }
    LClock(const LClock &other)
      : myValue(other.myValue){
    }
    void advance(void){
      if(myValue==smallClockValue)
	myValue=startClockValue;
      else
	myValue++;
    }
    void advance(const LClock &other){
      if(myValue<other.myValue)
        myValue=other.myValue;
    }
    void succeed(const LClock &other){
      if(other.myValue+syncDelta+1>myValue)
	myValue=other.myValue+syncDelta+1;
    }
    static bool isOrder(const LClock &first, const LClock &second){
      return first.myValue+syncDelta<second.myValue;
    }
  };
  
  // The type for a vector clock
  class VClock{
  public:
    typedef std::vector<ClockValue> ClockValueVector;
  private:
    // The size of all vector clocks in the system
    static size_t vectorSize;
    typedef std::vector<VClock *> VClockList;
    // Statically allocated VClock instances
    static VClockList statVClocks;
    // In-use dynamically allocated VClock instances
    static VClockList usedVClocks;
    // Free VClock instances ready for dynamic allocation
    static VClockList freeVClocks;
    // A clock with an invalid threadID and with too-small values in all fields
    static VClock smallVClockValue;
    // A clock with an invalid threadID and with too-large values in all fields
    static VClock largeVClockValue;
    // A clock with an invalid threadID. Its fields are valid only for free
    // thread IDs, and indicate the last used own clock value for each thread ID
    static VClock freeStart;

    // The threadID is the index to the thread's component of the vecotor clock
    ThreadID myThreadID;
    // The actual vector timestamp
    ClockValueVector myVector;
    // Position of this clock in allVClocks
    VClockList::size_type myListPos;
    // Constructs a new dynamically allocated VClock
    // whose vector is equal to that of the given clock,
    // but with invalidThreadID
    VClock(const VClock *srcClock);
  public:
    // Constructs a new statically allocated VClock,
    // with an invalid ThreadID
    VClock(void);
    // Destroys a VClock
    ~VClock(void);
    // Cleans up at the end
    static void staticDestructor(void);

    // Creates a full copy of the given clock and advances the copy's own value
    static VClock *newVClock(const VClock *srcClock, bool incOwnValue);
    // Creates a copy of the given clock, but with a given ThreadID
    static VClock *newVClock(const VClock *srcClock, ThreadID tid, bool incOwnValue);
    // Frees a VClock by moving it to the free list (it is not actually deallocated)
    static void freeVClock(VClock *vclock);

    enum CmpResult { Before, Unordered, After };
    static const VClock *getSmallValue(void){
      return &smallVClockValue;
    }
    static const VClock *getLargeValue(void){
      return &largeVClockValue;
    }
    static size_t size(void){
      return vectorSize;
    }
    ThreadID tid(void) const{
      return myThreadID;
    }
    ClockValue ownValue(void) const{
      I(myThreadID!=invalidThreadID);
      I((size_t)myThreadID<vectorSize);
      return myVector[myThreadID];
    }
    // Returns true iff first precedes second in VClock ordering
    // Note: is there is no ordering between the two, isOrder returns false
    static bool isOrder(const VClock *first, const VClock *second){
      if(first->myThreadID!=invalidThreadID){
        return (first->myVector[first->myThreadID]<=second->myVector[first->myThreadID]);
      }
      for(size_t index=0;index<vectorSize;index++)
        if(first->myVector[index]>second->myVector[index])
          return false;
      return true;
    }
    void succeed(const VClock *other){
      I((myThreadID==invalidThreadID)||(!isOrder(this,other)));
      I((myThreadID==invalidThreadID)||(!isOrder(other,this)));
      for(size_t index=0;index<vectorSize;index++)
	if(other->myVector[index]>myVector[index])
	  myVector[index]=other->myVector[index];
    }
    void restore(const VClock *other){
      I(myThreadID==other->myThreadID);
      I(myVector[myThreadID]==other->myVector[myThreadID]);
      for(size_t index=0;index<vectorSize;index++)
	I(myVector[index]>=other->myVector[index]);
      myVector=other->myVector;
    }
    void setComponent(ThreadID index, ClockValue value){
      I(index!=invalidThreadID);
      I((size_t)index<myVector.size());
      I(myVector[index]<=value);
      myVector[index]=value;
    }
    ClockValue getComponent(ThreadID index) const{
      I(index!=invalidThreadID);
      I((size_t)index<myVector.size());
      return myVector[index];
    }
    ClockValue &operator[](ThreadID index){
      I(index!=invalidThreadID);
      I((size_t)index<myVector.size());
      return myVector[index];
    }
    const ClockValue &operator[](ThreadID index) const{
      I(index!=invalidThreadID);
      I((size_t)index<myVector.size());
      return myVector[index];
    }
  };

  class Thread{
  private:
    // Thread ID for this thread
    ThreadID myID;
    // ThreadID for thread that spawned this one, -1 if no such thread
    ThreadID parentID;
    // Clock value in the parent thread when this thread was spawned
    ClockValue spawnClock;
    // Vector of Thread pointers
    typedef std::vector<Thread *> ThreadVector;
    // All active threads, indexed by ID
    static ThreadVector threadVector;
    // Clock delta for cross-thread synchronization
    static ClockValue syncClockDelta;
    // The Thread that currently owns the check clock
    static Thread *checkClockOwner;
  protected:
    // All epochs in this thread that were created and not yet destroyed
    // List is sorted by clock, front has highest clock
    EpochList threadEpochs;
    friend class Epoch;
    static void staticConstructor(void);
    static void report(void);
    Epoch *getSpawnEpoch(void);
  private:
    // Position in threadEpochs of the last epoch to hold the
    // ThreadSafe status in this thread. Note that this position
    // may be the end() iterator of the threadEpochs list.
    EpochList::iterator threadSafe;    
    // The vector clock of the thread-safe epoch in this thread
    VClock *threadSafeClk;
    // Not really a clock. Each entry contains a clock value.
    // The clock value at index "i" indicates a VClock value
    // of the youngest epoch in this thread that can no longer
    // miss any races with thread "i". The race frontier for "this"
    // thread is the minimum of (active) components in noRacesMissed.
    // the number of race frontier holders is the number of (active)
    // components in noRaceMissed that are larger than nextFrontierClk
    VClock *noRacesMissed;
    // Position in threadEpochs of the last epoch to cross the
    // no-races-missed frontier. It is the youngest epoch in this
    // thread that can have no races with uncommitted epochs in
    // other threads. Note that this position may be the end()
    // iterator of the threadEpochs list.
    EpochList::iterator raceFrontier;
    // VClock value of the most recent raceFrontier candidate
    // If there is a candidate, this is the clock value of that
    // candidate. If no current candidate, this is the clock of
    // the current raceFrontier epoch (it was the last candidate)
    ClockValue nextFrontierClk;
    // Number of threads that can still have races with the most
    // recent raceFrontier candidate. If there is a current
    // candidate, this number should be non-zero. If there is no
    // current candidate, the number is equal to zero (the most
    // recent candidate has no more races with any threads and is
    // already the new raceFrontier)
    size_t raceFrontierHolders;
    // Called when the next-after-race-frontier epoch changes
    void newFrontierCandidate(void);
    // Returns true iff the epoch (from this thread) can still miss races at this time
    bool canMissRaces(const Epoch *epoch) const;
#if (defined DEBUG)
    // Remembers the last epoch in commitEpoch for debugging
    Epoch *lastCommitEpoch;
    // Remembers the last epoch to befome a frontier candidate
    Epoch *lastFrontierCandidate;
#endif // (defined DEBUG)

    bool     exitAtEnd;
    int32_t      myExitCode;
    static size_t totalThreads;
    static size_t waitingThreads;
    bool isWaiting;
    typedef std::list<Thread *> ThreadList;
    ThreadList activeChildren;
    ThreadList zombieChildren;

    // Called when a child thread is spawned by this thread
    void newChild(Thread *child);
    // Called when we roll back a spawn of a child thread
    void delChild(Thread *child);
    // Called when a child thread ends
    bool endChild(Thread *child, Epoch *exitEpoch);

    // Called when both of these conditions are satisfied
    // 1) exit() is called in exitEpoch of this thread, and
    // 2) waitEpoch is ThreadSafe
    // Returns true if exitEpoch can be completed, false if it must wait
    bool exitCalled(Epoch *exitEpoch);
    // Called when both of these conditions are satisfied
    // 1) wait() is called in waitEpoch of this thread, and
    // 2) waitEpoch is ThreadSafe
    // Returns true if waitEpoch can proceed, false if it must wait
    bool waitCalled(Epoch *waitEpoch);

    // Internal helper function. Given a tid, returns a zombie child
    // with that tid. If tid is invalidThreadID, returs the child that
    // has been waiting the longest in the zombieChildren list.
    // In any case, the returned child is removed from zombieChildren list
    Thread *harvestZombieThread(ThreadID tid);
    
    void wait(void){
      waitingThreads++;
      I(waitingThreads<totalThreads);
      isWaiting=true;
    }
    void proceed(void){
      waitingThreads--;
      I(waitingThreads<totalThreads);
      isWaiting=false;
    }

  public:
    Thread(ThreadID myId, ThreadID parentID, ClockValue spawnClock);
    ~Thread(void);
    ClockValue getSyncClockDelta(void) const{
      return syncClockDelta;
    }
    bool isCheckClockOwner(void) const{
      return (this==checkClockOwner);
    }

    // Called when we need to undo a wait call that successfully
    // harvested a child thread (which is passed as a parameter)
    void undoWaitCall(Thread *child);
    // Called when we need to undo an exit call
    void undoExitCall(void);

    // When an Epoch is created, we first need its Thread
    // Parameter tid is the ID of the new epoch's thread
    // Parameter parentEpoch is the parent epoch of the new epoch
    // Note: if tid matches the ThreadID of parentEpoch's thread,
    // the new epoch will be in parent's thread, otherwise a new thread is spawned
    static Thread *getThreadForNewEpoch(ThreadID tid, Epoch *parentEpoch);
    // Adds a new epoch to this thread, inserting it into threadEpochs
    // Returns the position of the epoch in threadEpochs
    EpochList::iterator addEpoch(Epoch *epoch);
    // Removes an epoch from this thead
    // Returns true iff this was the last epoch in this thread
    bool removeEpoch(Epoch *epoch);
    // True iff no epoch from this thread is currently in the ThreadSafe state
    bool threadSafeAvailable;
    // Notifies the Thread that another epoch has become ThreadSafe
    void moveThreadSafe(void);
    // The VClock of the ThreadSafe epoch has changed
    // (We need to know it because the race frontier
    // is determined by the ThreadSafe clocks)
    void changeThreadSafeVClock(const VClock *newVClock);
    // Call to notify the Thread that one of its epochs became Committed
    void commitEpoch(class Epoch *epoch);
    // Internal function. Called when the race frontier advances
    void moveRaceFrontier(void);

    ThreadID getID(void) const{
      return myID;
    }
    ThreadID getParentID(void) const{
      return parentID;
    }
    Epoch *getInitialEpoch(void) const{
      return threadEpochs.back();
    }
    static Thread *getByID(ThreadID id){
      I(id!=invalidThreadID);
      I((size_t)id<threadVector.size());
      return threadVector[id];
    }
    static ThreadID getUbID(void){
      return threadVector.size();
    }
  private:
    // Current number of blocks buffered by epochs of this thread
    size_t myAllVersionsCount;
    // Current number of non-most-recent blocks buffered by epochs of this thread
    size_t myOldVersionsCount;
    // Maximum allowed per-thread number of all (not per-block) non-most-recent versions
    static size_t limitThreadOldVersions;
    // The actual maximum per-thread number of all (not per-block) non-most-recent versions
    static size_t maxThreadOldVersions;
    // Maximum allowed per-thread number of all (not per-block) versions
    static size_t limitThreadAllVersions;
    // The actual maximum per-thread number of all (not per-block) versions
    static size_t maxThreadAllVersions;
    // Maximum number of instructions to execute in a thread
    static InstrCount limitThreadInstrCount;
    // Actual maximum number of instructions executed in a thread
    static InstrCount maxThreadInstrCount;
    // Current total number of instructions executed in this thread
    InstrCount currentInstrCount;
  public:
    void incAllVersionsCount(void);
    void decAllVersionsCount(void){
      myAllVersionsCount--;
    }
    size_t getAllVersionsCount(void) const{
      return myAllVersionsCount;
    }
    void incOldVersionsCount(void);
    void decOldVersionsCount(void){
      myOldVersionsCount--;
    }
    size_t getOldVersionsCount(void) const{
      return myAllVersionsCount;
    }
  };
  
  class Epoch{
  public:
    static void staticConstructor(void);
    static void report(void);
    static void staticDestructor(void);
	ClockValue getClock(void) { return myClock; }

  private:
    //BACKEND: to stall memory request going up
    typedef std::multimap<Address,CallbackBase *> PendWb;
    static PendWb pendWb;
	//List of epoch that are waiting to be squashed (AnySync)
	typedef std::map <Epoch *,Time_t> WaitAcqMap;
	static WaitAcqMap  waitAcqMap;
    static GStatsCntr waitAcqCycles;
    

    // True iff epochs from the same thread must actually execute in sequence
    static bool threadsSequential;
    // The actual maximum system-wide number of buffered versions of any given block
    static size_t maxAllBlockVersions;
    // Maximum allowed per-thread number of buffered versions of any given block
    // The value of this variable is read from same-name conf variable in [TLS]
    static size_t limitThreadBlockVersions;
    // The actual maximum per-thread number of buffered versions of any given block
    static size_t maxThreadBlockVersions;

    // True iff all (VClock) races must be found before an epoch can merge
    static bool mustFindAllRaces;

    // Maximum allowed number of instructions per epoch
    static InstrCount limitEpochInstrCount;
    // Actual maximum number of instructions per epoch
    static InstrCount maxEpochInstrCount;

    // True iff data races should not be analyzed
    static bool dataRacesIgnored;
    static size_t numEpochs;
    static size_t numRaceMissEpochs;
    // Maximum number of buffer blocks an epoch is allowed to have
    // A value of 0 means there is no limit
    static size_t maxBlocksPerEpoch;
    // True iff program execution should be rolled back once
    // and replayed to test squash/replay facilities
    static bool       testRollback;
    static ClockValue clockAtLastRollback;
    static Epoch     *squashToTestRollback;

    typedef std::vector<Epoch *> PidToEpoch;
    static PidToEpoch pidToEpoch;

    // All epochs that were created and not yet destroyed
    // List is sorted by clock, front has highest clock
    static EpochList allEpochs;
    static EpochList::iterator findPos(ClockValue sClock, EpochList::iterator hint=allEpochs.begin()){
      if((hint!=allEpochs.end())&&(sClock<=(*hint)->myClock)){
	while((hint!=allEpochs.end())&&(sClock<(*hint)->myClock))
	  hint++;
      }else{
	EpochList::reverse_iterator rhint(hint);
	while((rhint!=allEpochs.rend())&&(sClock>(*rhint)->myClock))
	  rhint++;
	hint=rhint.base();
      }
      return hint;
    }

    // SESC context ID for this epoch
    Pid_t  myPid;
    // SESC context of this epoch at its very beginning
    ThreadContext myContext;
    
    // Create a SESC context for this thread, save its initial state, and create pid-to-epoch mapping
    void createContext(ThreadContext *context){
      I(myPid==-1);
      I(myState==State::Spawning);
      // Create SESC context
      myPid=mint_sesc_create_clone(context);
      ThreadContext *threadContext=osSim->getContext(myPid);
      threadContext->setEpoch(this);
      myContext=*threadContext;
      // Map the context's pid to this epoch
      I(myPid>0);
      PidToEpoch::size_type index=static_cast<PidToEpoch::size_type>(myPid);
      I((pidToEpoch.size()<=index)||pidToEpoch[index]==0);
      if(pidToEpoch.size()<=index)
	pidToEpoch.resize(index+1,0);
      I(pidToEpoch.size()>index);
      I(pidToEpoch[index]==0);
      pidToEpoch[index]=this;      
    }

    // Beginning time of the execution that got Decided
    Time_t beginDecisionExe;
    Time_t endDecisionExe;
    // Beginning time of the current execution
    Time_t beginCurrentExe;
    Time_t endCurrentExe;

    // Number of instructions this epoch is about to execute
    // in the current (or, if Ended, last executed) run
    InstrCount pendInstrCount;
    // Number of instructions this epoch is has begun executing
    // in the current (or, if Ended, last executed) run
    InstrCount execInstrCount; 
    // Number of instructions this epoch has fully executed
    // in the current (or, if Ended, last executed) run
    InstrCount doneInstrCount;
    // Number of instructions this epoch is allowed to execute
    // If the epoch ever was Decided, this is the number of
    // instructions actually executed in the last run before it
    // became decided
    InstrCount maxInstrCount;
    
    friend class Thread;
    friend class Checkpoint;

    // The thread this epoch belongs to
    Thread *myThread;
    // Returns the thread this epoch belongs to
    Thread *getThread(void) const{
      return myThread;
    }

    static VClock memVClock;
    static LClock memLClock;
    VClock *myVClock;
    VClock *originalVClock;

    // Scalar clock value of the epoch that spawned this one
    ClockValue parentClock;
    // Scalar clock state for this epoch
    ClockValue myClock;
    // Number of runtime clock adjustments in this epoch
    size_t runtimeClockAdjustments;
    

    ClockValue getSyncClockDelta(void) const{
      return myThread->getSyncClockDelta();
    }
    
    ClockValue myCheckClock;
    LClock     myLClock;
    ClockValue getCheckClock(void) const{ return myCheckClock; }
    
    class Checkpoint *myCheckpoint;

    // Position of this epoch in the allEpochs list
    EpochList::iterator myAllEpochsPos;
    // Position of this epoch in its Thread's threadEpochs list
    EpochList::iterator myThreadEpochsPos;
    
    class RaceEntry{
      class RaceTypeEntry{
	size_t count;
	size_t minAssoc;
	size_t minBlocks;
      public:
	RaceTypeEntry(void)
	  : count(0){
	}
	bool empty(void) const{
	  return !count;
	}
	void add(const RaceTypeEntry &other){
	  if(other.count){
	    if((!count)||(minAssoc>=other.minAssoc)){
	      if((!count)||(minBlocks>other.minBlocks))
		minBlocks=other.minBlocks;
	      minAssoc=other.minAssoc;
	    }
	    count+=other.count;
	  }
	}
	void add(const std::pair<size_t,size_t> &ageInfo){
	  size_t assoc=ageInfo.first;
	  size_t blocks=ageInfo.second;
	  if((!count)||(minAssoc>=assoc)){
	    if((!count)||(minBlocks>blocks))
	      minBlocks=blocks;
	    minAssoc=assoc;
	  }
	  count++;
	}
        size_t getCount(void) const{
          return count;
        }
      };
      RaceTypeEntry race, chka, lama, runa, anom;
    public:
      bool empty(void) const{
	return race.empty()&&chka.empty()&&lama.empty()&&runa.empty()&&anom.empty();
      }
      bool raceEmpty(void) const{ return race.empty(); }
      bool chkaEmpty(void) const{ return chka.empty(); }
      bool lamaEmpty(void) const{ return lama.empty(); }
      bool runaEmpty(void) const{ return runa.empty(); }
      bool anomEmpty(void) const{ return anom.empty(); }
 
      void add(const RaceEntry &other){
	race.add(other.race);
	chka.add(other.chka);
	lama.add(other.lama);
	runa.add(other.runa);
	anom.add(other.anom);
      }
      void addRace(const std::pair<size_t,size_t> &ageInfo){
	race.add(ageInfo);
      }
      void addChka(const std::pair<size_t,size_t> &ageInfo){
	chka.add(ageInfo);
      }
      void addLama(const std::pair<size_t,size_t> &ageInfo){
        //I(!race.empty());
	lama.add(ageInfo);
      }
      void addRuna(const std::pair<size_t,size_t> &ageInfo){
	runa.add(ageInfo);
      }
      void addAnom(const std::pair<size_t,size_t> &ageInfo){
	anom.add(ageInfo);
      }
      void report(VAddr addr) const{
	I(!empty());
        char allbuf[256];
        char onebuf[32];
        sprintf(allbuf,"TLS:raceInstr(%08x)=(",(unsigned)addr);
	if(race.empty()){
          strcat(allbuf,"norace");
        }else{
          sprintf(onebuf,"race:%d",race.getCount());
          strcat(allbuf,onebuf);
        }
	if(lama.empty()){
          strcat(allbuf,",nolama");
        }else{
          sprintf(onebuf,",lama:%d",lama.getCount());
          strcat(allbuf,onebuf);
	}
	if(runa.empty()){
          strcat(allbuf,",noruna");
        }else{
          sprintf(onebuf,",runa:%d",runa.getCount());
          strcat(allbuf,onebuf);
        }
	if(chka.empty()){
          strcat(allbuf,",nochka");
        }else{
          sprintf(onebuf,",chka:%d",chka.getCount());
          strcat(allbuf,onebuf);
        }
	if(anom.empty()){
          strcat(allbuf,",noanom");
        }else{
          sprintf(onebuf,",anom:%d",anom.getCount());
          strcat(allbuf,onebuf);
        }
        Report::field("%s)",allbuf);
      }
    };
    class InstRaces{
      typedef std::map<VAddr,RaceEntry> InstToRaceEntry;
      InstToRaceEntry instToRaceEntry;
    public:
      bool empty(void) const{
	return instToRaceEntry.empty();
      }
      void clear(void){
	instToRaceEntry.clear();
      }
      void add(const InstRaces &other){
	for(InstToRaceEntry::const_iterator it=other.instToRaceEntry.begin();
	    it!=other.instToRaceEntry.end();it++)
	  instToRaceEntry[it->first].add(it->second);
      }
      void addRace(VAddr iAddr, const std::pair<size_t,size_t> &ageInfo){
	instToRaceEntry[iAddr].addRace(ageInfo);
      }
      void addChka(VAddr iAddr, const std::pair<size_t,size_t> &ageInfo){
	instToRaceEntry[iAddr].addChka(ageInfo);
      }
      void addLama(VAddr iAddr, const std::pair<size_t,size_t> &ageInfo){
	instToRaceEntry[iAddr].addLama(ageInfo);
      }
      void addRuna(VAddr iAddr, const std::pair<size_t,size_t> &ageInfo){
	instToRaceEntry[iAddr].addRuna(ageInfo);
      }
      void addAnom(VAddr iAddr, const std::pair<size_t,size_t> &ageInfo){
	instToRaceEntry[iAddr].addAnom(ageInfo);
      }
      void report(void) const{
	size_t numRace=0;
	size_t numLama=0;
	size_t numRuna=0;
	size_t numLaRu=0;
	size_t numChka=0;
	size_t numAnom=0;
	for(InstToRaceEntry::const_iterator it=instToRaceEntry.begin();
	    it!=instToRaceEntry.end();it++){
	  it->second.report(it->first);
	  if(!it->second.raceEmpty())
	    numRace++;
	  if(!it->second.lamaEmpty())
	    numLama++;
	  if(!it->second.runaEmpty())
	    numRuna++;
	  if((!it->second.runaEmpty())||(!it->second.lamaEmpty()))
	    numLaRu++;
	  if(!it->second.chkaEmpty())
	    numChka++;
	  if(!it->second.anomEmpty())
	    numAnom++;
	}
        Report::field("TLS:raceInstrs=%d",numRace);
        Report::field("TLS:lamaInstrs=%d",numLama);
        Report::field("TLS:runaInstrs=%d",numRuna);
        Report::field("TLS:laruInstrs=%d",numLaRu);
        Report::field("TLS:chkaInstrs=%d",numChka);
        Report::field("TLS:anomInstrs=%d",numAnom);
      }
    };
    // Per-instruction race info for the entire program execution
    static InstRaces allInstRaces;
    // Per-instruction race info for this epoch. It is merged into
    // allInstRaces when the epoch becomes Committed
    InstRaces myInstRaces;
    
    struct State{
      enum AtmEnum  {NoAtm, Atm} atm;
      enum AcqEnum  {NoAcq, Acq} acq;
      enum RelEnum  {NoRel, Rel} rel;
      enum SuccEnum {
                      NoSuccessors=0,
		      NameSuccessors=1,
		      FlowSuccessors=2,
		      DataSuccessors=3, // Name or Flow
		      SpawnSuccessors=4
                     } succ;
      enum SpecEnum {Spec, ThreadSafe, GlobalSafe, Committed}  spec;
      enum ExecEnum {Spawning, Running, Waiting, Completed}    exec;
      enum IterEnum {
	Initial, PartReplay, FullReplay
      } iter;
      enum MergEnum {
	NoMerge,
	ReqLazyMerge, ReqEagerMerge,
	LazyMerge, EagerMerge
      } merg;
      enum HoldEnum {
	NoHold,
	PredHold=1,
	RaceHold=2
      } hold;
      enum WaitEnum {NoWait,
		     WaitThreadSafe, 
		     WaitGlobalSafe, 
		     WaitFullyMerged,
		     WaitUnspawn,
		     WaitAcqRetry,
		     WaitZombie,
                     WaitChild
      } wait;
      State(void)
	: atm(NoAtm), acq(NoAcq), rel(NoRel), succ(NoSuccessors),
	   spec(Spec), exec(Spawning), iter(Initial),
           merg(NoMerge),
           hold(NoHold),
           wait(NoWait){
      }
      State &operator=(AtmEnum newAtm){ atm=newAtm; return *this; }
      bool operator==(AtmEnum cmpAtm) const { return atm==cmpAtm; }
      bool operator!=(AtmEnum cmpAtm) const { return atm!=cmpAtm; }
      State &operator=(AcqEnum newAcq){ acq=newAcq; return *this; }
      bool operator==(AcqEnum cmpAcq) const { return acq==cmpAcq; }
      bool operator!=(AcqEnum cmpAcq) const { return acq!=cmpAcq; }
      State &operator=(RelEnum newRel){ rel=newRel; return *this; }
      bool operator==(RelEnum cmpRel) const { return rel==cmpRel; }
      bool operator!=(RelEnum cmpRel) const { return rel!=cmpRel; }
      State &operator=(SuccEnum newSucc){
	I(newSucc==NoSuccessors);
	succ=newSucc;
	return *this;
      }
      State &operator+=(SuccEnum addSucc){
	I(addSucc!=NoSuccessors);
	succ=static_cast<SuccEnum>(succ|addSucc);
	return *this;
      }
      bool operator==(SuccEnum cmpSucc) const{
	if(cmpSucc==NoSuccessors)
	  return (succ==cmpSucc);
	return (succ&cmpSucc);
      }
      State &operator-=(SuccEnum remSucc){
	I(remSucc!=NoSuccessors);
	succ=static_cast<SuccEnum>(succ^(succ&remSucc));
	return *this;
      }
      bool operator!=(SuccEnum cmpSucc) const {return !operator==(cmpSucc);}
      State &operator=(SpecEnum newSpec){ spec=newSpec; return *this; }
      State &operator=(ExecEnum newExec){ exec=newExec; return *this; }
      State &operator=(IterEnum newIter){ iter=newIter; return *this; }
      State &operator=(MergEnum newMerg){ merg=newMerg; return *this; }
      State &operator=(WaitEnum newWait){ wait=newWait; return *this; }
      bool operator==(SpecEnum cmpSpec) const{ return spec==cmpSpec; }
      bool operator!=(SpecEnum cmpSpec) const{ return spec!=cmpSpec; }
      bool operator>=(SpecEnum cmpSpec) const{ return spec>=cmpSpec; }
      bool operator<(SpecEnum cmpSpec) const{ return spec<cmpSpec; }
      bool operator==(ExecEnum cmpExec) const{ return exec==cmpExec; }
      bool operator!=(ExecEnum cmpExec) const{ return exec!=cmpExec; }
      bool operator==(IterEnum cmpIter) const{ return iter==cmpIter; }
      bool operator!=(IterEnum cmpIter) const{ return iter!=cmpIter; }
      bool operator>=(IterEnum cmpIter) const{ return iter>=cmpIter; }
      bool operator<(IterEnum cmpIter) const{ return iter<cmpIter; }
      bool operator==(MergEnum cmpMerg) const{ return merg==cmpMerg; }
      bool operator!=(MergEnum cmpMerg) const{ return merg!=cmpMerg; }
      bool operator>=(MergEnum cmpMerg) const{ return merg>=cmpMerg; }
      bool operator<(MergEnum cmpMerg) const{ return merg<cmpMerg; }
      bool operator==(WaitEnum cmpWait) const{ return wait==cmpWait; }
      bool operator!=(WaitEnum cmpWait) const{ return wait!=cmpWait; }
      bool operator>=(WaitEnum cmpWait) const{ return wait>=cmpWait; }
      bool operator<(WaitEnum cmpWait) const{ return wait<cmpWait; }
      State &operator+=(HoldEnum addHold){
	I(addHold!=NoHold);
	hold=static_cast<HoldEnum>(hold|addHold);
	return *this;
      }
      bool operator==(HoldEnum cmpHold) const{
	if(cmpHold==NoHold)
	  return (hold==cmpHold);
	return (hold&cmpHold);
      }
      State &operator-=(HoldEnum remHold){
	I(remHold!=NoHold);
	hold=static_cast<HoldEnum>(hold^(hold&remHold));
	return *this;
      }
      bool operator!=(HoldEnum cmpHold){return !operator==(cmpHold);}
    };
    
    // State of this epoch
    State myState;
    // Number of active epochs that were directly spawned by this one
    size_t spawnSuccessorsCnt;
    // The current level of nesting in an atomic section
    size_t atmNestLevel;
    // Which atomic section (if any) did this epoch enter
    VAddr myAtomicSection;
    // Counts the number of times each atomic section has been entered
    typedef std::map<VAddr,size_t> AddrToCount;
    static AddrToCount atomicEntryCount;

    static size_t staticAtmOmmitCount;
    static size_t dynamicAtmOmmitCount;
    static VAddr  atmOmmitInstr;
    static size_t atmOmmitCount;
    bool skipAtm;

    void initMergeHold(void);
    void removeMergeHold(State::HoldEnum hold);
    bool canEasilyAdvance(void) const{
      return                            // Can not easily advance if
	(myState<State::GlobalSafe)&&   // 1) globally safe (advancing can make us unsafe),
	(myState<State::PartReplay)&&   // 2) if this is a replay (was GlobalSafe before), or
	(myState==State::NoSuccessors); // 3) has successors (which must be squashed to advance)
    }
    bool canBeTruncated(void) const{
      return                                  // Can not truncate if
	(myState==State::Running)&&           // 1) it is not running, or
	(myState!=State::Atm)&&               // 2) atomic (breaks atomicity), or
	(!atmNestLevel)&&                     // 3) should be atomic (but we injected an error), or
	(myState!=State::SpawnSuccessors)&&   // 4) already has spawn-successors, or
        (pendInstrCount);                     // 5) has not executed any instructions
    }

    static void printEpochs(void){
      std::cout << "Epochs: ";
      for(EpochList::iterator it=allEpochs.begin();it!=allEpochs.end();it++){
	Epoch *ep=*it;
	std::cout << " " << ep->myClock << ":" << ep->getTid() << ":";
	if(ep->myState==State::Spec){
	  std::cout << "S";
	}else if(ep->myState==State::ThreadSafe){
	  std::cout << "T";
	}else if(ep->myState==State::GlobalSafe){
	  std::cout << "G";
	}else if(ep->myState==State::Committed){
	  std::cout << "C";
	}
      }
      std::cout << std::endl;
    }
    
    // Creates the initial epoch in a thread
    Epoch(ThreadID tid, Epoch *parentEpoch);
    // Creates a same-thread successor of an epoch
    Epoch(Epoch *parentEpoch);
    // Destroys this epoch
    ~Epoch(void);
   
    // True iff no epoch is currently in the GlobalSafe state
    static bool globalSafeAvailable;
    // Position in allEpochs of the most recent GlobalSafe epoch
    static EpochList::iterator globalSafe;
    // Returns the clock value of the GlobalSafe epoch
    static ClockValue getGlobalSafeClock(void){
      if(globalSafe==allEpochs.end())
        return startClockValue;
      return (*globalSafe)->getClock();
    }

    enum CmpResult{
      StrongBefore,
      WeakBefore,
      Unordered,
      WeakAfter,
      StrongAfter
    };

    CmpResult compareClock(const Epoch *otherEpoch) const;
    CmpResult compareCheckClock(const Epoch *otherEpoch) const;
    void advanceClock(const Epoch *predEpoch,bool sync);
    
  public:
	//BACKEND: getter/setter for callback
	//void setCB(CallbackBase *wcb) {wbCb=wcb;}
	//CallbackBase * getCB(){return wbCb;}
	
    const VClock *getVClock(void) const{
      return myVClock;
    }
    
    static Epoch *initialEpoch(ThreadID tid, Epoch *parentEpoch);
    Epoch *spawnEpoch(void);
    Epoch *changeEpoch(){
      I(canBeTruncated());
      Epoch *newEpoch=spawnEpoch();
      if(!newEpoch){
	if(waitGlobalSafe())
	  I(0);
	return 0;
      }
      complete();
      if(!threadsSequential)
	newEpoch->run();
      // printEpochs();
      return newEpoch;
    }
    // Called when thread-safety rules allow an epoch
    // to begin executing after a spawn or a squash
    void run(void);

    void beginReduction(VAddr iVAddr);
    void endReduction(void);
    void beginAtomic(VAddr iVAddr, bool isAcq, bool isRel);
    void retryAtomic(void);
    void changeAtomic(bool endAcq, bool begRel);
    void endAtomic(void);

    void succeeded(Epoch *succEpoch);
    void unSucceed(void);
    void threadSave(void);
    static void advanceGlobalSafe(void);
    void tryGlobalSave(void);
    void globalSave(void);
    void complete(void);
    void tryCommit(void);
    void commit(void);

    bool waitThreadSafe(void){
      if(myState>=State::ThreadSafe)
	return true;
      if(myState>=State::WaitThreadSafe)
	return false;
      I(myState==State::Running);
      I(myState!=State::Acq);
      // Make the epoch wait until it becomes ThreadSafe
      myState=State::Waiting;
      myState=State::WaitThreadSafe;
      osSim->stop(myPid);
      return false;
    }
    bool waitGlobalSafe(void){
      if(myState>=State::GlobalSafe)
	return true;
      if(myState>=State::WaitGlobalSafe)
	return false;
      I(myState==State::Running);
      I(myState!=State::Acq);
      // Make the epoch wait until it becomes GlobalSafe
      myState=State::Waiting;
      myState=State::WaitGlobalSafe;
      osSim->stop(myPid);
      return false;
    }
    bool waitFullyMerged(void){
      I(myState==State::Running);
      requestEagerMerge();
      if((myState>=State::LazyMerge)&&bufferBlockList.empty())
	return true;
      if(myState>=State::WaitFullyMerged)
	return false;
      I(myState!=State::Acq);
      // Make the epoch wait until it becomes GlobalSafe
      myState=State::Waiting;
      myState=State::WaitFullyMerged;
      osSim->stop(myPid);
      return false;
    }
    // Called to force an event to be at the beginning of an epoch
    // If "this" epoch is not beginning now, this method will change
    // to a new epoch and move the currently executing instruction there.
    // Returns true iff the epoch was changed
    bool forceEpochBeginning(void);
    void waitAcqRetry(void);
    void waitChild(void);
    void endWaitChild(void);
    void waitZombie(void);
    void endWaitZombie(void);
    void waitCalled(void);
    void exitCalled(void);

    void becomeFullyMerged(void){
      I(myState==State::WaitFullyMerged);
      I((myState>=State::LazyMerge)&&bufferBlockList.empty());
      myState=State::Running;
      myState=State::NoWait;
      osSim->unstop(myPid);
    }
    void squash(bool skipSelf);
    void undoSysCalls(void);
    void squashLocal(void);
    void unspawn(void);
    
    void requestLazyMerge(void);
    void requestEagerMerge(void);
    void beginLazyMerge(void);
    void beginEagerMerge(void);

    static Epoch *getEpoch(Pid_t pid){
      I(pid>=0);
      ID(ThreadContext *context=osSim->getContext(pid));
      PidToEpoch::size_type index=static_cast<PidToEpoch::size_type>(pid);
      if(pidToEpoch.size()<=index){
        I(context->getEpoch()==0);
	return 0;
      }
      I(context->getEpoch()==pidToEpoch[pid]);
      return pidToEpoch[pid];
    }

    SysCallLog sysCallLog;
    SysCallLog::iterator sysCallLogPos;
    
    template<class SysCallType>
    SysCallType *Epoch::newSysCall(void){
      SysCallType *sysCall;
      if(sysCallLogPos==sysCallLog.end()){
        sysCall=new SysCallType();
        sysCallLog.push_back(sysCall);
        I(sysCallLogPos==sysCallLog.end());
      }else{
        I(dynamic_cast<SysCallType *>(*sysCallLogPos)!=0);
        sysCall=static_cast<SysCallType *>(*sysCallLogPos);
        sysCallLogPos++;
      }
      return sysCall;
    }

    Pid_t getPid(void) const{
      return myPid;
    }
    ThreadID getTid(void) const{
      return myThread->getID();
    }
  private:
    // Code related to buffering memory state of an epoch begins here
    
    // Memory state buffer of an epoch consists of blocks
    // Each block buffers blockSize consecutive bytes of data and
    // their access bits. Each kind of access bits for a block is
    // stored in a sequence of entries whose type is ChunkBitMask,
    // each containing chunkSize access bits.
    typedef uint8_t ChunkBitMask;
    enum BufferBlockConstantsEnum{
      logChunkSize=3,
      logBlockSize=5,
      chunkSize=(1<<logChunkSize),
      blockSize=(1<<logBlockSize),
      chunkAddrMask=(chunkSize-1),
      blockAddrMask=(blockSize-1)
    };
    class BufferBlock;
    // List of buffer blocks
    typedef std::list<BufferBlock *> BufferBlockList;
    class BlockVersions{
      // Base address for all versions of this block
      Address baseAddr;
      // Maps a block base address to its BlockVersions entry
      typedef HASH_MAP<Address,BlockVersions *>  BlockAddrToVersions;
      static BlockAddrToVersions blockAddrToVersions;
      friend class BufferBlock;
      friend class Epoch;
      // Lists of all accessors and of writers of this block
      // Both lists are sorted in order of recency (most recent first)
      ID(public:)
      // List of all accessors of the block
      BufferBlockList accessors;
      // List of writers of the block
      BufferBlockList writers;
      ID(private:)
      // True iff there was never any writer
      bool rdOnly;
      // Current total number of accessors
      size_t accessorsCount;
      typedef std::vector<size_t> ThreadAccessorsCount;
      ThreadAccessorsCount threadAccessorsCount;
      BlockVersions(Address baseAddr)
	: baseAddr(baseAddr), rdOnly(true), accessorsCount(0){
	I(blockAddrToVersions.find(baseAddr)==blockAddrToVersions.end());
	blockAddrToVersions.insert(BlockAddrToVersions::value_type(baseAddr,this));
      }
      ~BlockVersions(void){
	I(accessorsCount==0);
        for(ThreadAccessorsCount::iterator tacIt=threadAccessorsCount.begin();
            tacIt!=threadAccessorsCount.end();tacIt++)
          I(!(*tacIt));
	I(accessors.empty());
	I(writers.empty());
      }
    public:
      static void staticDestructor(void){
	// Destroy all BlockVersions
	for(BlockAddrToVersions::iterator versIt=blockAddrToVersions.begin();
	    versIt!=blockAddrToVersions.end();versIt++){
	  BlockVersions *blockVersions=versIt->second;
	  delete blockVersions;
	}
	blockAddrToVersions.clear();
      }
      static BlockVersions *getVersions(Address blockBase){
	BlockAddrToVersions::iterator versionsIt=
	  blockAddrToVersions.find(blockBase);
	if(versionsIt!=blockAddrToVersions.end())
	  return versionsIt->second;
	return new BlockVersions(blockBase);
      }
      Address getBaseAddr(void) const{
	return baseAddr;
      }
      std::pair<size_t,size_t> getAgeInThread(BufferBlock *block);
      bool isRdOnly(void) const{
	return rdOnly;
      }
      struct ConflictInfo{
	BufferBlock *block;
	ChunkBitMask mask;
	ConflictInfo(BufferBlock *block, ChunkBitMask mask)
	  : block(block), mask(mask){
	}
	ConflictInfo(const ConflictInfo &other)
	  : block(other.block), mask(other.mask){
	}
      };
      typedef BufferBlockList::iterator BlockPosition;
      BlockPosition findBlockPosition(const BufferBlock *block);
      void advance(BufferBlock *block);
      void access(bool isWrite, BufferBlock *currBlock, BlockPosition &blockPos);

      typedef std::list<ConflictInfo> ConflictList;

      ChunkBitMask findReadConflicts(const BufferBlock *currBlock, size_t chunkIndx,
				     BlockPosition blockPos,
				     ChunkBitMask beforeMask, ChunkBitMask afterMask,
				     ConflictList &writesBefore, ConflictList &writesAfter);

      void findWriteConflicts(const BufferBlock *currBlock, size_t chunkIndx,
			      BlockPosition blockPos,
			      ChunkBitMask beforeMask, ChunkBitMask afterMask,
			      ConflictList &readsBefore, ConflictList &writesBefore,
			      ConflictList &writesAfter, ConflictList &readsAfter);

      void remove(BufferBlock *block);
      void check(Address baseAddr);
      static void checkAll(void);
    };
    // One block of buffer data and its access bits
    class BufferBlock{
      // Total number of buffered blocks in the system
      static size_t blockCount;
      // True iff the block has any exposed reads
      bool isConsumer;
      // True iff the block has any writes
      bool isProducer;
      // True iff the block has been a source for an exposed read
      bool myIsFlowSource;
      // True iff any part of the block has been overwriten by a successor
      bool myIsStale;
      // True iff this block has already been merged to memory
      bool myIsMerged;
      // Info on other versions of this block
      friend class BlockVersions;
      friend class Epoch;
      ID(public:)
      // All versions of this block and my position in the accessors and writers lists
      BlockVersions *myVersions;
      BufferBlockList::iterator accessorsPos;
      BufferBlockList::iterator writersPos;
    public:
      BlockVersions *getVersions(void) const{
	return myVersions;
      }
      Address getBaseAddr(void) const{
	return myVersions->getBaseAddr();
      }
      static size_t getBlockCount(void){
	return blockCount;
      }
      bool isAccessed(void){
	return isProducer||isConsumer;
      }
      bool isWritten(void){
	return isProducer;
      }
      bool becomeConsumer(void){
	if(isConsumer)
	  return false;
	epoch->consumedBufferBlocks++;
	isConsumer=true;
	return true;
      }
      bool becomeProducer(void){
	if(isProducer)
	  return false;
	epoch->producedBufferBlocks++;
	isProducer=true;
	return true;
      }
      bool isFlowSource(void) const{
	return myIsFlowSource;
      }
      void becomeFlowSource(void){
	if(!myIsFlowSource){
	  myIsFlowSource=true;
	}
      }
      bool isStale(void) const{
	return myIsStale;
      }
      void becomeStale(void){
	if(!myIsStale){
	  myIsStale=true;
	}
      }
      bool isMerged(void) const{
	return myIsMerged;
      }
      // The epoch the block belongs to and my position in the bufferBlockList
      Epoch *epoch;
      BufferBlockList::iterator bufferPos;
      // The base address of the block
      Address baseAddr;
      // A byte in wkData is valid if either its xpMask or
      // its wrMask bit is set.
      // Exposed mask. A set bit indicates that this version
      // did a copy-in of that byte from a previous version
      ChunkBitMask xpMask[blockSize/chunkSize];
      // Write mask. A set bit indicates that the version wrote
      // the corresponding byte
      ChunkBitMask wrMask[blockSize/chunkSize];
      // Work data. Current values of this version's data
      unsigned long long wkData[blockSize/sizeof(unsigned long long)];
      // Default constructor. Creates a new BufferBlock with all masks empty 
      BufferBlock(Epoch *epoch, Address baseAddr)
	: isConsumer(false), isProducer(false),
	  myIsFlowSource(false), myIsStale(false),
	  myIsMerged(false),
	  myVersions(BlockVersions::getVersions(baseAddr)),
	  accessorsPos(myVersions->accessors.end()),
	  writersPos(myVersions->writers.end()),
	  epoch(epoch), baseAddr(baseAddr){
	blockCount++;
	I(chunkSize==sizeof(ChunkBitMask)*8);
	for(int32_t i=0;i<blockSize/chunkSize;i++){
	  xpMask[i]=(ChunkBitMask)0;
	  wrMask[i]=(ChunkBitMask)0;
	}
      }
      ~BufferBlock(void){
	I(blockCount>0);
	blockCount--;
	if(isConsumer)
	  epoch->consumedBufferBlocks--;
	if(isProducer)
	  epoch->producedBufferBlocks--;
	myVersions->remove(this);
	ID(epoch=0);
	ID(myVersions=0);
      }
      static void maskedChunkCopy(uint8_t *srcChunk,
				  uint8_t *dstChunk,
				  ChunkBitMask cpMask){
	I(cpMask);
	uint8_t *srcPtr=srcChunk;
	uint8_t *dstPtr=dstChunk;
	uint8_t *srcChunkEnd=srcChunk+chunkSize;
	while(srcPtr!=srcChunkEnd){
	  if(cpMask&(1<<(chunkSize-1)))
	    (*dstPtr)=(*srcPtr);
	  cpMask<<=1;
	  srcPtr++;
	  dstPtr++;
	}
      }
      // Combines the sucessor block into this block block
      void append(const BufferBlock *succBlock){
	I(baseAddr==succBlock->baseAddr);
	ID(EpochList::reverse_iterator succEpochPos(epoch->myAllEpochsPos));
	I(succBlock->epoch==*succEpochPos);
	uint8_t *srcDataPtr=(uint8_t *)succBlock->wkData;
	uint8_t *dstDataPtr=(uint8_t *)wkData;
	for(int32_t i=0;i<blockSize/chunkSize;i++){
	  ChunkBitMask addXpMask=succBlock->xpMask[i];
	  addXpMask&=(~wrMask[i]);
	  addXpMask&=(~xpMask[i]);
	  ChunkBitMask copyMask=addXpMask;
	  if(addXpMask){
	    xpMask[i]|=addXpMask;
	    if(!isConsumer)
	      becomeConsumer();
	  }
	  ChunkBitMask succWrMask=succBlock->wrMask[i];
	  if(succWrMask){
	    copyMask|=succWrMask;
	    wrMask[i]|=succWrMask;
	    if(!isProducer)
	      becomeProducer();
	  }
	  if(copyMask){
	    maskedChunkCopy(srcDataPtr,dstDataPtr,copyMask);
	  }
	  srcDataPtr+=chunkSize;
	  dstDataPtr+=chunkSize;
	}
      }
      // Cleans the state of the block (invalidate dirty bytes)
      void clean(void){
	for(int32_t i=0;i<blockSize/chunkSize;i++){
	  xpMask[i]&=~wrMask[i];
	  wrMask[i]=(ChunkBitMask)0;
	}
      }
      // Merges the written data of this block into main memory
      void merge(void){
	uint8_t *memDataPtr=(uint8_t *)baseAddr;
	uint8_t *bufDataPtr=(uint8_t *)wkData;
	const uint8_t *memBlockEnd=memDataPtr+blockSize;
	const ChunkBitMask  *wrMaskPtr=wrMask;
	while(memDataPtr<memBlockEnd){
	  ChunkBitMask wrMask=*wrMaskPtr;
	  if(wrMask)
	    maskedChunkCopy(bufDataPtr,memDataPtr,wrMask);
	  memDataPtr+=chunkSize;
	  bufDataPtr+=chunkSize;
	  wrMaskPtr++;
	}
        I(!myIsMerged);
        myIsMerged=true;
      }
    };
    // All buffered blocks of this epoch (a map and a list)
    typedef HASH_MAP<Address,BufferBlock *> BufferBlockMap;
    BufferBlockMap  bufferBlockMap;
    BufferBlockList bufferBlockList;
    size_t consumedBufferBlocks;
    size_t producedBufferBlocks;

    BufferBlock *getBufferBlock(Address blockBase){
      BufferBlockMap::iterator blockIt=bufferBlockMap.find(blockBase);
      // If block exists, return it
      if(blockIt!=bufferBlockMap.end())
	return blockIt->second;
      // No block exists, can one be created?
      if((myState!=State::Atm)&&maxBlocksPerEpoch&&(bufferBlockMap.size()==maxBlocksPerEpoch)){
	changeEpoch();
	return 0;
      }
      // Create new block
      BufferBlock *block=new BufferBlock(this,blockBase);
      bufferBlockMap.insert(BufferBlockMap::value_type(blockBase,block));
      block->bufferPos=bufferBlockList.insert(bufferBlockList.end(),block);
      return block;
    }

    static const ChunkBitMask accessBitMask[16][chunkSize];


    // Internal function.
    // Combines the buffer of a successor epoch into the buffer of this epoch
    void appendBuffer(const Epoch *succEpoch);
    // Internal function.
    // Cleans all buffered blocks by invalidating their dirty data.
    void cleanBuffer(void);
    // Internal funstion.
    // Input:  The base address of the block in blockAddr
    // Effect: Block is erased. If it was the last block
    //         of a committed epoch, the epoch is destroyed
    void eraseBlock(Address baseAddr);
    // Internal function.
    // Input:  The address of the block in baseAddr
    // Effect: Block and all its predecessors are merged into memory
    void mergeBlock(Address baseAddr);
    // Internal function.
    // Input:  The address of the block in baseAddr
    // Effect: If possible, merge the block and erase it
    public:
    bool requestBlockRemoval(Address baseAddr);
    // Requests removal of a given number of blocks
    // Returns the number of blocks that could not be removed
    //class std::MemRequest;
    bool requestBlockRemovalWB(Address baseAddr,  CallbackBase *wcb);
    // Called by backend
    private:
    size_t requestAnyBlockRemoval(size_t count);
    size_t requestOldBlockRemoval(size_t count);
    typedef std::set<Address> AddressSet;
    AddressSet myBlockRemovalRequests;
    typedef std::set<Epoch *> EpochSet;
    static EpochSet pendingBlockRemovals;
    static bool blockRemovalEnabled;
    void doBlockRemovals(void);
    // Internal function.
    // Does eraseBlock on all blocks of this epoch.
    // Note: if epoch is committed, in effect it will be estroyed
    void eraseBuffer(void);
    // Internal function.
    // Does mergeBlock on all blocks of the epoch.
    void mergeBuffer(void);

    // Data addresses that this epoch has races on
    AddressSet traceDataAddresses;
    // Code addresses that cause data races in this epoch
    AddressSet traceCodeAddresses;

  public:
    static bool isBlockRemovalEnabled(void){
      return blockRemovalEnabled;
    }
    static void disableBlockRemoval(void){
      I(blockRemovalEnabled);
      blockRemovalEnabled=false;
    }
    static void enableBlockRemoval(void);

    class TraceEvent{
    protected:
      // ID of the thread performing the event
      ThreadID tid;
      // Address of the instruction performing the event
      VAddr  iAddrV;
      // Number of instructions since the beginning of this epoch
      size_t   instrOffset;
      // Time of the occurence of the event
      Time_t myTime;
    public:
      TraceEvent(Epoch *epoch){
	tid=epoch->getTid();
	iAddrV=osSim->eventGetInstructionPointer(epoch->myPid)->addr;
	instrOffset=epoch->pendInstrCount;
	// Initially store the time of the event in the reenactment
	myTime=globalClock;
      }
      void adjustTime(const Epoch *epoch){
	// Event time from the beginning of the reenactment execution
	myTime-=epoch->beginCurrentExe;
	// Duration of the reeanctment excution
	double reenactTime=epoch->endCurrentExe-epoch->beginCurrentExe;
	// Duration of the original execution
	double originalTime=epoch->endDecisionExe-epoch->beginDecisionExe;
	// Ratio of execution speed in reenactment and original execution
	double speedRatio=originalTime/reenactTime;
	// Event time from the beginning of the original execution
	myTime=(Time_t)(myTime*speedRatio);
	// Event time in the original execution
	myTime+=epoch->beginDecisionExe;
      }
      virtual ~TraceEvent(void){}
      virtual void print(void){
	printf("%12lld %3d %8x",myTime,tid,iAddrV);
      }
      Time_t getTime(void) const{
	return myTime;
      }
      size_t getInstrOffset(void) const{
	return instrOffset;
      }
    };
  private:
    class TraceAccessEvent : public TraceEvent{
    public:
      typedef enum AccessTypeEnum {Read, Write} AccessType;
    protected:
      VAddr dVAddr;
      AccessType accessType;
      size_t accessCount;
      size_t lastInstrOffset;
    public:
      TraceAccessEvent(Epoch *epoch, VAddr dVAddr, AccessType accessType)
	: TraceEvent(epoch), dVAddr(dVAddr), accessType(accessType){
	accessCount=1;
	lastInstrOffset=instrOffset;
      }
      virtual void print(void){
	TraceEvent::print();
	printf(": %8x ",dVAddr);
	switch(accessType){
	case Read: printf("Rd"); break;
	case Write: printf("Wr"); break;
	}
	if(accessCount>1){
	  printf(" %d times in %d instructions",
		 accessCount,lastInstrOffset-instrOffset);
	}
      }
      TraceAccessEvent *newAccess(Epoch *epoch, VAddr dVAddr,
				  AccessType accessType){
// 	if((iAddrV==(Address)(osSim->eventGetInstructionPointer(epoch->myEid)->addr))&&
// 	   (TraceAccessEvent::dAddrV==dAddrV)&&
// 	   (TraceAccessEvent::accessType==accessType)){
// 	  accessCount++;
// 	  lastInstrOffset=epoch->pendInstrCount;
// 	  return 0;
// 	}else{
	  return new TraceAccessEvent(epoch,dVAddr,accessType);
// 	}
      }
      VAddr getDataAddr(void) const{
	return dVAddr;
      }
      AccessType getAccessType(void) const{
	return accessType;
      }
    };
    typedef std::list<TraceEvent *> TraceList;
    TraceList myTrace;
    
    struct RaceInfo{
      enum RaceType {
	None=0,
	Anti=1,
	Output=2,
	Flow=4,
	All=Anti|Output|Flow
      } raceType;
      VAddr dVAddr;
      Epoch *epoch1;      
      Epoch *epoch2;
      RaceInfo(VAddr dVAddr, Epoch *ep1, Epoch *ep2, RaceType raceType)
	: raceType(raceType), dVAddr(dVAddr), epoch1(ep1), epoch2(ep2){
      }
      void addType(RaceType newType){
	raceType=static_cast<RaceType>(raceType|newType);
      }
    };
    
    typedef std::map<VAddr,RaceInfo *> RaceByAddr;
    typedef std::map<Epoch *,RaceByAddr> RaceByEpAddr;
    typedef std::map<Epoch *,RaceInfo *> RaceByEp;
    struct RaceAddrInfo{
      RaceByEp raceByEp;
      VAddr dVAddr;
      enum Position { First, Second} myPosition;
      RaceInfo::RaceType raceTypes;
      TraceAccessEvent *readAccess;
      TraceAccessEvent *writeAccess;
      RaceAddrInfo(VAddr dVAddr, Position myPosition)
	: dVAddr(dVAddr), myPosition(myPosition), raceTypes(RaceInfo::None),
	  readAccess(0), writeAccess(0){
      }
      ~RaceAddrInfo(void){
	if(readAccess)
	  delete readAccess;
	if(writeAccess)
	  delete writeAccess;
      }
      void addType(RaceInfo::RaceType newType){
	raceTypes=static_cast<RaceInfo::RaceType>(raceTypes|newType);
      }
      void addReadAccess(Epoch *epoch, VAddr dVAddr){
	if(myPosition==Second){
	  // A read can be the second access only in a flow-type race
	  if(raceTypes&RaceInfo::Flow){
	    // This is the second access of the race,
	    // so we want it as early as possible 
	    // Thus, take the first eligible access in this epoch 
	    if(!readAccess){
	      readAccess=
		new TraceAccessEvent(epoch,dVAddr,TraceAccessEvent::Read);
	      epoch->addAccessEvent(readAccess);
	    }
	  }
	}else{
	  I(myPosition==First);
	  // If no anti-type race, 
	  // A read can be the first access only in a anti-type race
	  if(raceTypes&RaceInfo::Anti){
	    // This is the first access of the race,
	    // so we want it as late as possible
	    // Thus, take the last eligible access in this epoch 
	    epoch->removeAccessEvent(readAccess);
	    delete readAccess;
	    readAccess=
	      new TraceAccessEvent(epoch,dVAddr,TraceAccessEvent::Read);
	    epoch->addAccessEvent(readAccess);
	  }
	}
      }
      void addWriteAccess(Epoch *epoch, VAddr dVAddr){
	if(myPosition==Second){
	  // A write can be the second access only in
	  // output- and anti-type races
	  if(raceTypes&(RaceInfo::Output|RaceInfo::Anti)){
	    // This is the second access of the race
	    // and we want it as early as possible
	    // Thus, take the first eligible access in this epoch 
	    if(!writeAccess){
	      writeAccess=
		new TraceAccessEvent(epoch,dVAddr,TraceAccessEvent::Write);
	      epoch->addAccessEvent(writeAccess);
	    }
	  }
	}else{
	  I(myPosition==First);
	  // A write can be the first access only in
	  // output- and flow-type races
	  if(raceTypes&(RaceInfo::Output|RaceInfo::Flow)){
	    // This is the first access of the race
	    // and we want it as late as possible
	    // Thus, take the last eligible access in this epoch 
	    epoch->removeAccessEvent(writeAccess);
	    delete writeAccess;
	    writeAccess=
	      new TraceAccessEvent(epoch,dVAddr,TraceAccessEvent::Write);
	    epoch->addAccessEvent(writeAccess);
	  }
	}
      }
    };
    typedef std::map<VAddr,RaceAddrInfo *> RaceByAddrEp;
    
    RaceByEpAddr myForwRacesByEpAddr;
    RaceByAddrEp myForwRacesByAddrEp;
    RaceByEpAddr myBackRacesByEpAddr;
    RaceByAddrEp myBackRacesByAddrEp;
    
    void addRace(Epoch *epoch1, Epoch *epoch2,
		 VAddr dVAddr, RaceInfo::RaceType raceType){
      RaceByAddr &forwRaceByAddr=epoch1->myForwRacesByEpAddr[epoch2];
      std::pair<RaceByAddr::iterator,bool> insOutcome=
	forwRaceByAddr.insert(RaceByAddr::value_type(dVAddr,(RaceInfo *)0));
      if(insOutcome.second){
	// If the element was actually inserted, create a new ReaceInfo
	RaceInfo *raceInfo=new RaceInfo(dVAddr,epoch1,epoch2,raceType);
	// Insert it into all the structures where it needs to be
	insOutcome.first->second=raceInfo;
	epoch2->myBackRacesByEpAddr[epoch1][dVAddr]=raceInfo;
	std::pair<RaceByAddrEp::iterator,bool> forwAddrEpInsOutcome=
	  epoch1->myForwRacesByAddrEp.insert(RaceByAddrEp::value_type(dVAddr,(RaceAddrInfo *)0));
	if(forwAddrEpInsOutcome.second){
	  RaceAddrInfo *forwAddrInfo=
	    new RaceAddrInfo(dVAddr,RaceAddrInfo::First);
	  forwAddrEpInsOutcome.first->second=forwAddrInfo;
	}
	forwAddrEpInsOutcome.first->second->raceByEp[epoch2]=raceInfo;
	forwAddrEpInsOutcome.first->second->addType(raceType);
	std::pair<RaceByAddrEp::iterator,bool> backAddrEpInsOutcome=
	  epoch2->myBackRacesByAddrEp.insert(RaceByAddrEp::value_type(dVAddr,(RaceAddrInfo *)0));
	if(backAddrEpInsOutcome.second){
	  RaceAddrInfo *backAddrInfo=
	    new RaceAddrInfo(dVAddr,RaceAddrInfo::Second);
	  backAddrEpInsOutcome.first->second=backAddrInfo;
	}
	backAddrEpInsOutcome.first->second->raceByEp[epoch1]=raceInfo;
	backAddrEpInsOutcome.first->second->addType(raceType);
      }else{
	// Race info already exists, just update the type
	insOutcome.first->second->addType(raceType);
	I(epoch1->myForwRacesByAddrEp.count(dVAddr));
	I(epoch2->myBackRacesByAddrEp.count(dVAddr));
	epoch1->myForwRacesByAddrEp[dVAddr]->addType(raceType);
	epoch2->myBackRacesByAddrEp[dVAddr]->addType(raceType);
	I(epoch2->myBackRacesByEpAddr[epoch1][dVAddr]==
	  insOutcome.first->second);
	I(epoch1->myForwRacesByAddrEp[dVAddr]->raceByEp[epoch2]==
	  insOutcome.first->second);
	I(epoch2->myBackRacesByAddrEp[dVAddr]->raceByEp[epoch1]==
	  insOutcome.first->second);
      }
    }
    bool hasNewRaces(void){
      // If no races at all, return false
      if(myForwRacesByAddrEp.empty()&&myBackRacesByAddrEp.empty())
	return false;
      // If any forward race address has not been traced already, return true
      for(RaceByAddrEp::iterator forwIt=myForwRacesByAddrEp.begin();
	  forwIt!=myForwRacesByAddrEp.end();forwIt++){
	VAddr dVAddr=forwIt->first;
	if(!traceDataAddresses.count(dVAddr))
	  return true;
      }
      // If any forward race address has not been traced already, return true
      for(RaceByAddrEp::iterator backIt=myBackRacesByAddrEp.begin();
	  backIt!=myBackRacesByAddrEp.end();backIt++){
	VAddr dVAddr=backIt->first;
	if(!traceDataAddresses.count(dVAddr))
	  return true;
      }
      return false;
    }
    void extractRaceAddresses(void){
      for(RaceByAddrEp::iterator forwIt=myForwRacesByAddrEp.begin();
	  forwIt!=myForwRacesByAddrEp.end();forwIt++){
	VAddr dVAddr=forwIt->first;
	traceDataAddresses.insert(dVAddr);
      }
      for(RaceByAddrEp::iterator backIt=myBackRacesByAddrEp.begin();
	  backIt!=myBackRacesByAddrEp.end();backIt++){
	VAddr dVAddr=backIt->first;
	traceDataAddresses.insert(dVAddr);
      }
    }
  public:
    // Prepare to read from this version. Returns the address to read from.
    RAddr read(VAddr iVAddr, short iFlags,
		 VAddr dVAddr, Address dAddrR);
    
    // Prepare to write to this version. Returns the address to write to.
    RAddr write(VAddr iVAddr, short iFlags,
		  VAddr dVAddr, Address dAddrR);

    void pendInstr(void){
      I(myState==State::Running);
      pendInstrCount++;
      I(pendInstrCount>execInstrCount);
    }
    void unPendInstr(void){
      I((myState==State::Running)||(myState==State::Completed));
      pendInstrCount--;
      I(pendInstrCount>=execInstrCount);
      I(pendInstrCount>=doneInstrCount);
      if(myState==State::Completed)
	tryCommit();
    }
    void execInstr(void){
      I((myState!=State::Spawning)||(execInstrCount<0));
      I((!maxInstrCount)||(myState!=State::FullReplay)||(pendInstrCount<=maxInstrCount));
      I(execInstrCount<pendInstrCount);
      execInstrCount++;
      if(execInstrCount>maxEpochInstrCount)
	maxEpochInstrCount=execInstrCount;
      // If epoch has exceeded its instruction count, end it if possible
      if(maxInstrCount&&(execInstrCount>=maxInstrCount)&&canBeTruncated())
	changeEpoch();
      // Epoch is prevented from becoming GlobalSafe until it executes 1st instr
      // So once the first instr is executed, we must try to grab GlobalSafe
      if(execInstrCount==1)
	tryGlobalSave();
      if(squashToTestRollback){
	Epoch *squashEpoch=squashToTestRollback;
	squashToTestRollback=0;
	squashEpoch->squash(false);
      }
    }
    void doneInstr(void){
      I((myState!=State::Spawning)||(doneInstrCount<0));
      I(doneInstrCount<=execInstrCount);
      doneInstrCount++;
      if((myState==State::WaitUnspawn)&&(doneInstrCount==pendInstrCount)){
	//delete this;
      }else if(myState==State::Completed){
	tryCommit();
	if(squashToTestRollback){
          Epoch *squashEpoch=squashToTestRollback;
          squashToTestRollback=0;
          squashEpoch->squash(false);
        }
      }
    }
    void removeInstr(void){
      I(execInstrCount<pendInstrCount);
      I(execInstrCount);
      pendInstrCount--;
      execInstrCount--;
      doneInstrCount--;
    }
    void insertInstr(void){
      I((!maxInstrCount)||(pendInstrCount<maxInstrCount));
      I((!maxInstrCount)||(execInstrCount<=pendInstrCount));
      pendInstrCount++;
      execInstrCount++;
      doneInstrCount++;
    }

    // Adds the event to this epoch's list of trace events
    void addTraceEvent(TraceEvent *traceEvent){
      myTrace.push_back(traceEvent);
    }
    void clearTrace(void){
      while(!myTrace.empty()){
	TraceEvent *traceEvent=myTrace.front();
	myTrace.pop_front();
	delete traceEvent;
      }
    }
    typedef std::set<TraceAccessEvent *> EventSet;
    EventSet accessEvents;
    void addAccessEvent(TraceAccessEvent *event){
      accessEvents.insert(event);
    }
    void removeAccessEvent(TraceAccessEvent *event){
      accessEvents.erase(event);
    }
  };

}
#endif
