
/* This file tries to handle the translation from big endian to
 *  little endian
 */

#include "mendian.h"
#include <stdio.h>
#include "icode.h"
#include "globals.h"


void EndianElf32_Ehdr(Elf32_Ehdr *Ehdr)
{
  Ehdr->e_type = SWAP_SHORT((unsigned)Ehdr->e_type);
  Ehdr->e_machine = SWAP_SHORT((unsigned)Ehdr->e_machine);
  Ehdr->e_version = SWAP_WORD((unsigned)Ehdr->e_version);
  Ehdr->e_entry = SWAP_WORD((unsigned)Ehdr->e_entry);
  Ehdr->e_phoff = SWAP_WORD((unsigned)Ehdr->e_phoff);
  Ehdr->e_shoff = SWAP_WORD((unsigned)Ehdr->e_shoff);
  Ehdr->e_flags = SWAP_WORD((unsigned)Ehdr->e_flags);
  Ehdr->e_ehsize = SWAP_SHORT((unsigned)Ehdr->e_ehsize);
  Ehdr->e_phentsize = SWAP_SHORT((unsigned)Ehdr->e_phentsize);
  Ehdr->e_phnum = SWAP_SHORT((unsigned)Ehdr->e_phnum);
  Ehdr->e_shentsize = SWAP_SHORT((unsigned)Ehdr->e_shentsize);
  Ehdr->e_shnum = SWAP_SHORT((unsigned)Ehdr->e_shnum);
  Ehdr->e_shstrndx = SWAP_SHORT((unsigned)Ehdr->e_shstrndx);
	
}
	
void EndianElf32_Shdr(Elf32_Shdr *shdr)
{
  shdr->sh_name = SWAP_WORD(shdr->sh_name);
  shdr->sh_type = SWAP_WORD(shdr->sh_type);
  shdr->sh_flags = SWAP_WORD(shdr->sh_flags);
  shdr->sh_addr = SWAP_WORD(shdr->sh_addr);
  shdr->sh_offset = SWAP_WORD(shdr->sh_offset);
  shdr->sh_size = SWAP_WORD(shdr->sh_size);
  shdr->sh_link = SWAP_WORD(shdr->sh_link);
  shdr->sh_info = SWAP_WORD(shdr->sh_info);
  shdr->sh_addralign = SWAP_WORD(shdr->sh_addralign);
  shdr->sh_entsize = SWAP_WORD(shdr->sh_entsize);
}

void EndianElf32_Sym(Elf32_Sym *sym)
{
  sym->st_name  = SWAP_WORD(sym->st_name);
  sym->st_value = SWAP_WORD(sym->st_value);
  sym->st_size  = SWAP_WORD(sym->st_size);
  sym->st_shndx = SWAP_SHORT(sym->st_shndx);
}

void Endianaouthdr(struct aouthdr *h)
{
  h->magic = SWAP_SHORT(h->magic);
  h->vstamp = SWAP_SHORT(h->vstamp);
  h->tsize = SWAP_WORD((unsigned)h->tsize);
  h->dsize = SWAP_WORD((unsigned)h->dsize);
  h->bsize = SWAP_WORD((unsigned)h->bsize);
  h->entry = SWAP_WORD((unsigned)h->entry);
  h->text_start = SWAP_WORD((unsigned)h->text_start);
  h->data_start = SWAP_WORD((unsigned)h->data_start);
  h->bss_start = SWAP_WORD((unsigned)h->bss_start);
  h->gprmask = SWAP_WORD((unsigned)h->gprmask);
  h->cprmask[0] = SWAP_WORD((unsigned)h->cprmask[0]);
  h->cprmask[1] = SWAP_WORD((unsigned)h->cprmask[1]);
  h->cprmask[2] = SWAP_WORD((unsigned)h->cprmask[2]);
  h->cprmask[3] = SWAP_WORD((unsigned)h->cprmask[3]);
  h->gp_value = SWAP_WORD((unsigned)h->gp_value);

}

void Endiansymhdr(HDRR *h)
{
  h->magic = SWAP_SHORT(h->magic);
  h->vstamp = SWAP_SHORT(h->vstamp);
	
  h->ilineMax = SWAP_WORD((unsigned)h->ilineMax);
  h->cbLine = SWAP_WORD((unsigned)h->cbLine);
  h->cbLineOffset = SWAP_WORD((unsigned)h->cbLineOffset);
  h->idnMax = SWAP_WORD((unsigned)h->idnMax);
  h->cbDnOffset = SWAP_WORD((unsigned)h->cbDnOffset);
  h->ipdMax = SWAP_WORD((unsigned)h->ipdMax);
  h->cbPdOffset = SWAP_WORD((unsigned)h->cbPdOffset);
  h->isymMax = SWAP_WORD((unsigned)h->isymMax);
  h->cbSymOffset = SWAP_WORD((unsigned)h->cbSymOffset);
  h->ioptMax = SWAP_WORD((unsigned)h->ioptMax);
  h->cbOptOffset = SWAP_WORD((unsigned)h->cbOptOffset);
  h->iauxMax = SWAP_WORD((unsigned)h->iauxMax);
  h->cbAuxOffset = SWAP_WORD((unsigned)h->cbAuxOffset);
  h->issMax = SWAP_WORD((unsigned)h->issMax);
  h->cbSsOffset = SWAP_WORD((unsigned)h->cbSsOffset);
  h->issExtMax = SWAP_WORD((unsigned)h->issExtMax);
  h->cbSsExtOffset = SWAP_WORD((unsigned)h->cbSsExtOffset);
  h->ifdMax = SWAP_WORD((unsigned)h->ifdMax);
  h->cbFdOffset = SWAP_WORD((unsigned)h->cbFdOffset);
  h->crfd = SWAP_WORD((unsigned)h->crfd);
  h->cbRfdOffset = SWAP_WORD((unsigned)h->cbRfdOffset);
  h->iextMax = SWAP_WORD((unsigned)h->iextMax);
  h->cbExtOffset = SWAP_WORD((unsigned)h->cbExtOffset);
}

