/*
 * Routines for reading the object file header information.
 *
 * Copyright (C) 1993 by Jack E. Veenstra (veenstra@cs.rochester.edu)
 * 
 * This file is part of MINT, a MIPS code interpreter and event generator
 * for parallel programs.
 * 
 * MINT is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 * 
 * MINT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with MINT; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "icode.h"
#include "globals.h"
#include "coff.h"
#include "myelf.h"
#include "symtab.h"

#include "mendian.h"

extern FILE *Fobj;	/* file descriptor for object file */
struct filehdr Fhdr;

/* File_info contains information about the application needed for
 * mapping instruction addresses to (source file, line number) pairs.
 */
static struct file_info *File_info;

/* The number of entries in File_info */
static int32_t Numfiles;

/* Stringtab contains all the symbol names in the application program */
char *Stringtab;

/* Linetab contains source line number information about the application */
static char *Linetab;

/* Line_end is the address of the byte past the end of Linetab */
static uint8_t *Line_end;

/* Nmlist is an array of namelist entries for the "interesting" symbols in
 * the application object file. Each namelist entry is a structure
 * containing the value and type of the symbol and a pointer into the
 * Stringtab array for the symbol name.
 */
namelist_ptr Nmlist;
namelist_ptr Funclist;


/* Num_symbols is the number of symbols stored in Nmlist */
int32_t Num_symbols;
int32_t Num_funcs;

/* External procedure declarations */
void elf_read_nmlist();

/* Forward procedure declarations */
void read_nmlist();
void read_linenum();
int32_t nm_cmp_name(const void *v1, const void *v2);
int32_t nm_cmp_name_ptr(const void *v1, const void *v2);
int32_t nm_cmp_value_ptr(const void *v1, const void *v2);
int32_t readsrc(char *fname, int32_t lnum, int32_t numlines, char *buf, int32_t buflen);


int32_t noun_strcmp(const char *s1, const char *s2)
{
  /* A normal strcmp but ignores the underscores at the beginning of
     the of the string */

  while(*s1 == '_' && *s1 != '\0') { s1++; }
  while(*s2 == '_' && *s2 != '\0') { s2++; }

  if(strncmp(s1,"libc_",5) == 0)
    s1+=5;
  if(strncmp(s2,"libc_",5) == 0)
    s2+=5;

  return strcmp(s1, s2);
}

void
close_object()
{
  if (Fobj==0)
    return;

  fclose(Fobj);
  Fobj = 0;
}

