#include "Signature.h"
#include "ReportGen.h"
#include "EnergyMgr.h"
#include <stdio.h>

SignatureVector::SignatureVector(){
  // Clear all bits
  for(int32_t i = 0; i < 16; i++){
    signator[i] = 0;
  }// End Array
}

SignatureVector::~SignatureVector(){}


void SignatureVector::setPCBit(uint32_t pc)
{
  setBit(hashPC(pc));
}

int32_t SignatureVector::hashPC(uint32_t pc)
{
#ifdef ADVANCED_HASH
  uint32_t temp9;
  uint32_t temp3High;
  uint32_t temp3Mid;
  uint32_t temp3Low;
  uint32_t resultTri;
  uint32_t result;

  result = 0;

  // Get Higher order bits (highest 9 bits)
  temp9 = (pc & 0xFF800000) >> 23;
  // Split 9 bit into 3 catigories
  temp3High = (temp9 & 0x1C0) >> 6;
  temp3Mid = (temp9 & 0x38) >> 3;
  temp3Low = (temp9 & 0x7);
  // Get 3 bit pattern
  resultTri = (temp3High ^ temp3Mid) ^ temp3Low;
   result = result | (resultTri << 6);

  // Get Mid order bits (highest 9 bits)
  temp9 = (pc & 0x007FC000) >> 14;
  // Split 9 bit into 3 catigories
  temp3High = (temp9 & 0x1C0) >> 6;
  temp3Mid = (temp9 & 0x38) >> 3;
  temp3Low = (temp9 & 0x7);
  // Get 3 bit pattern
  resultTri = (temp3High ^ temp3Mid) ^ temp3Low;
  result = result | (resultTri << 3);

  // Get low order bits (highest 9 bits)
  temp9 = (pc & 0x00006FFF)  >> 5;
  // Split 9 bit into 3 catigories
  temp3High = (temp9 & 0x1C0) >> 6;
  temp3Mid = (temp9 & 0x38) >> 3;
  temp3Low = (temp9 & 0x7);
  // Get 3 bit pattern
  resultTri = (temp3High ^ temp3Mid) ^ temp3Low;
  result = result | resultTri;

  return result;
#else
  // Get only 9 bits (0x1ff)
  return (((pc>>13) ^ pc)>>4) & 0x1FF;
#endif
}

#if 0
int32_t SignatureVector::hashPC(uint32_t pc){
  uint32_t t1, bits_4, bits_3, bits_2;
  uint32_t result = 0;
  
  t1 = (pc & 0xFFFF0000) >> 16;
  bits_4 = 0;
  for(int32_t i = 0; i < 4; i++){
    bits_4 = (t1 & 0xF) ^ bits_4; 
    t1 = t1 >> 4;
  }
 
  t1 = (pc & 0x0000FF80) >> 7;
  bits_3 = 0;
  for(int32_t i = 0; i <3 ; i++){
    bits_3 = (t1 & 0x7) ^ bits_3;
    t1 = t1 >> 3;
  }

  t1 = (pc & 0x0000007D)  ;
  bits_2 = 0;
  for(int32_t i = 0; i < 2; i++){
    bits_2 = (t1 & 0x3) ^ bits_2;
    t1 = t1 >> 2 ;
  }
 
  result = (bits_4 << 5) | (bits_3 << 2)|  bits_2;
  //printf("Results:%d\n", result);
  return result;
}
#endif

void SignatureVector::setBit(int32_t index)
{
  int32_t arrayIndex  = 0;
  int32_t subIndex    = 0;
  int32_t range_count = 32;

//printf("Set bit:%d", index);

  I(index <= 511);

  // Get array index (0-15)
  for(arrayIndex = 0; arrayIndex < 16; arrayIndex++){
    if (index < range_count)
      break;
    range_count += 32;
  }                                                       
  // Get index in subArray
  //printf("Index:%d RangeCounts:%d\n", index, range_count);
  if(range_count == 32){
    subIndex = index;
  }else{
    subIndex = index % (range_count - 32);
  }
  // Set Bit to onern 
  signator[arrayIndex] = signator[arrayIndex] | (1 << subIndex);
}

int32_t SignatureVector::getNumberOfBits()
{
  I(0);

  int32_t bitCount = 0;
  int32_t tempLong = 0;
  for(int32_t i = 0; i < 16; i++){
    tempLong =signator[i];
    for(int32_t j = 0; j < 32; j++){
      if(tempLong & 0x80000000){
        bitCount++;
      }
      tempLong = tempLong << 1;
    }// End long
  }// End Array
  
  return 1;
}