void Endianfilehdr(struct filehdr *h)
{
  h->f_magic = SWAP_SHORT(h->f_magic);
  h->f_nscns = SWAP_SHORT(h->f_nscns);
  h->f_timdat = SWAP_WORD((unsigned)h->f_timdat);
  h->f_symptr = SWAP_WORD((unsigned)h->f_symptr);
  h->f_nsyms = SWAP_WORD((unsigned)h->f_nsyms);
  h->f_opthdr = SWAP_SHORT(h->f_opthdr);
  h->f_flags = SWAP_SHORT(h->f_flags);
}

void Endianscnhdr(struct scnhdr *h)
{
  h->s_paddr = SWAP_WORD((unsigned)h->s_paddr);
  h->s_vaddr = SWAP_WORD((unsigned)h->s_vaddr);
  h->s_size = SWAP_WORD((unsigned)h->s_size);
  h->s_scnptr = SWAP_WORD((unsigned)h->s_scnptr);
  h->s_relptr = SWAP_WORD((unsigned)h->s_relptr);
  h->s_lnnoptr = SWAP_WORD((unsigned)h->s_lnnoptr);
	
  h->s_nreloc = SWAP_SHORT(h->s_nreloc);
  h->s_nlnno = SWAP_SHORT(h->s_nlnno);
	
  h->s_flags = SWAP_WORD((unsigned)h->s_flags);

}


void EndianFDR(FDR *h)
{
  h->adr = SWAP_WORD(h->adr);
  h->rss = SWAP_WORD((unsigned)h->rss);
  h->issBase = SWAP_WORD((unsigned)h->issBase);
  h->cbSs = SWAP_WORD((unsigned)h->cbSs);
  h->isymBase = SWAP_WORD((unsigned)h->isymBase);
  h->csym = SWAP_WORD((unsigned)h->csym);
  h->ilineBase = SWAP_WORD((unsigned)h->ilineBase);
  h->cline = SWAP_WORD((unsigned)h->cline);
  h->ioptBase = SWAP_WORD((unsigned)h->ioptBase);
  h->copt = SWAP_WORD((unsigned)h->copt);
	
  h->ipdFirst = SWAP_SHORT(h->ipdFirst);
  h->cpd = SWAP_SHORT(h->cpd);
	
  h->iauxBase = SWAP_WORD((unsigned)h->iauxBase);
  h->caux = SWAP_WORD((unsigned)h->caux);
  h->rfdBase = SWAP_WORD((unsigned)h->rfdBase);
  h->crfd = SWAP_WORD((unsigned)h->crfd);
	
#ifdef LENDIAN
  h->fBigendian = (h->reserved & 0x1) != 0;
  h->fReadin    = (h->reserved & 0x2) != 0;
  h->fMerge     = (h->reserved & 0x4) != 0;
  h->lang       = ((h->reserved >> 3) & 0x1F );
  h->reserved   = 0; /* May be a BUG */
#endif

  h->cbLineOffset = SWAP_WORD(h->cbLineOffset);
  h->cbLine = SWAP_WORD(h->cbLine);
	

}

void EndianPDR(PDR *h)
{
  h->adr = SWAP_WORD((unsigned)h->adr);
  h->isym = SWAP_WORD((unsigned)h->isym);
  h->iline = SWAP_WORD((unsigned)h->iline);
  h->regmask = SWAP_WORD((unsigned)h->regmask);
  h->regoffset = SWAP_WORD((unsigned)h->regoffset);
  h->iopt = SWAP_WORD((unsigned)h->iopt);
  h->fregmask = SWAP_WORD((unsigned)h->fregmask);
  h->fregoffset = SWAP_WORD((unsigned)h->fregoffset);
  h->frameoffset = SWAP_WORD((unsigned)h->frameoffset);
	
  h->framereg = SWAP_SHORT(h->framereg);
  h->pcreg = SWAP_SHORT(h->pcreg);
	
  h->lnLow = SWAP_WORD((unsigned)h->lnLow);
  h->lnHigh = SWAP_WORD((unsigned)h->lnHigh);
  h->cbLineOffset = SWAP_WORD((unsigned)h->cbLineOffset);
}

void EndianSYMR(SYMR *h)
{
#ifdef LENDIAN
  uint32_t swaped;
	
  h->iss = SWAP_WORD((unsigned)h->iss);
  h->value = SWAP_WORD((unsigned)h->value);

  /* I expect that I'll never will understand why I did that.
     I wish that it works!
	 */

  swaped = ((h->st&0x3F) << 26) | ((h->sc&0x1F) << 21) | ((h->reserved&0x1) << 20) | h->index;
	
  h->index    = swaped >> 12;
  h->reserved = (swaped >> 11) & 0x1;
  h->sc = (swaped >> 6) & 0x1F;
  h->st = (swaped>>28) & 0x3F;
#endif
}

void EndianEXTR(EXTR *h)
{
  h->reserved = SWAP_SHORT((unsigned)h->reserved);
  h->ifd = SWAP_SHORT((unsigned)h->ifd);

  EndianSYMR(&h->asym);
}