void
read_hdrs(char *objfile)
{
  int32_t i, magic;
  struct aouthdr ahdr;
  struct scnhdr shdr;
  char ident[EI_NIDENT];

  close_object();
  Fobj = fopen(objfile, "r");
  if (Fobj == NULL) {
    perror(objfile);
    exit(1);
  }
        
  fseek(Fobj, 0, SEEK_SET);
  if (fread(ident, EI_NIDENT, 1, Fobj) < 1)
    fatal("read_hdrs: cannot read file \"%s\".\n", objfile);
  if (ident[0] == 0x7f && ident[1] == 'E' &&
      ident[2] == 'L' && ident[3] == 'F') {
    close_object();
    elf_read_hdrs(objfile);
    return;
  }

  fseek(Fobj, 0, SEEK_SET);
  if (fread(&Fhdr, sizeof(struct filehdr), 1, Fobj) < 1)
    fatal("read_hdrs: could not read file header\n");

  Endianfilehdr(&Fhdr);
	
#ifdef DEBUG_HEADERS
  dump_filehdr(&Fhdr);
#endif

  fseek(Fobj, sizeof(struct filehdr), SEEK_SET);
  if (fread(&ahdr, sizeof(struct aouthdr), 1, Fobj) < 1)
    fatal("read_hdrs: could not read optional header\n");

  Endianaouthdr(&ahdr);

#ifdef DEBUG_HEADERS
  dump_aouthdr(&ahdr);
#endif
  magic = ahdr.magic;
  if (magic != 0407 && magic != 0x410 && magic != 0413 && magic != 0411) {
    fprintf(stderr, "\n\"%s\" is not a MIPS COFF executable object file.\n",
	    objfile);
    fatal("read_hdrs: bad magic number (0%o)\n", ahdr.magic);
  }

  Data_start = ahdr.data_start;
  Data_size = ahdr.dsize;
  Bss_size = ahdr.bsize;
  Bss_start = ahdr.bss_start;
  Gp_value = ahdr.gp_value;
  Text_entry = ahdr.entry;

  /* Seek to the beginning of the first section header.
   * The file header comes first, followed by the optional header
   * (this is the aouthdr). The size of the aouthdr is given in
   * Fdhr.f_opthdr.
   */
  fseek(Fobj, sizeof(struct filehdr) + Fhdr.f_opthdr, SEEK_SET);

  /* loop through the section headers */
  for (i = 0; i < Fhdr.f_nscns; i++) {
    if (fread(&shdr, sizeof(struct scnhdr), 1, Fobj) < 1)
      fatal("read_hdrs: could not read section header %d\n", i);

    Endianscnhdr(&shdr);
		
    switch (shdr.s_flags) {
    case STYP_TEXT:
      Text_seek = shdr.s_scnptr;
      Text_start = shdr.s_vaddr;
      Text_size = shdr.s_size / 4;
      /* there is a null routine after the supposed end of text */
      Text_size += 10;
      Text_end = Text_start + Text_size * 4;
      /* create_text_reloc(shdr.s_relptr, shdr.s_nreloc); */
      break;
    case STYP_RDATA:
      /* The .rdata section is sometimes placed before the text
       * section instead of being contiguous with the .data section.
       */
      Rdata_start = shdr.s_vaddr;
      Rdata_size = shdr.s_size;
      Rdata_seek = shdr.s_scnptr;
      break;
    case STYP_DATA:
      Data_seek = shdr.s_scnptr;
      break;
    case STYP_SDATA:
      Sdata_seek = shdr.s_scnptr;
      break;
    case STYP_BSS:
      break;
    case STYP_SBSS:
      break;
    }
#ifdef DEBUG_HEADERS
    dump_scnhdr(&shdr);
#endif
  }
  read_linenum();
  read_nmlist();
}

/* Read the names from the symbol table of the application object file.
 * This uses the file header structure "Fhdr" that is initialized
 * in "read_hdrs()".
 */
