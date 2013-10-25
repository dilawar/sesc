#ifndef MCOFF_H_
#define MCOFF_H_
/* structure definitions needed to read MIPS coff files */

struct filehdr {
    uint16_t f_magic;
    uint16_t f_nscns;
	 int32_t f_timdat;
	 int32_t f_symptr;
	 int32_t f_nsyms;
    uint16_t f_opthdr;
    uint16_t f_flags;
};

struct scnhdr {
    char s_name[8];
	 int32_t s_paddr;
	 int32_t s_vaddr;
	 int32_t s_size;
	 int32_t s_scnptr;
	 int32_t s_relptr;
	 int32_t s_lnnoptr;
    uint16_t s_nreloc;
    uint16_t s_nlnno;
	 int32_t s_flags;
};

struct aouthdr {
    short magic;
    short vstamp;
	 int32_t tsize;
	 int32_t dsize;
	 int32_t bsize;
	 int32_t entry;
	 int32_t text_start;
	 int32_t data_start;
	 int32_t bss_start;
	 int32_t gprmask;
	 int32_t cprmask[4];
	 int32_t gp_value;
};

typedef struct symhdr_t {
    short magic;
    short vstamp;
	 int32_t ilineMax;
	 int32_t cbLine;
	 int32_t cbLineOffset;
	 int32_t idnMax;
	 int32_t cbDnOffset;
	 int32_t ipdMax;
	 int32_t cbPdOffset;
	 int32_t isymMax;
	 int32_t cbSymOffset;
	 int32_t ioptMax;
	 int32_t cbOptOffset;
	 int32_t iauxMax;
	 int32_t cbAuxOffset;
	 int32_t issMax;
	 int32_t cbSsOffset;
	 int32_t issExtMax;
	 int32_t cbSsExtOffset;
	 int32_t ifdMax;
	 int32_t cbFdOffset;
	 int32_t crfd;
	 int32_t cbRfdOffset;
	 int32_t iextMax;
	 int32_t cbExtOffset;
} HDRR;

#define magicSym 0x7009

typedef struct fdr {
	 uint32_t adr;
	 int32_t rss;
	 int32_t issBase;
	 int32_t cbSs;
	 int32_t isymBase;
	 int32_t csym;
	 int32_t ilineBase;
	 int32_t cline;
	 int32_t ioptBase;
	 int32_t copt;
    uint16_t ipdFirst;
    uint16_t cpd;
	 int32_t iauxBase;
	 int32_t caux;
	 int32_t rfdBase;
	 int32_t crfd;
    unsigned lang :5;
    unsigned fMerge :1;
    unsigned fReadin :1;
    unsigned fBigendian :1;
    unsigned reserved :24;
	 int32_t cbLineOffset;
	 int32_t cbLine;
} FDR;

typedef struct pdr {
	 uint32_t adr;
	 int32_t isym;
	 int32_t iline;
	 int32_t regmask;
	 int32_t regoffset;
	 int32_t iopt;
	 int32_t fregmask;
	 int32_t fregoffset;
	 int32_t frameoffset;
    short framereg;
    short pcreg;
	 int32_t lnLow;
	 int32_t lnHigh;
	 int32_t cbLineOffset;
} PDR;

typedef struct {
	 int32_t iss;
	 int32_t value;
    unsigned st :6;
    unsigned sc :5;
    unsigned reserved :1;
    unsigned index :20;
} SYMR;

typedef struct {
    short reserved;
    short ifd;
    SYMR asym;
} EXTR;

#ifndef R_SN_BSS
#define R_SN_TEXT 1
#define R_SN_RDATA 2
#define R_SN_DATA 3
#define R_SN_SDATA 4
#define R_SN_SBSS 5
#define R_SN_BSS 6

#define STYP_TEXT 0x20
#define STYP_RDATA 0x100
#define STYP_DATA 0x40
#define STYP_SDATA 0x200
#define STYP_SBSS 0x400
#define STYP_BSS 0x80

#define stNil           0
#define stGlobal        1
#define stStatic        2
#define stParam         3
#define stLocal         4
#define stLabel         5
#define stProc          6
#define stBlock         7
#define stEnd           8
#define stMember        9
#define stTypedef       10
#define stFile          11
#define stRegReloc	12
#define stForward	13
#define stStaticProc	14
#define stConstant	15

#endif
#endif
