#include <unistd.h>
#include <fcntl.h>
#include "SescConf.h"
#include "ThreadContext.h"
#include "ElfObject.h"
//#include "MipsSysCalls.h"
//#include "MipsRegs.h" 
#include "EmulInit.h"
#include "FileSys.h"

// For Mips32_STD*_FILENO
//#include "Mips32Defs.h"

void fail(const char *fmt, ...){
  va_list ap;
  fflush(stdout);
  fprintf(stderr, "\nERROR: ");
  va_start(ap, fmt);
  vfprintf(stderr,fmt,ap);
  va_end(ap);
  exit(1);
}

void emulInit(int32_t argc, char **argv, char **envp){
  FileSys::Node::insert("/dev/null",new FileSys::NullNode());
  FileSys::Node::insert("/dev/tty",0);

  FileSys::Description *inDescription=0;
  FileSys::Description *outDescription=0;
  FileSys::Description *errDescription=0;

  extern char *optarg;
  int32_t opt;
  while((opt=getopt(argc, argv, "+hi:o:e:"))!=-1){
    switch(opt){
    case 'i':
      inDescription=FileSys::Description::open(optarg,O_RDONLY,S_IRUSR);
      if(!inDescription)
	fail("Could not open `%s' as simulated stdin file\n",optarg);
      break;
    case 'o':
      outDescription=FileSys::Description::open(optarg,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
      if(!outDescription)
	fail("Could not open `%s' as simulated stdout file\n",optarg);
      break;
    case 'e':
      errDescription=FileSys::Description::open(optarg,O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
      if(!errDescription)
	fail("Could not open `%s' as simulated stderr file %s\n",optarg);
      break;
    case 'h':
    default:
      fail("\n"
	   "Usage: %s [EmulOpts] [-- SimOpts] AppExec [AppOpts]\n"
	   "  EmulOpts:\n"
	   "  [-i FName] Use file FName as stdin  for AppExec\n"
	   "  [-o FName] Use file FName as stdout for AppExec\n"
	   "  [-e FName] Use file FName as stderr for AppExec\n",
	   argv[0]);
    }
  }
  if(!inDescription){
    inDescription=FileSys::TtyDescription::wrap(STDIN_FILENO);
    if(!inDescription)
      fail("Could not wrap stdin\n");
  }
  if(!outDescription){
    outDescription=FileSys::TtyDescription::wrap(STDOUT_FILENO);
    if(!outDescription)
      fail("Could not wrap stdout\n");
  }
  if(!errDescription){
    errDescription=FileSys::TtyDescription::wrap(STDERR_FILENO);
    if(!errDescription)
      fail("Could not wrap stderr\n");
  }
  int32_t    appArgc=argc-optind;
  char **appArgv=&(argv[optind]);
  char **appEnvp=envp;
  // Count environment variables
  int32_t    appEnvc=0;
  while(appEnvp[appEnvc])
    appEnvc++;

  FileSys::NameSpace::pointer nameSpace(new FileSys::NameSpace(SescConf->getCharPtr("FileSys","mount")));
  char hostCwd[PATH_MAX];
  if(getcwd(hostCwd,PATH_MAX)==0)
    fail("emulInit: Failed to get host current directory (getcwd)\n");
  const string targetCwd(nameSpace->toTarget(nameSpace->normalize("/",hostCwd)));
  FileSys::FileSys::pointer fileSys(new FileSys::FileSys(nameSpace,targetCwd));
  const string exeLinkName(fileSys->toHost(appArgv[0]));
  const string exeRealName(FileSys::Node::resolve(exeLinkName));
  if(exeRealName.empty())
    fail("emulInit: Link loop when executable %s\n",exeLinkName.c_str());
  FileSys::Node *node=FileSys::Node::lookup(exeRealName);
  if(!node)
    fail("emulInit: Executable %s does not exist\n",exeLinkName.c_str());
  FileSys::FileNode *fnode=dynamic_cast<FileSys::FileNode *>(node);
  if(!fnode)
    fail("emulInit: Executable %s is not a regular file\n",exeLinkName.c_str());
  FileSys::FileDescription *fdesc=new FileSys::FileDescription(fnode,O_RDONLY);
  FileSys::Description::pointer pdesc(fdesc);
  ThreadContext *mainThread=new ThreadContext(fileSys);
  // TODO: Use ELF_ET_DYN_BASE instead of a constant here
  loadElfObject(mainThread,fdesc,0x200000);
  mainThread->getSystem()->initSystem(mainThread);
  mainThread->getSystem()->createStack(mainThread);
  mainThread->getSystem()->setProgArgs(mainThread,appArgc,appArgv,appEnvc,appEnvp);
  FileSys::OpenFiles *openFiles=mainThread->getOpenFiles();
  openFiles->openDescriptor(STDIN_FILENO,inDescription);
  openFiles->openDescriptor(STDOUT_FILENO,outDescription);
  openFiles->openDescriptor(STDERR_FILENO,errDescription);
}