void
read_nmlist()
{
    int32_t i, err, str_offset, ifd, ipd, len, size, nm_next, func_next, type;
    int32_t symnum, lastsym, addr, first, last, nextf, offset, offsetpd;
    char *fname;
    HDRR symhdr;
    FDR *fdr;
    SYMR *symr;
    EXTR *extr;
    PDR *pdr;
    
    /* seek to the beginning of the symbolic header */
    fseek(Fobj, Fhdr.f_symptr, SEEK_SET);
    if (fread(&symhdr, sizeof(HDRR), 1, Fobj) < 1)
        fatal("read_nmlist: could not read symbolic header\n");

    Endiansymhdr(&symhdr);
	
#ifdef DEBUG_HEADERS
    dump_symhdr(&symhdr);
#endif
    if (symhdr.magic != magicSym)
        fatal("read_nmlist: bad magic number (0x%x) in symbolic header\n",
              symhdr.magic);

    /* allocate space for the string table */
    len = symhdr.issMax + symhdr.issExtMax;
    Stringtab = (char *) malloc(len);

    /* read all the symbol names into memory */
    fseek(Fobj, symhdr.cbSsOffset, SEEK_SET);
    err = fread(Stringtab, len, 1, Fobj);
    if (err == -1) {
        perror("reading symbol table names");
        exit(1);
    }

    /* allocate space for the namelist pointers and entries */
    len = symhdr.isymMax + symhdr.iextMax;
    size = len * sizeof(namelist_t);
    Nmlist = (namelist_ptr) malloc(size);
    Funclist = (namelist_ptr) malloc(size);

    /* allocate space for the file descriptor entries */
    fdr = (FDR *) malloc(symhdr.ifdMax * sizeof(FDR));
    if (fdr == NULL)
        fatal("read_nmlist: cannot allocate 0x%x bytes for FDR entries.\n",
              symhdr.ifdMax * sizeof(FDR));

    /* read in the file descriptor entries */
    fseek(Fobj, symhdr.cbFdOffset, SEEK_SET);
    err = fread(fdr, sizeof(FDR), symhdr.ifdMax, Fobj);
    if (err == -1) {
        perror("reading file descriptor entries");
        exit(1);
    }
	{
		int32_t i;

		for(i=0;i<symhdr.ifdMax;i++)
			EndianFDR(&fdr[i]);
	}

    /* allocate space for the procedure descriptor entries */
    pdr = (PDR *) malloc(symhdr.ipdMax * sizeof(PDR));
    if (pdr == NULL)
        fatal("read_nmlist: cannot allocate 0x%x bytes for PDR entries.\n",
              symhdr.ipdMax * sizeof(PDR));

    /* read in the procedure descriptor entries */
    fseek(Fobj, symhdr.cbPdOffset, SEEK_SET);
    err = fread(pdr, sizeof(PDR), symhdr.ipdMax, Fobj);
    if (err == -1) {
        perror("reading procedure descriptor entries");
        exit(1);
    }
	
    {
      int32_t i;
      
      for(i=0;i<symhdr.ipdMax;i++)
	EndianPDR(&pdr[i]);
    }

    /* allocate space for the local symbol entries */
    symr = (SYMR *) malloc(symhdr.isymMax * sizeof(SYMR));
    if (symr == NULL)
        fatal("read_nmlist: cannot allocate 0x%x bytes for SYMR entries.\n",
              symhdr.isymMax * sizeof(SYMR));

    /* read in the local symbol entries */
    fseek(Fobj, symhdr.cbSymOffset, SEEK_SET);
    err = fread(symr, sizeof(SYMR), symhdr.isymMax, Fobj);
    if (err == -1) {
        perror("reading local symbol entries");
        exit(1);
    }

    {
      int32_t i;
      
      for(i=0;i<symhdr.isymMax;i++){
	EndianSYMR(&symr[i]);
      }
    }
    
    Numfiles = symhdr.ipdMax;
    File_info = (struct file_info *) calloc(Numfiles, sizeof(struct file_info));

    nextf = 0;
	
    for (ifd = 0; ifd < symhdr.ifdMax; ifd++) {
        symnum = fdr[ifd].isymBase;
        lastsym = symnum + fdr[ifd].csym;
		
        if (symr[symnum].st != stFile)
            fprintf(stderr, "read_nmlist: cannot find source file name\n");
        str_offset = fdr[ifd].issBase + symr[symnum].iss;
        fname = &Stringtab[str_offset];
		
        symnum++;
        if (symnum >= lastsym)
            continue;
		
        type = symr[symnum].st;
        while (type != stProc && type != stStaticProc) {
            symnum++;
            if (symnum >= lastsym)
                break;
            type = symr[symnum].st;
        }
        if (symnum >= lastsym)
            continue;

        addr = symr[symnum].value;
        first = fdr[ifd].ipdFirst;
        last = first + fdr[ifd].cpd;
        offset = fdr[ifd].cbLineOffset;
        for (ipd = first; ipd < last; ipd++, nextf++) {
            File_info[nextf].fname = fname;
            File_info[nextf].addr = addr + pdr[ipd].adr;
            File_info[nextf].linelow = pdr[ipd].lnLow;
            offsetpd = offset + pdr[ipd].cbLineOffset;
            File_info[nextf].lptr = (uint8_t *) &Linetab[offsetpd];
        }
    }

    ifd = 0;
    nm_next = 0;
    func_next = 0;
    for (i = 0; i < symhdr.isymMax; i++) {
        if (i >= fdr[ifd].isymBase + fdr[ifd].csym)
            ifd++;
        type = symr[i].st;
	if( type == stProc ) {
	  str_offset = fdr[ifd].issBase + symr[i].iss;
	  Funclist[func_next].n_value = symr[i].value;
	  Funclist[func_next].n_type = type;
	  Funclist[func_next].n_name = &Stringtab[str_offset];

/*	  printf("%08x %2d %s\n",
		 Funclist[func_next].n_value, Funclist[func_next].n_type, Funclist[func_next].n_name);
*/

	  func_next++;
	}else if (type == stGlobal || type == stStatic || type == stProc ||
            type == stStaticProc) {
            str_offset = fdr[ifd].issBase + symr[i].iss;
            Nmlist[nm_next].n_value = symr[i].value;
            Nmlist[nm_next].n_type = type;
            Nmlist[nm_next].n_name = &Stringtab[str_offset];
            nm_next++;
        }
    }
    free(symr);
    free(fdr);

    /* allocate space for the external symbol entries */
    extr = (EXTR *) malloc(symhdr.iextMax * sizeof(EXTR));
    if (extr == NULL)
        fatal("read_symtab: cannot allocate 0x%x bytes for external symbols.\n",
              symhdr.iextMax * sizeof(EXTR));
    fseek(Fobj, symhdr.cbExtOffset, SEEK_SET);
    err = fread(extr, sizeof(EXTR), symhdr.iextMax, Fobj);
    if (err == -1) {
        perror("reading external symbol entries");
        exit(1);
    }

    {
      int32_t i;
      
      for(i=0;i<symhdr.iextMax;i++)
	EndianEXTR(&extr[i]);
    }

    for (i = 0; i < symhdr.iextMax; i++) {
        type = extr[i].asym.st;
	if (extr[i].asym.index == 0xfffff
            || type == stGlobal || type == stStatic
            || type == stProc || type == stStaticProc)
        {
            str_offset = symhdr.issMax + extr[i].asym.iss;
            Nmlist[nm_next].n_value = extr[i].asym.value;
            Nmlist[nm_next].n_type = extr[i].asym.st;
            Nmlist[nm_next].n_name = &Stringtab[str_offset];
            nm_next++;
        }
    }
    free(extr);

    Num_symbols = nm_next;
    Num_funcs   = func_next;

    /* sort namelist into alphabetical order for fast searching */
    qsort((void *) Nmlist, Num_symbols, sizeof(namelist_t), nm_cmp_name);
}

