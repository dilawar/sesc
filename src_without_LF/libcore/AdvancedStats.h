#ifndef _AdvancedStats_hpp_
#define _AdvancedStats_hpp_

#include <map>
#include <list>
#include <string>
#include <iostream>
#include <nanassert.h>

using namespace std;

namespace Stats{
  
  typedef long long uint32_t LargeCount;

  class Group{
  protected:
    Group *parentGroup;
    char *myName;
    typedef list<Group *> GroupList;
    GroupList groupOrder;
    typedef GroupList::iterator GroupListIt;
    typedef map<Group *,GroupListIt> GroupMap;
    GroupMap groupMembers;

    virtual void reportPrefix(size_t level) const;
    virtual void reportMiddle(size_t level) const;
    virtual void reportSuffix(size_t level) const;
    static void indent(size_t num);

  public:
    Group(Group *parentGroup=0, char *name=0);
    virtual void insertMember(Group *newMember);
    virtual void eraseMember(Group *newMember);
    virtual void addSample(const double value) const;
    virtual void addSamples(const double value, const LargeCount count) const;
    virtual void report(size_t level=0) const;
    virtual ~Group(void);
  };
  
  class Distribution : public Group{
  protected:
    size_t numPoints;
    typedef std::map<double,LargeCount> SampleCounts;
    SampleCounts sampleCounts;
    LargeCount totalCount;
    virtual void reportMiddle(size_t level) const;
  public:
    Distribution(Group *parentGroup=0, char *name=0,size_t numPoints=100)
      : Group(parentGroup,name), numPoints(numPoints){
    }
    virtual void addSample(const double value);
    virtual void addSamples(const double value, const LargeCount count);
  };
  
}
#endif
