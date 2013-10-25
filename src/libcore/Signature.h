
#ifndef SIGNATURE_H
#define SIGNATURE_H


#define OUTORDER 0
#define INORDER 1

#define STABLE 0
#define TRAINING 1

#define WINDOW_SIZE 10000

//==================================================================
//
//==================================================================
class SignatureVector {
 private:
  uint32_t signator[16];
 public:
  SignatureVector();
  ~SignatureVector();
  
  uint32_t* getSignature();
  
  int32_t  hashPC(uint32_t pc);

  void setPCBit(uint32_t pc); // Calls hashPC
  void setBit(int32_t index);

  int32_t getNumberOfBits();
  int32_t getNumberOfBits(uint32_t input);
  int32_t matchBits(uint32_t compareSig[]);

  float getSignatureDifference(uint32_t compareSig[]);
  int32_t copySignature(uint32_t copySig[]);
  int32_t clearBits();
};


//==================================================================
//
//==================================================================
class SignatureEntry{
public:
  SignatureVector sigVec;
  int32_t mode;
  int32_t taken;
  int32_t accessTimes; // For LRU policy (to be implemented )
  
  SignatureEntry(){               
    mode = -1;
    taken = 0;
    accessTimes = 0;
  }
  
  ~SignatureEntry(){
  }
};

class SignatureTable {
 private:
  SignatureEntry sigEntries[128];
  int32_t nextUpdatePos;

 public:
  /* Return -1, index in array */
  SignatureTable();
  ~SignatureTable();
  int32_t isSignatureInTable(SignatureVector signature);
  int32_t addSignatureVector(SignatureVector signature, int32_t mode);
  int32_t getSignatureMode(int32_t index);
};

//==========================================================================
//
//==========================================================================
class PipeLineSelector {
 private:

  SignatureVector signatureCurr;
  SignatureVector signaturePrev;
  SignatureTable table;
  
  double totEnergy;
  int32_t clockCount;
  double inOrderEDD, outOrderEDD; // Energy * delay^2
  
  int32_t windowInstCount; // counter from 0 - 10,000
  int32_t trainWindowCount;  
  int32_t currPipelineMode, currState; 
  
  void updateCurrSignature(uint32_t pc);
  double calculateEDD(int32_t currClockCount, double currTotEnergy);
        
 public:
  PipeLineSelector();
  ~PipeLineSelector();
  int32_t getPipeLineMode(uint32_t pc, int32_t currClockCount, double currTotEnergy); // called on every clock
};

#endif   // SIGNATURE_H