int32_t isFirstInFuncCall(uint32_t addr)
{
  int32_t i;
  
  for (i = 0; i < Num_funcs; i++)
    if( Funclist[i].n_value == addr )
      return 1;

  return 0;
}

/* This function uses the native machine's "nm" to retrieve the list of
 * function names and addresses used in the MINT simulator. This is useful
 * for debugging and for meaningful error messages.
 */

int32_t nm_cmp_name(const void *v1, const void *v2)
{
  const namelist_t *p1 = (const namelist_t *)v1;
  const namelist_t *p2 = (const namelist_t *)v2;
  
  return noun_strcmp(p1->n_name, p2->n_name);
}

int32_t nm_cmp_name_ptr(const void *v1, const void *v2)
{
  const namelist_t **p1 = (const namelist_t **)v1;
  const namelist_t **p2 = (const namelist_t **)v2;

  return noun_strcmp((*p1)->n_name, (*p2)->n_name);
}

int32_t nm_cmp_value_ptr(const void *v1, const void *v2)
{
  const namelist_t **p1 = (const namelist_t **)v1;
  const namelist_t **p2 = (const namelist_t **)v2;

  return (*p1)->n_value - (*p2)->n_value;
}

/* This uses the application symbol table namelist in "Nmlist" created by
 * read_nmlist() to fill in the values of symbols passed in the array
 * pointed to by "pnlist".
 */