int32_t SignatureVector::getNumberOfBits(uint32_t input)
{
  int32_t bitCount = 0;
  uint32_t tempLong = 0;
 
  tempLong =input;
  for(int32_t j = 0; j < sizeof(uint32_t); j++) {
    
    bitCount = bitCount + (tempLong & 0x1);

    tempLong = tempLong >> 1;
  }// End long
  
  return bitCount;
}

float SignatureVector::getSignatureDifference(uint32_t compareSig[])
{
  float percentDif = 0.0;
  int32_t bitCountCommon = 0;
  int32_t bitCountDiff = 0;

  uint32_t bitsCommon[16];
  uint32_t bitsDiff[16];


  for(int32_t i = 0; i < 16; i++){
    bitsDiff[i]   = signator[i] ^ compareSig[i];
    bitsCommon[i] = signator[i] & compareSig[i];
    
    bitCountCommon += getNumberOfBits(bitsCommon[i]);
    bitCountDiff   += getNumberOfBits(bitsDiff[i]);
   //printf("BtCommon:%d, BitDiff:%d\n",bitCountCommon, bitCountDiff); 
  }// End For

 
  if(bitCountCommon == 0)
  	return 1.0;
  	
  percentDif = (float)bitCountDiff / (float)bitCountCommon;

  return percentDif;
}

uint32_t* SignatureVector::getSignature(){
	return signator;
}

int32_t SignatureVector::matchBits(uint32_t compareSig[16]){

  uint32_t tempLong;
  uint32_t tempComp;
  for(int32_t i = 0; i < 16; i++){
    tempLong = signator[i];
    tempComp = compareSig[i];
      if(tempLong ^ tempComp){
        return -1;
      }
  }// End Array

  return 1;
}

int32_t SignatureVector:: copySignature(uint32_t copySig[]){
  for(int32_t i = 0; i < 16; i++){
    signator[i] = copySig[i];
  }// End Array
  
  return 1;
}

int32_t SignatureVector:: clearBits(){
  for(int32_t i = 0; i < 16; i++){
    signator[i] = 0;
  }// End Array
  
  return 1;
}


//======================================
SignatureTable::SignatureTable()
{
  nextUpdatePos = 0;
}

SignatureTable::~SignatureTable()
{
  for(int32_t i = 0; i < 128; i++){
    //   delete(&sigEntries[i]);
  }
}

int32_t SignatureTable::isSignatureInTable(SignatureVector vec)
{
  SignatureEntry *entry;
  for(int32_t i = 0; i < 128; i++){
    entry = &sigEntries[i];
    if(entry->taken != 0){
      if(entry->sigVec.matchBits(vec.getSignature())){
	return i;
      }
    }
  }
  return -1;
}

int32_t SignatureTable::getSignatureMode(int32_t index)
{
  SignatureEntry *entry = &sigEntries[index];
  if(entry->taken == 0)
    return -1;

  return entry->mode;
}

int32_t SignatureTable::addSignatureVector(SignatureVector vec, int32_t mode)
{
  SignatureEntry *entry = &sigEntries[nextUpdatePos];
  //vec.copySignature(entry->sigVec.getSignature()); // Need to make a copy constructor
  entry->sigVec.copySignature(vec.getSignature());
  entry->taken = 1;
  entry->mode = mode;
  
  nextUpdatePos = (nextUpdatePos + 1) & 0xFF; // 128
  
  return 1;
}

//===========================================================================
//
//===========================================================================
PipeLineSelector::PipeLineSelector()
{
  clockCount  = 0;
  inOrderEDD  = 0;
  outOrderEDD = 0;
  totEnergy = 0;
  windowInstCount = 0;
  currPipelineMode = 0; 
  trainWindowCount = 0;
  currState = 0;
  inOrderWindowCount  = 0;
  outOrderWindowCount = 0;
}

PipeLineSelector::~PipeLineSelector()
{
}

//=====================================================
void PipeLineSelector::updateCurrSignature(uint32_t pc)
{
  signatureCurr.setPCBit(pc);
}


double PipeLineSelector::calculateEDD(int32_t currClockCount, double currTotEnergy)
{ 
  double caculatedEDD = 0;
  //double energy =  GStatsEnergy::getTotalEnergy();
  
  double delta_energy = currTotEnergy - totEnergy;       
  int32_t  delta_time = currClockCount - clockCount;
  caculatedEDD = (delta_energy/100) * ((double)delta_time/100.0* (double)delta_time/100.0);
  
  return caculatedEDD;
}

