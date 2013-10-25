
#include <string>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <regex.h>

#include "SescConf.h"
#include "ThermTrace.h"

void ThermTrace::tokenize(const char *str, TokenVectorType &tokens) {

  I(tokens.empty());

  const char *new_pos;
  do {
    if (str[0] == ' ' || str[0] == '\t' || str[0] == '\n') {
      str++;
      continue;
    }

    new_pos = strchr(str, ' ');
    if (new_pos == 0)
      new_pos = strchr(str, '\t');
    else {
      const char *tab_pos = strchr(str, '\t');
      if (tab_pos && tab_pos < new_pos)
	new_pos = tab_pos;
    }
    if (new_pos == 0)
      new_pos = strchr(str, '\n');
    else {
      const char *ret_pos = strchr(str, '\n');
      if (ret_pos && ret_pos < new_pos)
	new_pos = ret_pos;
    }

    if (new_pos == 0) {
      // last string
      tokens.push_back(strdup(str));
      break;
    }

    int32_t len = new_pos-str;
    char *new_str = (char *)malloc(len+4);
    strncpy(new_str,str,len);
    new_str[len]=0;
    tokens.push_back(new_str);

    str = new_pos;

  }while(str[0] != 0);
}

void ThermTrace::read_sesc_variable(const char *input_file) {

  // READ first line of input_file. It has the energies reported by sesc

  input_fd_ = open(input_file, O_RDONLY, S_IRUSR);
  if (input_fd_ <0 ) {
    MSG("Warning: Impossible to open input file %s",input_file);
    return;
  }

  char buffer=0;
  bool eof = false;
  std::string line;
  while( !eof  && buffer != '\n') {
    int32_t s = read(input_fd_, &buffer, 1);
    if ( s == 0)
      eof = true;
    else
      line += buffer;
  }
  if(eof) {
    MSG("Error: Input file %s has invalid format",input_file);
    exit(2);
  }

  TokenVectorType sesc_variable;
  tokenize(line.c_str(), sesc_variable);
  if (sesc_variable.empty()) {
    MSG("Error: Input file %s does not have input variable", input_file);
    exit(2);
  }

  mapping.resize(sesc_variable.size());
  for(size_t j=0; j<sesc_variable.size(); j++) {
    LOG("variable[%d]=[%s] ",j, sesc_variable[j]);
    mapping[j].name = strdup(sesc_variable[j]);
  }
}

bool ThermTrace::grep(const char *line, const char *pattern) {

  int32_t    status;
  regex_t    re;

  if (regcomp(&re, pattern, REG_NOSUB|REG_ICASE) != 0)
    return false;

  status = regexec(&re, line, (size_t) 0, NULL, 0);
  regfree(&re);
  if (status != 0)
    return false;

  return true;
}

const ThermTrace::FLPUnit *ThermTrace::findBlock(const char *name) const {

  for(size_t id=0;id<flp.size();id++) {
    if (strcasecmp(name, flp[id]->name) == 0)
      return flp[id];
  }

  return 0; // Not found
}

void ThermTrace::read_floorplan_mapping() {

  GI(input_file_[0]!=0,!mapping.empty()); // first call read_sesc_variable

  const char *flpSec = SescConf->getCharPtr("","floorplan");
  size_t min = SescConf->getRecordMin(flpSec,"blockDescr");
  size_t max = SescConf->getRecordMax(flpSec,"blockDescr");

  // Floor plan parameters
  for(size_t id=min;id<=max;id++) {
    if (!SescConf->checkCharPtr(flpSec,"blockDescr", id)) {
      MSG("There is a WHOLE on the floorplan. This can create problems blockDescr[%d]", id);
      exit(-1);
      continue;
    }

    const char *blockDescr = SescConf->getCharPtr(flpSec,"blockDescr", id);
    TokenVectorType descr;
    tokenize(blockDescr, descr);

    FLPUnit *xflp = new FLPUnit(strdup(descr[0]));
    xflp->id   = id;
    xflp->area = atof(descr[1])*atof(descr[2]);

    xflp->x       = atof(descr[3]);
    xflp->y       = atof(descr[4]);
    xflp->delta_x = atof(descr[1]);
    xflp->delta_y = atof(descr[2]);

    const char *blockMatch = SescConf->getCharPtr(flpSec,"blockMatch", id);
    tokenize(blockMatch, xflp->match);

    flp.push_back(xflp);
  }

  // Find mappings between variables and flp

  for(size_t id=0;id<flp.size();id++) {
    for(size_t i=0; i<flp[id]->match.size(); i++) {
      for(size_t j=0; j<mapping.size(); j++) {
	if (grep(mapping[j].name, flp[id]->match[i])) {

#ifdef DEBUG
	  MSG("mapping[%d].map[%d]=%d (%s -> %s)", 
	      j, mapping[j].map.size(), id, flp[id]->match[i], mapping[j].name);
#endif

	  I(id < flp.size());
	  flp[id]->units++;
	  I(j < mapping.size());
	  mapping[j].area += flp[id]->area;
	  mapping[j].map.push_back(id);
	}
      }
    }
  }

  for(size_t i=0;i<mapping.size();i++) {
    for(size_t j=0; j<mapping[i].map.size(); j++) {
      float ratio = flp[mapping[i].map[j]]->area/mapping[i].area;
      mapping[i].ratio.push_back(ratio);
    }
  }

  for(size_t j=0; j<mapping.size(); j++) {
    I(mapping[j].map.size() == mapping[j].ratio.size());
    if (mapping[j].map.empty()) {
      MSG("Error: sesc variable %s [%d] does not update any block", mapping[j].name, j);
      exit(3);
    }
  }
}

ThermTrace::ThermTrace(const char *input_file)
  : input_file_(strdup(input_file?input_file:"")) {
	
  if( input_file) {
    read_sesc_variable(input_file);
  }
    
  read_floorplan_mapping();
}

void ThermTrace::dump() const {

  for(size_t i=0;i<flp.size();i++) {
    flp[i]->dump();
  }

  for(size_t i=0;i<mapping.size();i++) {
    mapping[i].dump();
  }
  
}

bool ThermTrace::read_energy() {

  float buffer[mapping.size()];

  I(input_fd_>=0);

  int32_t s = read(input_fd_, 
	       buffer, 
	       sizeof(float)*mapping.size());

  if (s != (int)(sizeof(float)*mapping.size()))
    return false;

  // Do mapping
  for(size_t k=0;k<flp.size();k++) {
    flp[k]->energy = 0;
  }

  for(size_t i=0;i<mapping.size();i++) {

    for(size_t k=0;k<mapping[i].map.size();k++) {
      int32_t flp_id  = mapping[i].map[k];

      flp[flp_id]->energy += mapping[i].ratio[k]*buffer[i];
    }
  }

#if 0
  printf("[");
  for(size_t i=0;i<mapping.size();i++) {
    printf(" %g",buffer[i]);
  }
  printf("]\n");
#endif

#if 0
  printf("[");
  for(size_t i=0;i<flp.size();i++) {
    printf(" %g",flp[i]->area);
  }
  printf("]\n");
#endif

  return true;
}