int32_t namelist(char *objname, namelist_ptr pnlist)
{
  int32_t count, next, found, symbol;
  int32_t result=0;
  namelist_ptr psym, *base, pnmlist;

  /* initialize the return values to 0, and count the number of elements */
  count = 0;
  for (psym = pnlist; psym->n_name && *psym->n_name; psym++) {
    psym->n_type = 0;
    psym->n_value = 0;
    count++;
  }
  found = 0;

  /* sort the elements using an external array of pointers */
  base = (namelist_ptr *) malloc(count * sizeof(namelist_ptr));
  next = 0;
  for (psym = pnlist; psym->n_name && *psym->n_name; psym++) {
    base[next] = psym;
    next++;
  }
  qsort((char *) base, count, sizeof(namelist_ptr), nm_cmp_name_ptr);

  /* base now points to the base of the sorted array of pointers to
   * namelist structures.
   *
   * Nmlist is an array of namelist structures (not pointers) and is
   * already sorted.
   */

  pnmlist = &Nmlist[0];
  symbol = 0;
  for (next = 0; next < count; /* "next" incremented inside loop */ ) {
    psym = base[next];

    /* Advance the pnmlist pointer if the name is less than
     * the psym name. Save the result of the comparison.
     */
    while (symbol < Num_symbols &&
	   (result = noun_strcmp(pnmlist->n_name, psym->n_name)) < 0) {
      pnmlist++;
      symbol++;
    }
    if (symbol == Num_symbols)
      break;

    /* If the names match, then fill in the fields. */
    if (result == 0) {
      /* There was a match. Find all matches. */
      do {
	psym->n_type = pnmlist->n_type;
	psym->n_value = pnmlist->n_value;
	found++;
	if (++next >= count)
	  break;
	psym = base[next];
      } while (noun_strcmp(pnmlist->n_name, psym->n_name) == 0);

      /* at this point, the psym name is greater than the pnmlist
       * name, so advance the pnmlist pointer
       */
      pnmlist++;
      symbol++;
    } else {
      /* There was no match. Advance the psym pointer */
      next++;
    }
  }
  free(base);
  return found;
}

/* Read the line number info from the application program.
 */
void
read_linenum()
{
    int32_t err;
    HDRR symhdr;
    
    /* seek to the beginning of the symbolic header */
    fseek(Fobj, Fhdr.f_symptr, SEEK_SET);
    if (fread(&symhdr, sizeof(HDRR), 1, Fobj) < 1)
        fatal("read_linenum: could not read symbolic header\n");

	Endiansymhdr(&symhdr);
	
#ifdef DEBUG_HEADERS
    dump_symhdr(&symhdr);
#endif
    if (symhdr.magic != magicSym)
        fatal("read_linenum: bad magic number (0x%x) in symbolic header\n",
              symhdr.magic);

    /* allocate space for the line number entries */
    Linetab = (char *) malloc(symhdr.cbLine);

    /* read all the line number entries into memory */
    fseek(Fobj, symhdr.cbLineOffset, SEEK_SET);
    err = fread(Linetab, symhdr.cbLine, 1, Fobj);
    if (err == -1) {
        perror("reading line number entries");
        exit(1);
    }
    Line_end = (uint8_t *) Linetab + symhdr.cbLine;
}

/* addr2src() maps an instruction address in the application program
 * to the filename and line number in the source.
 *
 * Parameters:
 *   iaddr	the instruction address
 *   fname	if not NULL, the address of a character pointer that
 * 		will be set to point to the file name containing iaddr
 *   linenum	if not NULL, the address of an integer that will be
 * 		set to the source line number for the given iaddr
 *   buf	if not NULL, a buffer that will contain the source
 * 		line from the application file
 *   buflen	the maximum length of buf
 *
 * If successful at finding a mapping, addr2src() returns 1 and sets
 * the non-null parameters fname, linenum, and buf. If not successful
 * addr2src() returns 0 and does not modify any of the parameters.
 * If a mapping (file:line) is found, but the source file does not exist
 * then 1 is returned but "buf" is not modified.
 */