void PipeLineSelector::report(const char* str)
{
  printf("Printing pipelineslector data\n");

  Report::field("inOrderWindowCount=%ld",inOrderWindowCount);
  Report::field("outOrderWindowCount=%ld",outOrderWindowCount);
}

//====================================================================================
int32_t PipeLineSelector::getPipeLineMode(uint32_t pc)
{ 
 

  // called on every pc dispatch
  if(pc == 0)
    return currPipelineMode;
  
  updateCurrSignature(pc);
  windowInstCount++;
  
  // Check if we are at an end of a window interval
  if(windowInstCount != 1000)
    return currPipelineMode;

   double currTotEnergy =0;

	int32_t currClockCount = globalClock;
  // FIXME: begin of getPipeLineMode (move previous instructions to updateHashPC)
  float signatureDiff = 0.0;
  int32_t tableIndex;

  windowInstCount = 0;
  // We can calclulate the edd for the last window
  if(currPipelineMode == INORDER){
    inOrderEDD = calculateEDD(currClockCount, currTotEnergy); 
  }else{
    outOrderEDD = calculateEDD(currClockCount, currTotEnergy); 
  }	
  
  // Get Signature difference
  signatureDiff = signaturePrev.getSignatureDifference(signatureCurr.getSignature());
  printf("Signature diff:%f\n", signatureDiff);                
  if(signatureDiff > 0.5) {
    //==========================
    // Phase Change Detected
    //=========================
    //  printf("Phase change detected\n");
    pipelineSelectorEnergy->inc();          
    // Check if signature is in table
    if((tableIndex = table.isSignatureInTable(signatureCurr)) == 1){
      // Signature found
      printf("Found Siganature\n");
      currPipelineMode = table.getSignatureMode(tableIndex);  
      currState = STABLE;             
      
    }else{
      if(currState == STABLE){
	// Signature NOT FOUND! (NEED TO TRAIN)
	// reset statiscsi
	printf("Start training\n");
	currState = TRAINING;
	currPipelineMode = INORDER;
	trainWindowCount = 0;
	}else{
	// Already Training  (need to reset training) 
	// reset statiscs
	printf("Start trianing\n"); 
	currState = TRAINING;
	currPipelineMode = INORDER;
	trainWindowCount = 0;
      }
    }// END Signature found 
    
    signaturePrev.copySignature(signatureCurr.getSignature());
    
  }else{
    //==========================
    // NO Phase Change
    //=========================
    if(currState == TRAINING){
      switch(trainWindowCount){
      case 0:
	// capture statiics
	printf("Training Step 0\n");
	clockCount = currClockCount;
	totEnergy = currTotEnergy;
	currPipelineMode = INORDER;
	break;
      case 1:
	printf("Training Step 1\n");
	  // compute inorder edd, swithc to outorder  	
	inOrderEDD = calculateEDD(currClockCount, currTotEnergy);               	  
	currPipelineMode = OUTORDER;
	break;
      case 2:
	printf("Traing Step 2\n");
	// capture statistics
	clockCount = currClockCount;
	totEnergy = currTotEnergy;
	currPipelineMode = OUTORDER;
	break;
      case 3:
	printf("Training Step 3\n");
	// compute out of order edd, find best, add to table
	outOrderEDD = calculateEDD(currClockCount, currTotEnergy);   
	printf("InorderEDD:%f, OutOrderEDD:%f\n", inOrderEDD, outOrderEDD); 
	if(inOrderEDD < outOrderEDD){                 	
	  currPipelineMode = INORDER;
	}else{
	  currPipelineMode = OUTORDER;
	}
	// Add signuter to table
	table.addSignatureVector(signatureCurr, currPipelineMode);	
	currState = STABLE;
	break;
 	
	}// End Switch
      ++trainWindowCount;                                                        
    }else{
      // printf("No phase change\n");
    }
  }/* EnD No Phase Change */
  
  // Copy curr signister to prev, reset currentSignature fo rnew window
  // printf("Clering signature\n");
  signatureCurr.clearBits();    
  windowInstCount = 0; 
  
  if(currPipelineMode == INORDER){
    ++inOrderWindowCount;
    //   printf("inOrderWindowCount:%ld\n",inOrderWindowCount);
  }else{
    ++outOrderWindowCount;
      //	printf("outOrderWindowCount:%ld\n",outOrderWindowCount);
  } 

  return currPipelineMode;
}


