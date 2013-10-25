/* 
   SESC: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Milos Prvulovic

This file is part of SESC.

SESC is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2, or (at your option) any later version.

SESC is    distributed in the  hope that  it will  be  useful, but  WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should  have received a copy of  the GNU General  Public License along with
SESC; see the file COPYING.  If not, write to the  Free Software Foundation, 59
Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "FileSys.h"

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#include <iostream>
// Needed to get I()
#include "nanassert.h"
// Needed just for "fail()"
#include "EmulInit.h"
// Needed for thread suspend/resume calls
#include "OSSim.h"
// Need to access config info for mount points
#include "SescConf.h"

//#define DEBUG_MOUNTS

using std::cout;
using std::endl;

#include <algorithm>
using std::min;
using std::max;

namespace FileSys {
  size_t Node::simInodeCounter=1;
  Node::ByNatKey   Node::byNatKey;
  Node::ByNatName  Node::byNatName;
  Node::ToNatName  Node::toNatName;
  Node::Node(dev_t dev, uid_t uid, gid_t gid, mode_t mode, ino_t natInode)
    : natInode(natInode), dev(dev), simInode(simInodeCounter++), uid(uid), gid(gid), mode(mode), size(0){
  }
  Node::~Node(void){
  }
  Node *Node::lookup(const std::string &name){
    ByNatName::iterator nameIt=byNatName.find(name);
    if(nameIt!=byNatName.end())
      return nameIt->second;
    Node *node=0;
    struct stat stbuf;
    if(lstat(name.c_str(),&stbuf)!=0)
      return node;
    ByNatKey::iterator keyIt=byNatKey.find(NatKey(stbuf.st_dev,stbuf.st_ino));
    if(keyIt==byNatKey.end()){
      switch(stbuf.st_mode&S_IFMT){
      case S_IFLNK: {
        char lbuf[stbuf.st_size+1];
        if(readlink(name.c_str(),lbuf,stbuf.st_size+1)!=stbuf.st_size)
          fail("Node::lookup realink failed for %s\n",name.c_str());
        lbuf[stbuf.st_size]=(char)0;
        node=new LinkNode(lbuf,stbuf);
      } break;
      case S_IFREG: node=new FileNode(stbuf); break;
      case S_IFDIR: node=new DirectoryNode(stbuf); break;
      default: fail("Node::lookup(%s) for unknown st_mode %x\n",name.c_str(),stbuf.st_mode);
      }
      I(node->natInode!=(ino_t)-1);
      byNatKey[NatKey(node->dev,node->natInode)]=node;
    }else{
      node=keyIt->second;
    }
    byNatName.insert(ByNatName::value_type(name,node));
    toNatName.insert(ToNatName::value_type(node,name));
    return node;
  }
  std::string Node::resolve(const std::string &name){
    std::string res(name);
    for(size_t i=0;i<1024;i++){
      LinkNode *lnode=dynamic_cast<LinkNode *>(lookup(res));
      if(!lnode)
        return res;
      const std::string &lnk=lnode->read();
      if(lnk[0]=='/'){
        res=lnk;
      }else{
        res.resize(res.rfind('/'));
        res=FileSys::normalize(res,lnk);
      }
    }
    return "";
  }
  void Node::insert(const std::string &name, Node *node){
    I(!byNatName.count(name));
    byNatName.insert(ByNatName::value_type(name,node));
    if(node){
      if(node->natInode!=(ino_t)-1){
	I(!byNatKey.count(NatKey(node->dev,node->natInode)));
	byNatKey[NatKey(node->dev,node->natInode)]=node;
      }
      I(!toNatName.count(node));
      toNatName.insert(ToNatName::value_type(node,name));
    }
  }
  void Node::remove(const std::string &name, Node *node){
    I(byNatName.count(name)==1);
    byNatName.erase(name);
    ToNatName::iterator nodeIt=toNatName.lower_bound(node);
    I(nodeIt!=toNatName.end());
    I(nodeIt->first==node);
    while(nodeIt->second!=name){
      nodeIt++;
      I(nodeIt!=toNatName.end());
      I(nodeIt->first==node);
    }
    toNatName.erase(nodeIt);
  }
  const std::string *Node::getName(void){
    ToNatName::const_iterator it=toNatName.lower_bound(this);
    if(it==toNatName.upper_bound(this))
      return 0;
    return &(it->second);
  }
  LinkNode::LinkNode(const std::string &link, struct stat &buf)
   : Node(buf.st_dev,buf.st_uid,buf.st_gid,S_IFLNK|S_IRWXU|S_IRWXG|S_IRWXO,buf.st_ino), link(link){
      setSize(link.length());
  }

  Description *Description::create(Node *node, flags_t flags){
    NullNode *nnode=dynamic_cast<NullNode *>(node);
    if(nnode)
      return new NullDescription(nnode,flags);
    FileNode *fnode=dynamic_cast<FileNode *>(node);
    if(fnode)
      return new FileDescription(fnode,flags);
    DirectoryNode *dnode=dynamic_cast<DirectoryNode *>(node);
    if(dnode)
      return new DirectoryDescription(dnode,flags);
    fail("Description::create for unknown node type\n");
  }

  Description *Description::open(const std::string &name, flags_t flags, mode_t mode){
    Node *node=Node::lookup(name);
    if(!node){
      if(!(flags&O_CREAT)){
        errno=ENOENT;
        return 0;
      }
      fd_t fd=::open(name.c_str(),flags,mode);
      if(fd==-1)
        fail("Description::open could not open %s\n",name.c_str());
      struct stat stbuf;
      if(fstat(fd,&stbuf)!=0)
        fail("Description::open could not fstat %s\n",name.c_str());
      node=new FileNode(stbuf);
      Node::insert(name,node);
      close(fd);
    }else{
      if((flags&O_CREAT)&&(flags&O_EXCL)){
        errno=EEXIST;
        return 0;
      }
    }
    return create(node,flags);
  }
  Description::Description(Node *node, flags_t flags)
    : node(node), flags(flags){
  }
  Description::~Description(void){
  }
  const std::string Description::getName(void) const{
    const std::string *str=node->getName();
    return str?*str:std::string("Anonymous");
  }
  bool Description::canRd(void) const{
    return ((flags&O_ACCMODE)!=O_WRONLY);
  }
  bool Description::canWr(void) const{
    return ((flags&O_ACCMODE)!=O_RDONLY);
  }
  bool Description::isNonBlock(void) const{
    return ((flags&O_NONBLOCK)==O_NONBLOCK);
  }
  flags_t Description::getFlags(void) const{
    return flags;
  }
  NullNode::NullNode()
    : Node(0x000d,0,0,S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH,(ino_t)-1){
  }
  NullNode NullNode::node;
  NullNode *NullNode::create(void){
    return &node;
  }
  NullDescription::NullDescription(NullNode *nnode, flags_t flags)
    : Description(nnode,flags){
  }
  NullDescription::NullDescription(flags_t flags)
    : Description(NullNode::create(),flags){
  }
  ssize_t NullDescription::read(void *buf, size_t count){
    return 0;
  }
  ssize_t NullDescription::write(const void *buf, size_t count){
    return count;
  }
  SeekableNode::SeekableNode(dev_t dev, uid_t uid, gid_t gid, mode_t mode, off_t len, ino_t natInode)
    : Node(dev,uid,gid,mode,natInode){
    Node::setSize(len);
  }
  SeekableDescription::SeekableDescription(Node *node, flags_t flags)
    : Description(node,flags), pos(0){
  }
  off_t SeekableDescription::getSize(void) const{
    return node->getSize();
  }
  off_t SeekableDescription::getPos(void) const{
    return pos;
  }
  void  SeekableDescription::setPos(off_t npos){
    pos=npos;
  }
  ssize_t SeekableDescription::read(void *buf, size_t count){
    ssize_t rcount=pread(buf,count,pos);
    if(rcount>0){
      pos+=rcount;
      I(pos<=getSize());
    }
    return rcount;
  }
  ssize_t SeekableDescription::write(const void *buf, size_t count){
    ssize_t wcount=pwrite(buf,count,pos);
    if(wcount>0){
      pos+=wcount;
      I(pos<=getSize());
    }
    return wcount;
  }
  ssize_t SeekableDescription::pread(void *buf, size_t count, off_t offs){
    return dynamic_cast<SeekableNode *>(node)->pread(buf,count,offs);
  }
  ssize_t SeekableDescription::pwrite(const void *buf, size_t count, off_t offs){
    return dynamic_cast<SeekableNode *>(node)->pwrite(buf,count,offs);
  }
  void SeekableDescription::mmap(void *data, size_t size, off_t offs){
    ssize_t rsize=pread(data,size,offs);
    if(rsize<0)
      fail("FileStatus::mmap failed with error %d\n",errno);
    memset((void *)((char *)data+rsize),0,size-rsize);
  }
  void SeekableDescription::msync(void *data, size_t size, off_t offs){
    off_t endoff=getSize();
    if(offs>=endoff)
      return;
    size_t wsize=(size_t)(endoff-offs);
    if(size<wsize)
      wsize=size;
    ssize_t nbytes=pwrite(data,wsize,offs);
    I(getSize()==endoff);
    I(nbytes==(ssize_t)wsize);
  }
  FileNode::FileNode(struct stat &buf)
    : SeekableNode(buf.st_dev,buf.st_uid,buf.st_gid,buf.st_mode,buf.st_size,buf.st_ino), fd(-1) {
    close(fd);
  }
  FileNode::~FileNode(void){
//    if(close(fd)!=0)
//      fail("FileSys::FileNode destructor could not close file %s\n",name.c_str());
  }
  void FileNode::setSize(off_t nlen){
    if(truncate(getName()->c_str(),nlen)==-1)
      fail("FileNode::setSize truncate failed\n");
    Node::setSize(nlen);
  }
  ssize_t FileNode::pread(void *buf, size_t count, off_t offs){
    fd_t rfd=open(getName()->c_str(),O_RDONLY);
    if(rfd==-1)
      fail("FileNode::pread could not open %s\n",getName()->c_str());
    ssize_t rcount=::pread(rfd,buf,count,offs);
    int     rerror=errno;
    if(close(rfd)!=0)
      fail("FileNode::pread could not close %s\n",getName()->c_str());
    errno=rerror;
    return rcount;
  }
  ssize_t FileNode::pwrite(const void *buf, size_t count, off_t offs){
    fd_t wfd=open(getName()->c_str(),O_WRONLY);
    if(wfd==-1)
      fail("FileNode::pwrite could not open %s\n",getName()->c_str());
    off_t olen=getSize();
    ssize_t wcount=::pwrite(wfd,buf,count,offs);
    int     werror=errno;
    if(close(wfd)!=0)
      fail("FileNode::pwrite could not close %s\n",getName()->c_str());
    if((wcount>0)&&(offs+wcount>olen))
      SeekableNode::setSize(offs+wcount);
    errno=werror;
    return wcount;
  }
  FileDescription::FileDescription(FileNode *node, flags_t flags)
    : SeekableDescription(node,flags){
  }
  FileDescription::~FileDescription(void){
  }
  DirectoryNode::DirectoryNode(struct stat &buf)
    : SeekableNode(buf.st_dev,buf.st_uid,buf.st_gid,buf.st_mode,-1,buf.st_ino), dirp(0), entries(){
  }
  DirectoryNode::~DirectoryNode(void){
  }
  void DirectoryNode::setSize(off_t nlen){
    fail("DirectoryNode::setSize called\n");
  }
  ssize_t DirectoryNode::pread(void *buf, size_t count, off_t offs){
    fail("DirectoryNode::pread called\n");
  }
  ssize_t DirectoryNode::pwrite(const void *buf, size_t count, off_t offs){
    fail("DirectoryNode::pwrite called\n");
  }
  void DirectoryNode::refresh(void){
    DIR *dir=opendir(getName()->c_str());
    entries.clear();
    for(size_t i=0;true;i++){
      struct dirent *dent=readdir(dir);
      if(!dent)
	break;
      entries.push_back(dent->d_name);
    }
    SeekableNode::setSize(entries.size());
    closedir(dir);
  }
  std::string DirectoryNode::getEntry(off_t index){
    if(getSize()==0)
      refresh();
    I((index>=0)&&(entries.size()>(size_t)index));
    return entries[index];
  }
  DirectoryDescription::DirectoryDescription(DirectoryNode *node, flags_t flags)
    : SeekableDescription(node,flags){
  }
  DirectoryDescription::~DirectoryDescription(void){
  }
  off_t DirectoryDescription::getSize(void) const{
    if(node->getSize()==-1)
      dynamic_cast<DirectoryNode *>(node)->refresh();
    return node->getSize();
  }
  void DirectoryDescription::setPos(off_t npos){
    dynamic_cast<DirectoryNode *>(node)->refresh();
    SeekableDescription::setPos(npos);
  }
  std::string DirectoryDescription::readDir(void){
    off_t opos=SeekableDescription::getPos();
    if(opos>=getSize())
      fail("DirectoryDescription::readDir past the end\n");
    SeekableDescription::setPos(opos+1);
    return dynamic_cast<DirectoryNode *>(node)->getEntry(opos);
  }
  void StreamNode::unblock(PidSet &pids, SigCode sigCode){
    for(PidSet::iterator it=pids.begin();it!=pids.end();it++){
      pid_t pid=*it;
      ThreadContext *context=osSim->getContext(pid);
      if(context){   
        SigInfo *sigInfo=new SigInfo(SigIO,sigCode);
	//        sigInfo->pid=pid;
	//        sigInfo->data=fd;     
        context->signal(sigInfo);
      }
    }
    pids.clear();
  }
  StreamNode::StreamNode(dev_t dev, dev_t rdev, uid_t uid, gid_t gid, mode_t mode)
    : Node(dev,uid,gid,mode,(ino_t)-1), rdev(rdev), rdBlocked(), wrBlocked(){
  }
  StreamNode::~StreamNode(void){
    I(rdBlocked.empty());
    I(wrBlocked.empty());
  }
  void StreamNode::rdBlock(pid_t pid){
    I(willRdBlock());
    rdBlocked.push_back(pid);    
  }
  void StreamNode::wrBlock(pid_t pid){
    I(willWrBlock());
    wrBlocked.push_back(pid);
  }
  void StreamNode::rdUnblock(void){
    I(!willRdBlock());
    if(!rdBlocked.empty())
      unblock(rdBlocked,SigCodeIn);
    I(rdBlocked.empty());
  }
  void StreamNode::wrUnblock(void){
    I(!willWrBlock());
    if(!wrBlocked.empty())
      unblock(wrBlocked,SigCodeOut);
    I(wrBlocked.empty());
  }
  bool StreamDescription::willRdBlock(void) const{
    return dynamic_cast<const StreamNode *>(node)->willRdBlock();
  }
  bool StreamDescription::willWrBlock(void) const{
    return dynamic_cast<const StreamNode *>(node)->willWrBlock();
  }
  void StreamDescription::rdBlock(pid_t pid){
    dynamic_cast<StreamNode *>(node)->rdBlock(pid);
  }
  void StreamDescription::wrBlock(pid_t pid){
    dynamic_cast<StreamNode *>(node)->wrBlock(pid);
  }
  StreamDescription::StreamDescription(StreamNode *node, flags_t flags)
    : Description(node,flags){
  }
  ssize_t StreamDescription::read(void *buf, size_t count){
    return dynamic_cast<StreamNode *>(node)->read(buf,count);
  }
  ssize_t StreamDescription::write(const void *buf, size_t count){
    return dynamic_cast<StreamNode *>(node)->write(buf,count);
  }
  
  PipeNode::PipeNode(void)
    : StreamNode(0x0007,0x0000,getuid(),getgid(),S_IFIFO|S_IREAD|S_IWRITE), data(), readers(0), writers(0){
  }
  PipeNode::~PipeNode(void){
    I(!readers);
    I(!writers);
  }
  void PipeNode::addReader(void){
    readers++;
  }
  void PipeNode::delReader(void){
    I(readers);
    readers--;
    if((readers==0)&&(writers==0))
      delete this;
  }
  void PipeNode::addWriter(void){
    writers++;
  }
  void PipeNode::delWriter(void){
    I(writers);
    writers--;
    if((readers==0)&&(writers==0))
      delete this;
  }
  ssize_t PipeNode::read(void *buf, size_t count){
    I(readers);
    if(data.empty()){
      I(!writers);
      return 0;
    }
    size_t ncount=count;
    if(data.size()<ncount)
      ncount=data.size();
    Data::iterator begIt=data.begin();
    Data::iterator endIt=begIt+ncount;
    copy(begIt,endIt,(uint8_t *)buf);
    data.erase(begIt,endIt);
    I(ncount>0);
    wrUnblock();
    return ncount;
  }
  ssize_t PipeNode::write(const void *buf, size_t count){
    I(writers);
    const uint8_t *ptr=(const uint8_t *)buf;
    data.resize(data.size()+count);
    copy(ptr,ptr+count,data.end()-count);
    rdUnblock();
    return count;
  }
  bool PipeNode::willRdBlock(void) const{
    return data.empty()&&writers;
  }
  bool PipeNode::willWrBlock(void) const{
    return false;
  }
  PipeDescription::PipeDescription(PipeNode *node, flags_t flags)
    : StreamDescription(node,flags){
    if(canRd())
      dynamic_cast<PipeNode *>(node)->addReader();
    if(canWr())
      dynamic_cast<PipeNode *>(node)->addWriter();
  }
  PipeDescription::~PipeDescription(void){
    if(canRd())
      dynamic_cast<PipeNode *>(node)->delReader();
    if(canWr())
      dynamic_cast<PipeNode *>(node)->delWriter();
  }
  TtyDescription::TtyDescription(TtyNode *node, flags_t flags)
    : StreamDescription(node,flags){
  }
  TtyDescription::~TtyDescription(void){
  }
  TtyNode::TtyNode(fd_t srcfd)
    : StreamNode(0x009,0x8803,getuid(),getgid(),S_IFCHR|S_IRUSR|S_IWUSR), fd(dup(srcfd)){
    if(fd==-1)
      fail("TtyNode constructor cannot dup()\n");
  }
  TtyNode::~TtyNode(void){
    close(fd);
  }
  ssize_t TtyNode::read(void *buf, size_t count){
    return ::read(fd,buf,count);
  }
  ssize_t TtyNode::write(const void *buf, size_t count){
    return ::write(fd,buf,count);
  }
  bool TtyNode::willRdBlock(void) const{
    struct pollfd pollFd;
    pollFd.fd=fd;
    pollFd.events=POLLIN;
    int32_t res=poll(&pollFd,1,0);
    return (res<=0);    
  }
  bool TtyNode::willWrBlock(void) const{
    struct pollfd pollFd;
    pollFd.fd=fd;
    pollFd.events=POLLOUT;
    int32_t res=poll(&pollFd,1,0);
    return (res<=0);
  }
  TtyDescription *TtyDescription::wrap(fd_t fd){
    flags_t flags=fcntl(fd,F_GETFL);
    if(flags==-1)
      return 0;
    TtyNode *node=new TtyNode(fd);
    return new TtyDescription(node,flags);
  }

  OpenFiles::FileDescriptor::FileDescriptor(void)
    : description(0), cloexec(false){
  }
  OpenFiles::OpenFiles(void)
    : GCObject(), fileDescriptors(){ 
  }
  OpenFiles::OpenFiles(const OpenFiles &src)
    : GCObject(), fileDescriptors(src.fileDescriptors){
  }
  OpenFiles::~OpenFiles(void){
  }
  fd_t OpenFiles::nextFreeFd(fd_t minfd) const{
    for(FileDescriptors::const_iterator it=fileDescriptors.lower_bound(minfd);
        (it!=fileDescriptors.end())&&(it->first==minfd);minfd++,it++);
    return minfd;
  }
  void OpenFiles::openDescriptor(fd_t fd, Description *description){
    I(fd>=0);
    std::pair<FileDescriptors::iterator,bool> res=fileDescriptors.insert(FileDescriptors::value_type(fd,FileDescriptor()));
    I(res.second&&(res.first->first==fd));
    res.first->second.description=description;
    res.first->second.cloexec=false;
  }
  void OpenFiles::closeDescriptor(fd_t fd){
    I(fileDescriptors.count(fd)==1);
    fileDescriptors.erase(fd);
    I(!isOpen(fd));
  }
  bool OpenFiles::isOpen(fd_t fd) const{
    I(fd>=0);
    I((fileDescriptors.count(fd)==0)||fileDescriptors.find(fd)->second.description);
    return fileDescriptors.count(fd);
  }
  Description *OpenFiles::getDescription(fd_t fd){
    FileDescriptors::iterator it=fileDescriptors.find(fd);
    I(it!=fileDescriptors.end());
    I(it->second.description);
    return it->second.description;
  }
  void OpenFiles::setCloexec(fd_t fd, bool cloex){
    FileDescriptors::iterator it=fileDescriptors.find(fd);
    I(it!=fileDescriptors.end());
    I(it->second.description);
    it->second.cloexec=cloex;
  }
  bool OpenFiles::getCloexec(fd_t fd) const{
    FileDescriptors::const_iterator it=fileDescriptors.find(fd);
    I(it!=fileDescriptors.end());
    I(it->second.description);
    return it->second.cloexec;
  }
  void OpenFiles::exec(void){
    FileDescriptors::iterator it=fileDescriptors.begin();
    while(it!=fileDescriptors.end()){
      FileDescriptors::iterator nxit=it;
      nxit++;
      I(it->second.description);
      if(it->second.cloexec)
	fileDescriptors.erase(it);
      it=nxit;
    }
  }
  void OpenFiles::save(ChkWriter &out) const{
    out << "Descriptors: " << fileDescriptors.size() << endl;
    for(FileDescriptors::const_iterator it=fileDescriptors.begin();
        it!=fileDescriptors.end();it++){
      out << "Desc " << it->first << " Cloex " << (it->second.cloexec?'+':'-');
      /*
      BaseStatus *st=it->second.description_old;
      bool hasIndex=out.hasIndex(st);
      out << " Status " << out.getIndex(st) << endl;
      if(!hasIndex)
        st->save(out);
      */
    }
  }
  OpenFiles::OpenFiles(ChkReader &in){
    size_t _size;
    in >> "Descriptors: " >> _size >> endl;
    for(size_t i=0;i<_size;i++){
      fd_t _fd;
      char _cloex;
      in >> "Desc " >> _fd >> " Cloex " >> _cloex;
      /*
      size_t _st;
      in >> " Status " >> _st >> endl;
      BaseStatus *st;
      if(!in.hasObject(_st)){
        in.newObject(_st);
        st=BaseStatus::create(in);
        in.setObject(_st,st);
      }else{
        st=reinterpret_cast<BaseStatus *>(in.getObject(_st));
      }
      fileDescriptors[_fd].description_old=st;
      fileDescriptors[_fd].cloexec=(_cloex=='+');
      */
    }
  }

  NameSpace::NameSpace(const string &mtlist) : GCObject(), mounts() {
    string::size_type bpos=0;
    while(bpos!=string::npos){
      string::size_type mpos=mtlist.find('=',bpos);
      string::size_type epos=mtlist.find(':',bpos);
      string targpath(mtlist,bpos,mpos-bpos);
      string hostpath(mtlist,mpos+1,epos-mpos-1);
      mounts[normalize("",targpath)]=normalize("",hostpath);
      bpos=epos+((epos==string::npos)?0:1);
    }
    if(mounts.find("/")==mounts.end())
      mounts["/"]="/";
#if (defined DEBUG_MOUNTS)
    for(Mounts::const_iterator it=mounts.begin();it!=mounts.end();it++){
      cout << "Target " << it->first <<
	" mounted at " << it->second << endl;
    }
#endif
  }
  NameSpace::NameSpace(const NameSpace &src) : GCObject(), mounts() {
    fail("FileSys::NameSpace copying not supported!\n");
  }
  const string NameSpace::toTarget(const string &fname) const{
#if (defined DEBUG_MOUNTS)
    cout << "toTarget called with " << fname << endl;
#endif
    Mounts::const_iterator found=mounts.end();
    for(Mounts::const_iterator it=mounts.begin();it!=mounts.end();it++){
      // If exact match, no need to look for a better one
      if(fname.compare(it->second)==0)
	return it->first;
      // Root match is good only if no other match found
      if((it->second=="/")&&(found==mounts.end())){
	found=it;
	continue;
      }
      // Host path is not a substring of fname
      if(fname.compare(0,it->second.length(),it->second)!=0)
	continue;
      // Host path is not a sub-path of fname
      if(fname.at(it->second.length())!='/')
	continue;
      // If prior match non-extant or root, now we have a new match
      if((found==mounts.end())||(found->second=="/")){
	found=it;
	continue;
      }
      // If this a better match than the previous one?
      if(fname.length()-it->second.length()+it->first.length()<
	 fname.length()-found->second.length()+found->first.length())
	found=it;
    }
    if(found==mounts.end())
      fail("NameSpace::toTarget could not find mapping for %s\n",fname.c_str());
    string rv;
    if((found->second=="/")&&(found->first!="/"))
      rv=found->first+fname;
    else
      rv=found->first+string(fname,found->second.length(),string::npos);
    if(toHost(rv)!=fname)
      fail("NameSpace::toTarget %s -> %s maps back to %s\n",
	   fname.c_str(),rv.c_str(),toHost(rv).c_str());
    return rv;
  }
  const string NameSpace::toHost(const string &fname) const{
#if (defined DEBUG_MOUNTS)
    cout << "toHost called with " << fname << endl;
#endif
    I(fname[0]=='/');
    string::size_type fnameLen=fname.length();
    string lookStr=fname;
    string::size_type lookLen=fnameLen;
    while(true){
      Mounts::const_iterator it=mounts.lower_bound(lookStr);
      if(it==mounts.end())
	fail("Namespace::toHost: Cannot find the mapping for %s\n",fname.c_str());
      const string &targFound=it->first;
      const string &hostFound=it->second;
#if (defined DEBUG_MOUNTS)
      cout << "Checking " << targFound << " -> " << hostFound <<
	" for " << lookStr << endl;
#endif
      string::size_type foundLen=targFound.length();
      string::size_type bothLen=min(foundLen,lookLen);
      string::size_type matchPos=0;
      string::size_type checkPos=0;
      while(checkPos<bothLen){
	char c=targFound[checkPos];
	// Char differs in two strings, the longest path match between
	// the two is in matchPos already, just exit the comparison loop
	if(c!=lookStr[checkPos])
	  break;
	// Same char and it's a slash, the new longest path match is
	// what we found so far, including the slash
	if(c=='/')
	  matchPos=checkPos+1;
	// Strings are still the same, keep comparing them 
	checkPos++;
      }
      // If we ran out of targFound characters without finding a difference,
      // see if the entire targFound path is a match
      if(checkPos==foundLen){
	// We have a match 1) if we are also out of lookStr chars 
	// (in which case targFound and lookStr are exactly the same),
	// 2) if targFound ends with '/' (in which case matchPos is
	// already equal to checkPos), or 3) if the next char in lookStr
	// is '/' (in which case targFound is part of the lookStr path)
	if((checkPos==lookLen)||(lookStr[checkPos]=='/'))
	  matchPos=checkPos;
      }
      // If we have a full match, we can complete the mapping and return
      if(matchPos==foundLen){
	bool addSlash=
	  (targFound.at(targFound.length()-1)=='/')&&
	  (hostFound.at(hostFound.length()-1)!='/');
#if (defined DEBUG_MOUNTS)
	cout << "toHost returns " << hostFound+(addSlash?"/":"")+string(fname,targFound.length()) << endl;
#endif
	return hostFound+(addSlash?"/":"")+string(fname,targFound.length());
      }
      // We have just a partial match, so we truncate lookStr to only
      // contain that longest path match and look for the mounting again
      lookStr.erase(matchPos);
    }
  }
  FileSys::FileSys(NameSpace *ns, const string &cwd)
    : GCObject()
    , nameSpace(ns)
    , cwd(cwd){
  }
  FileSys::FileSys(const FileSys &src, bool newNameSpace)
    : GCObject()
    , nameSpace(newNameSpace?(NameSpace *)(new NameSpace(*(src.nameSpace))):(NameSpace *)(src.nameSpace))
    , cwd(src.cwd){
  }
  void FileSys::setCwd(const string &newCwd){
    cwd=normalize(cwd,newCwd);
  }
  const string NameSpace::normalize(const string &base, const string &fname){
    string rv(fname+"/");
    if(string(fname,0,1)!="/")
      rv=base+"/"+rv;
    while(true){
      string::size_type pos=rv.find("//");
      if(pos==string::npos)
        break;
      rv.erase(pos,1);
    }
    while(true){
      string::size_type pos=rv.find("/./");
      if(pos==string::npos)
        break;
      rv.erase(pos,2);
    }
    while(true){
      string::size_type epos=rv.find("/../");
      if(epos==string::npos)
        break;
      string::size_type bpos=(epos==0)?0:rv.rfind('/',epos-1);
      if(bpos==string::npos)
        fail("Found /../ with no /dir before it\n"); 
      rv.erase(bpos,epos-bpos+3);
    }
    if(rv!="/")
      rv.erase(rv.length()-1);
#if (defined DEBUG_MOUNTS)
    cout << "NameSpace::normalize (" << base << "," << fname <<") to " << rv << endl;
#endif
    return rv;
  }
  const string FileSys::toHost(const string &fname) const{
    return nameSpace->toHost(normalize(cwd,fname));
  }

}
