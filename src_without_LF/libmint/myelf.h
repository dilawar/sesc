#ifndef MELF_H
#define MELF_H

#include <stdint.h>

#ifdef DARWIN
/* this structure is at the beginning of the ELF object file */
#define EI_NIDENT	16
typedef struct {
    uint8_t  e_ident[EI_NIDENT];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf32_Ehdr;

/* the type for statically linked executables */
#define ET_EXEC	2
#define ET_DYN	3

#define EI_DATA            5       /* Data encoding */
#define ELFDATA2MSB        2

#define EF_MIPS_PIC		0x00000002
#define EF_MIPS_CPIC		0x00000004
#define EF_MIPS_ARCH		0xf0000000
#define EF_MIPS_ARCH_1		0x00000000
#define EF_MIPS_ARCH_2		0x10000000
#define EF_MIPS_ARCH_3		0x20000000
#define EF_MIPS_ARCH_4		0x30000000

#define EM_MIPS	8

/* section header */
typedef struct {
	 uint32_t sh_name;
	 uint32_t sh_type;
	 uint32_t sh_flags;
	 uint32_t sh_addr;
	 uint32_t sh_offset;
	 uint32_t sh_size;
	 uint32_t sh_link;
	 uint32_t sh_info;
	 uint32_t sh_addralign;
	 uint32_t sh_entsize;
} Elf32_Shdr;

#define SHT_PROGBITS	1
#define SHT_STRTAB	3


typedef struct {
	 uint32_t st_name;
	 uint32_t st_value;
	 uint32_t st_size;
    uint8_t st_info;
    uint8_t st_other;
    uint16_t st_shndx;
} Elf32_Sym;

#define STB_LOCAL	0
#define STB_GLOBAL	1
#define STB_WEAK	2

#define STT_OBJECT	1
#define STT_FUNC	2
#define STT_SECTION	3
#define STT_FILE	4

#else
#include <elf.h>
#ifndef EF_MIPS_PIC
#define EF_MIPS_PIC         2 
#endif
#ifndef EF_MIPS_CPIC
#define EF_MIPS_CPIC        4 
#endif
#endif

#define SHT_MDEBUG	0x70000005

void elf_read_hdrs(char *objfile);
void elf_read_nmlist();


#endif