int32_t addr2src(int32_t iaddr, const char **fname, int32_t *linenum, char *buf, int32_t buflen)
{
    int32_t i, lnum, delta, count;
	 int32_t addr;
    int32_t index = 0;
    uint8_t *ptr;
    unsigned const char *last;

    if (File_info == NULL || iaddr < File_info[0].addr)
        return 0;
    for (i = 0; i < Numfiles; i++) {
        if (File_info[i].addr == 0)
            continue;

        /* save the index of the last non-zero address */
        if (iaddr >= File_info[i].addr)
            index = i;
        else
            break;
    }
    if (i < Numfiles)
        last = File_info[i].lptr;
    else
        last = Line_end;

    /* try to find the line number for this instruction address */
    lnum = File_info[index].linelow;
    addr = File_info[index].addr;
    for (ptr = File_info[index].lptr; ptr < last; ptr++) {
        delta = *ptr >> 4;
        count = *ptr & 0xf;
        count++;
        if (delta > 8) {
            delta -= 16;
        } else if (delta == 8) {
            delta = *++ptr << 8;
            delta |= *++ptr;
            if (delta & 0x8000)
                delta -= 65536;
        }
        lnum += delta;
        if (iaddr >= addr && iaddr < addr + count * 4)
            break;
        addr += count * 4;
    }
    /* follow the convention of "dis" output */
    if (ptr >= last) {
        lnum = File_info[index].linelow;
    }
    if (fname)
        *fname = File_info[index].fname;
    if (linenum)
        *linenum = lnum;
    readsrc(File_info[index].fname, lnum, 1, buf, buflen);
    return 1;
}

#define BUFSIZE 4096
static char Buffer[BUFSIZE];

/* This routine reads the "numlines" lines starting at line "lnum"
 * from the source file into a buffer. If the file is less than "lnum"
 * lines long, then "buf" is unmodified. If modified, "buf" is terminated
 * with a zero byte. The number of bytes read into "buf" is returned.
 */
int32_t readsrc(char *fname, int32_t lnum, int32_t numlines, char *buf, int32_t buflen)
{
    FILE *fd;
    char path[300], *ptr;
    int32_t lines, len, spaceleft;

    if (buf == NULL || buflen <= 0 || numlines <= 0)
        return 0;

    /* if the file name is not an absolute path, then prefix the object path */
    if (fname[0] != '/') {
        /* prefix the object pathname, if any */
        strcpy(path, Objname);
        len = strlen(path);

        /* remove the filename component from the path */
        for (ptr = path + len - 1; ptr > path; ptr--)
            if (*ptr == '/')
                break;

        /* if we found a '/' then prefix the object path to the source path */
        if (*ptr == '/') {
            *++ptr = 0;
            strcat(path, fname);
            fd = fopen(path, "r");
        } else {
            fd = fopen(fname, "r");
        }
    } else
        fd = fopen(fname, "r");
    
    if (fd == NULL) {
        return 0;
    }

    /* read the file up to and including line "lnum" */
    lines = 0;
    while (fgets(Buffer, BUFSIZE, fd)) {
        lines++;
        if (lines == lnum)
            break;
    }
    spaceleft = buflen;
    if (lines == lnum) {
        len = strlen(Buffer);
        strncpy(buf, Buffer, buflen);
        if (len < buflen && numlines > 1) {
            ptr = buf + len;
            *ptr = 0;
            spaceleft = buflen - len;
            while (fgets(Buffer, BUFSIZE, fd)) {
                lines++;
                if (lines == lnum + numlines)
                    break;
                len = strlen(Buffer);
                strncpy(ptr, Buffer, spaceleft);
                if (spaceleft <= len) {
                    *(ptr + spaceleft - 1) = 0;
                    break;
                }
                spaceleft -= len;
                ptr = ptr + len;
                *ptr = 0;
            }
        } else {
            spaceleft = 0;
            buf[buflen - 1] = 0;
        }
    }

    fclose(fd); /* close_object(); */
    return buflen - spaceleft;
}

void
print_nmlist()
{
    int32_t i;

    for (i = 0; i < Num_symbols; i++)
        printf("0x%x %2d %s\n",Nmlist[i].n_value, Nmlist[i].n_type, Nmlist[i].n_name);
}
