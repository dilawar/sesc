#ifndef CACHEFLAGS_H
#define CACHEFLAGS_H
//#include "Epoch.h"
namespace tls{
	class Epoch;
	
	typedef long int32_t ClockValue;
	//Template disabled because of vector object container problems
	//template <uint32_t blksize=32, uint32_t wordsize=4>
	class CacheFlags
	{
	  static const int32_t blksize=32;
	  static const int32_t wordsize=4;
	  protected:
	   uint16_t int32_t wmFlags[blksize/wordsize];
	   uint16_t int32_t erFlags[blksize/wordsize];
	   uint8_t oeFlags;
	   Epoch *epoch; 
	public:
		CacheFlags() 
		{
			int32_t allocsize=(blksize/wordsize);
			epoch=0;
			oeFlags=0;
			for (int32_t i=0;i<allocsize;i++)
			{
				wmFlags[i]=0;
				erFlags[i]=0;
			}
		}
		//Copy Constructor
		CacheFlags(const CacheFlags &cp)
		{
			int32_t allocsize=(blksize/wordsize);
			epoch=cp.getEpoch();
			oeFlags=cp.getOE();
			for (int32_t i=0;i<allocsize;i++)
			{
				wmFlags[i]=cp.getWordWM(i);
				erFlags[i]=cp.getWordER(i);
			}
		}
		CacheFlags& operator=(const CacheFlags& cf) 
		{
			int32_t allocsize=(blksize/wordsize);
			epoch=cf.getEpoch();
			oeFlags=cf.getOE();
			for (int32_t i=0;i<allocsize;i++)
			{
				wmFlags[i]=cf.getWordWM(i);
				erFlags[i]=cf.getWordER(i);
			}
			return *this;
		}
		bool operator==(const CacheFlags& cf) const
		{
			int32_t allocsize=(blksize/wordsize);
			if (epoch!=cf.getEpoch() || oeFlags!=cf.getOE())
				return false;
			for (int32_t i=0;i<allocsize;i++)
			{
				if (wmFlags[i]!=cf.getWordWM(i) ||erFlags[i]!=cf.getWordER(i))
					return false;
			}
			return true;
		}
		bool operator!=(const CacheFlags& cf) const
		{
			int32_t allocsize=(blksize/wordsize);
			if (epoch!=cf.getEpoch() || oeFlags!=cf.getOE())
				return true;
			for (int32_t i=0;i<allocsize;i++)
			{
				if (wmFlags[i]!=cf.getWordWM(i) ||erFlags[i]!=cf.getWordER(i))
					return true;
			}
			return false;
		}
		~CacheFlags()
		{
			//delete erFlags;
			///elete wmFlags;
		}
		void setEpoch(Epoch* myepoch){
			I(myepoch);
			epoch=myepoch;
			}
		void setWordER(long int32_t pos)
		{
			//pos is assumed to be byte offset from 1st byte of line
			long int32_t pos1=pos/(long)wordsize;
			if (wmFlags[pos1]!=1)
			{
				erFlags[pos1]=1;
			}
		}
		
		void setWordWM(long int32_t pos)
		{
			long int32_t pos1=pos/(long)wordsize;
			wmFlags[pos]=1;
		}
		
		void setOE(void)
		{
			oeFlags=1;
		}
		void clearOE(void)
		{
			if (oeFlags==1) 
			//printf("OE Flag reset\n");
			oeFlags=0;
		}
		
		uint32_t getWordER(int32_t pos) const
		{
			long int32_t pos1=pos/(long)wordsize;
			return  erFlags[pos1];
		}
		
		uint32_t getWordWM(int32_t pos) const
		{
			long int32_t pos1=pos/(long)wordsize;
			return wmFlags[pos1];
		}
		
		uint32_t getOE(void) const
		{
			return oeFlags;
		}
		//ClockValue getEpochNum()const {return epoch->getClock();}
		Epoch * getEpoch() const{return epoch;}
		void clearFlags()
		{
			unsigned i;
			for (i=0;i<(blksize/wordsize);i++)
			{
				erFlags[i]=0;
				wmFlags[i]=0;
			}
			oeFlags=0;
			epoch=0;
			
		}
	};
}
#endif
