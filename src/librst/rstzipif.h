/*
* ========== Copyright Header Begin ==========================================
* 
* OpenSPARC T1 Processor File: rstzipif.h
* Copyright (c) 2006 Sun Microsystems, Inc.  All Rights Reserved.
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES.
* 
* The above named program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public
* License version 2 as published by the Free Software Foundation.
* 
* The above named program is distributed in the hope that it will be 
* useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* General Public License for more details.
* 
* You should have received a copy of the GNU General Public
* License along with this work; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
* 
* ========== Copyright Header End ============================================
*/

// File: rstzipif.H
//
// Written by: Kelvin Fong
//
// Send complaints to: klf@eng.sun.com

#ifndef _RSTZIPIF_H
#define _RSTZIPIF_H

enum {
  RSTZIP_MAJOR_VERSION = 2,
  RSTZIP_MINOR_VERSION = 1,

  RSTZIP_MAXCPUS = 64
};

#ifdef __cplusplus

#include <stdio.h>
#include <string.h>

#include "Rstzipi.h"
#include "cpuid.h"
#include "buffer.h"
#include "header.h"
#include "footer.h"

#include "mendian.h"

typedef struct {
  int32_t cpu;
  uint8_t count8;
  uint16_t count16;
  uint32_t count32;
} CpuCount;

class RstzipIF {
public:
  RstzipIF() {
    infp = NULL;
    outfp = NULL;
    dbgfp = NULL;
    gzfile = NULL;

    for (int32_t i = 0; i < RSTZIP_MAXCPUS; i++) {
      rstzip[i] = NULL;
      rstunzip[i] = NULL;
    }

    unirst = (RstSplit*) calloc(RSTZIP_MAXCPUS, sizeof(RstSplit));

    curcpu = 0;

    header = new RstzipHeader;
    footer = new RstzipFooter;
    stats = 0;
    ncpus = 0;
    decompress_done = 0;
    buffersize = 0;
  }

  ~RstzipIF() {
    free(unirst);
    delete header;
    delete footer;
  }

  int32_t openRstzip(const char* outfile, int32_t bufsize, int32_t gzip, int32_t stat, int32_t numcpus) {
    // Get numcpu and initialize the individual RST buffers for each cpu.
    char text[80] = { 0 };    

    rstbuf = new RstzipBuffer(bufsize);
    cpucount = (CpuCount*) calloc(bufsize, sizeof(CpuCount));;
    zbuffer = (uint8_t*) calloc(bufsize, sizeof(rstf_unionT));

    int32_t retval = openFiles(NULL, outfile, gzip);

    if (retval == 1) {
      if (numcpus == 1) {
        ncpus = 1;
        rstzip[0] = new Rstzipi;
      }

      // Write the file header.
      sprintf(text, "Rstzip2 version: %d.%02d.\n", RSTZIP_MAJOR_VERSION, RSTZIP_MINOR_VERSION);
      strcat(header->getHeader(), text);
      header->setNumcpus(ncpus);
      zfwrite(header->getHeader(), header->getMaxHeaderSize());

      stats = stat;
      buffersize = bufsize;
    }

    return retval;
  }

  void closeRstzip() {
    // Write a zero record MPchunck to indicate the end of the compressed file.
    while (compress(NULL, 0) != 0) {
      //fprintf(stderr, "Hello!\n");
    }
    //compress(NULL, 0);

    calcFooter(footer);
    writeFooter();

    if (stats == 1) {
      footer->fprint(stderr);
    }

    closeFiles();

    for (int32_t i = 0; i < RSTZIP_MAXCPUS; i++) {
      free(unirst[i].rst);
      delete rstzip[i];
    }

    delete rstbuf;
    free(cpucount);
    free(zbuffer);
  }

  int32_t openRstunzip(const char* infile, int32_t bufsize, int32_t gzip, int32_t stat) {
    int32_t retval = -1;

    // Test and open files.
    if (gzip == 1) {
      if (strcmp(infile, "-") != 0 && strcmp(infile, "stdin") != 0) {
        uint8_t magic[2] = { 0 };

        FILE* fp = fopen(infile, "r");
        if (fp == NULL) {
          return retval;
        }

        fread(magic, sizeof(char), 2, fp);
        fclose(fp);

        if (magic[0] != 0x1f || magic[1] != 0x8b) {
          fprintf(stderr, "Error: %s is not gzip compressed.\n", infile);
          return retval;
        }
      }
    }

    rstbuf = new RstzipBuffer(bufsize);
    cpucount = (CpuCount*) calloc(bufsize, sizeof(CpuCount));;
    zbuffer = (uint8_t*) calloc(bufsize, sizeof(rstf_unionT));

    retval = openFiles(infile, NULL, gzip);

    if (retval == 1) {
      // Read file header.
      zfread(header->getHeader(), header->getMaxHeaderSize());

      if (header->isValid() == 0) {
        fprintf(stderr, "Error: %s is not rstzip2 compressed.\n", infile);
        exit(1);
      }

      ncpus = header->getNumcpus();

      if (ncpus == 1) {
        unirst[0].rst = (rstf_unionT*) calloc(bufsize, sizeof(rstf_unionT));
        rstunzip[0] = new Rstunzip;
      }

      stats = stat;
      buffersize = bufsize;
    }

    return retval;
  }

  void closeRstunzip() {
    // You may want to fix this.
    // Problem occurs when partial file is decompressed (using -n flag).
    //readFooter();
    //checkFooter();

    if (stats == 1) {
      footer->init();
      calcFooter(footer);
      footer->fprint(stderr);
    }

    closeFiles();

    for (int32_t i = 0; i < RSTZIP_MAXCPUS; i++) {
      free(unirst[i].rst);
      delete rstunzip[i];
    }

    delete rstbuf;
    free(cpucount);
    free(zbuffer);
  }

#if 0

  int32_t compress(rstf_unionT* rst, int32_t nrecs) {
    int32_t zrecs = 0;
    int32_t i;

    for (i = 0; i < nrecs / buffersize; i++) {
      zrecs += compressX(&rst[i * buffersize], buffersize);
    }

    if (nrecs % buffersize != 0 || nrecs == 0) {
      zrecs += compressX(&rst[i * buffersize], nrecs % buffersize);
    }

    return zrecs;
  }

#else

  int32_t compress(rstf_unionT* rst, int32_t nrecs) {
    return compressX(rst, nrecs);
  }

#endif

  // Decompress exactly nrecs RST records to buf.
  int32_t decompress(rstf_unionT* buf, int32_t nrecs) {
    int32_t totalrecs = 0;

    if (nrecs > buffersize) {
      totalrecs += decompress(buf, buffersize);
      totalrecs += decompress(buf, nrecs - buffersize);

#if 0
      fprintf(stderr, "Error: RST buffer size argument must <= %d.\n", buffersize);
      exit(1);
#endif
    } else if (nrecs <= rstbuf->nrecs) {
      // Easy case; rstbuf has sufficient records.
      memcpy(buf, &rstbuf->rstbuf[rstbuf->curindex], nrecs * sizeof(rstf_unionT));
      totalrecs += nrecs;

      // Update buffer info.
      rstbuf->shiftBuffer(rstbuf->curindex + nrecs);
    } else {
      rstf_unionT* pbuf = (rstf_unionT*) buf;

      // Copy buffer contexts; refill buffer; call decompress().
      memcpy(pbuf, &rstbuf->rstbuf[rstbuf->curindex], rstbuf->nrecs * sizeof(rstf_unionT));
      totalrecs += rstbuf->nrecs;

      nrecs -= rstbuf->nrecs;
      pbuf += rstbuf->nrecs;

      // Update buffer info.
      rstbuf->shiftBuffer(0);

      // Call decompressX() to refill rstbuf.
      rstbuf->nrecs = decompressX(rstbuf->rstbuf, buffersize);

      if (rstbuf->nrecs != 0) {
        totalrecs += decompress(pbuf, nrecs);
      }
    }

    return totalrecs;
  }

protected:
  FILE* infp;           // Compressed input file pointer; NULL if none
  FILE* outfp;          // Compressed output file pointer; NULL if none
  FILE* dbgfp;          // Debugging file pointer; NULL if none
  gzFile gzfile;        // Gzip compressor pointer; NULL if none

  Rstzipi* rstzip[RSTZIP_MAXCPUS];
  Rstunzip* rstunzip[RSTZIP_MAXCPUS];

  RstzipBuffer* rstbuf;

  RstSplit* unirst;
  int32_t curcpu;

  CpuCount* cpucount;
  uint8_t* zbuffer;

  RstzipHeader* header;
  RstzipFooter* footer;
  int32_t stats;
  int32_t ncpus;
  int32_t decompress_done;
  int32_t buffersize;

  // Compress RST buffer to file.
  int32_t compressX(rstf_unionT* rst, int32_t nrecs) {
    //uint8_t size8;
    uint16_t size16;
    uint32_t size32;
    int32_t i, zbytes;

    if (ncpus == 1) {
      // Just compress the whole buffer in this case.
      zbytes = rstzip[0]->rstz_compress(zbuffer, rst, nrecs);

      // Write size of compressed RST records plus footers.
      zbuffer[zbytes] = z_FOOTER_T;
      zbytes++;
      zfwrite(&zbytes, sizeof(zbytes));

      // Write compressed RST records.
      zfwrite(zbuffer, zbytes);
    } else {
      // Sort the records in rst into unirst buffers and return the number of cpucount recs used.
      size16 = sortRstTrace(rst, nrecs);

      // Compress the CpuCount records to buffer and write to file.
      size32 = compressCpuCount(zbuffer, cpucount, size16);

#if 0
      fprintf(stderr, "number of cpucount recs=%d (0x%x)\n", size16, size16);
      fprintf(stderr, "size of cpucount recs=%d (0x%x)\n\n", size32, size32);
#endif

      // Write size of compressed CpuCount records to file.
      zfwrite(&size32, sizeof(uint32_t));

      zfwrite(zbuffer, size32);

      // Compress the individual cpu rst buffers to file.
      zbytes = 0;
      for (i = 0; i < RSTZIP_MAXCPUS; i++) {
        if (unirst[i].nrecs > 0) {
          zbytes += rstzip[i]->rstz_compress(&zbuffer[zbytes], unirst[i].rst, unirst[i].nrecs);
          zbuffer[zbytes] = z_FOOTER_T;
          zbytes++;
        }
      }

      // Write size of compressed RST records plus footers.
      zfwrite(&zbytes, sizeof(zbytes));

      // Write compressed RST records.
      zfwrite(zbuffer, zbytes);
    }

    return nrecs;
  }

  // Decompress file to RST buffer; return number of records decompressed.
  int32_t decompressX(rstf_unionT* buf, int32_t nrecs) {
    //rstf_cpuT rstcpu = { CPU_T, 0, 0, 0, 0, 0 };
    uint8_t* zbufptr = NULL;
    int32_t index[RSTZIP_MAXCPUS] = { 0 };
    int32_t decompressed_recs = 0;
    int32_t count, ncpurecs;
    //uint16_t size16;
    uint32_t size32;
    int32_t zbytes;

    if (decompress_done == 0) {
      if (buffersize > nrecs) {
        fprintf(stderr, "Error: buf size must be >= %d in RstzipIF::decompressX().\n", buffersize);
        exit(1);
      }

      if (ncpus == 1) {
        // Read the compressed RST records into zbuffer.
        zfread(&zbytes, sizeof(zbytes));
        zbytes = SWAP_WORD(zbytes);
        zfread(zbuffer, zbytes);
        zbufptr = zbuffer;

        decompressed_recs = rstunzip[0]->rstz_decompress(&zbufptr, buf, nrecs);

        // Set cpuid to 0.
        for (int32_t i = 0; i < nrecs; i++) {
          setRstCpuID(&buf[i], 0);
        }
      } else {
        // Get the size of the compressed CpuCount records.
        zfread(&size32, sizeof(uint32_t));
        size32 = SWAP_WORD(size32);

        // Get the CpuCount records.
        zfread(zbuffer, size32);
        ncpurecs = decompressCpuCount(zbuffer, cpucount, size32);

#if 0
        fprintf(stderr, "number of cpucount recs=%d\n", ncpurecs);
        fprintf(stderr, "size of cpucount recs=%d\n\n", size32);
#endif

        for (int32_t i = 0; i < RSTZIP_MAXCPUS; i++) {
          unirst[i].nrecs = 0;
        }

        for (int32_t i = 0; i < ncpurecs; i++) {
          unirst[cpucount[i].cpu].nrecs += cpucount[i].count32;
        }

        decompressed_recs = 0;

        // Read the compressed RST records into zbuffer.
        zfread(&zbytes, sizeof(zbytes));
        zbytes = SWAP_WORD(zbytes);
        zfread(zbuffer, zbytes);
        zbufptr = zbuffer;

        for (int32_t i = 0; i < RSTZIP_MAXCPUS; i++) {
          if (unirst[i].nrecs != 0) {
            if (rstunzip[i] == NULL) {
              rstunzip[i] = new Rstunzip;
              unirst[i].rst = (rstf_unionT*) calloc(buffersize, sizeof(rstf_unionT));
            }

            rstunzip[i]->rstz_decompress(&zbufptr, unirst[i].rst, unirst[i].nrecs);

            // Set the cpuid fields.
            for (size_t j = 0; j < unirst[i].nrecs; j++) {
              setRstCpuID(&unirst[i].rst[j], i);
            }
          }
        }

        for (int32_t i = 0; i < ncpurecs; i++) {
          curcpu = cpucount[i].cpu;
          count = cpucount[i].count32;

          memcpy(&buf[decompressed_recs], &unirst[curcpu].rst[index[curcpu]], count * sizeof(rstf_unionT));
          index[curcpu] += count;
          decompressed_recs += count;
        }
      }

      if (decompressed_recs == 0) {
        decompress_done = 1;
      }
    }

    return decompressed_recs;
  }

  int32_t openFiles(const char* infile, const char* outfile, int32_t gzip) {
    if (infile != NULL) {
      if (strcmp("-", infile) != 0 && strcmp("stdin", infile) != 0) {
        infp = fopen(infile, "r");
        if (infp == NULL) {
          return -1;
        }
      } else {
        infp = stdin;
      }

      if (gzip == 1) {
        gzfile = gzdopen(fileno(infp), "r");
        if (gzfile == NULL) {
          fprintf(stderr, "Error: unable to create gzfile for decompressing stdin.\n");
          return -1;
        }
      }
    }

    if (outfile != NULL) {
      if (strcmp("-", outfile) != 0 && strcmp("stdout", outfile) != 0) {
        outfp = fopen(outfile, "w");
        if (outfp == NULL) {
          return -1;
        }
      } else {
        outfp = stdout;
      }

      if (gzip == 1) {
        gzfile = gzdopen(fileno(outfp), "w");
        if (gzfile == NULL) {
          fprintf(stderr, "Error: unable to create gzfile for decompressing stdin.\n");
          return -1;
        }
      }
    }

    return 1;
  }

  void closeFiles() {
    if (gzfile)
      gzclose(gzfile);
    if (infp)
      fclose(infp);
    if (outfp)
      fclose(outfp);
  }

  int32_t zfread(void* buf, long size) {
    int32_t ret = -1;

    if (gzfile != NULL) {
      ret = gzread(gzfile, buf, size);
    } else if (infp != NULL) {
      ret = fread(buf, 1, size, infp);
    } else {
      fprintf(stderr, "Error: nothing to read from; all inputs are NULL.\n");
      exit(1);
    }

    return ret;
  }

  int32_t zfwrite(void* buf, long size) {
    int32_t ret = -1;

    if (gzfile != NULL) {
      ret = gzwrite(gzfile, buf, size);
    } else if (outfp != NULL) {
      ret = fwrite(buf, 1, size, outfp);
    } else {
      fprintf(stderr, "Error: nothing to write to; all outputs are NULL.\n");
      exit(1);
    }

    return ret;
  }

  // Sort the rst records in buffer by their cpu, into the unirst buffers. 
  // The total number of CpuRecCount records is returned.
  int32_t sortRstTrace(rstf_unionT* rst, int32_t nrecs) {
    int32_t total_recs = 0;
    int32_t prevcpu = -1;
    int32_t count = 0;

    for (int32_t i = 0; i < RSTZIP_MAXCPUS; i++) {
      unirst[i].nrecs = 0;
    }

    int32_t j = 0;
    for (int32_t i = 0; i < nrecs; i++) {
      curcpu = getRstCpuID(&rst[i]);
      if (curcpu == -1) {
        curcpu = 0;
      }

      if (prevcpu == -1) {
        prevcpu = curcpu;
        count++;
      } else if (prevcpu != curcpu) {
        cpucount[j].cpu = prevcpu;
        cpucount[j].count32 = count;
        j++;

        prevcpu = curcpu;
        count = 1;
      } else {
        count++;
      }

      if (unirst[curcpu].rst == NULL) {
        unirst[curcpu].rst = (rstf_unionT*) calloc(buffersize, sizeof(rstf_unionT));
        rstzip[curcpu] = new Rstzipi;
      }

      unirst[curcpu].rst[unirst[curcpu].nrecs] = rst[i];
      unirst[curcpu].nrecs++;
    }

    cpucount[j].cpu = prevcpu;
    cpucount[j].count32 = count;
    j++;

    return (j);
  }

  int32_t compressCpuCount(uint8_t* zbuf, CpuCount* cpucnt, int32_t ncpurecs) {
    int32_t zbytes = 0;

    for (int32_t i = 0; i < ncpurecs; i++) {
      if (cpucnt[i].count32 <= UINT_MAX) {
        cpucnt[i].count8 = cpucnt[i].count32;

        zbuf[zbytes] = cpucnt[i].cpu;
        zbytes++;

        zbuf[zbytes] = cpucnt[i].count8;
        zbytes++;
      } else if (cpucnt[i].count32 <= USHRT_MAX) {
        cpucnt[i].count16 = cpucnt[i].count32;

        zbuf[zbytes] = cpucnt[i].cpu;
        zbytes++;

        zbuf[zbytes] = 0;
        zbytes++;

        memcpy(&zbuf[zbytes], &cpucnt[i].count16, sizeof(uint16_t));
        zbytes += 2;
      } else {
        zbuf[zbytes] = cpucnt[i].cpu;
        zbytes++;

        zbuf[zbytes] = 0;
        zbytes++;

        zbuf[zbytes] = 0;
        zbytes++;

        zbuf[zbytes] = 0;
        zbytes++;

        memcpy(&zbuf[zbytes], &cpucnt[i].count32, sizeof(uint32_t));
        zbytes += 4;
      }
    }

    return zbytes;
  }

  int32_t decompressCpuCount(uint8_t* zbuf, CpuCount* cpucnt, int32_t zcpurecsize) {
    int32_t i, j;

    i = 0;
    j = 0;
    while (i < zcpurecsize) {
      cpucnt[j].cpu = zbuf[i];
      i++;
      cpucnt[j].count32 = zbuf[i];
      i++;

      if (cpucnt[j].count32 == 0 && i != zcpurecsize && i != 1) {
        uint16_t *base= (uint16_t *)&zbuf[i];
        cpucnt[j].count16 = SWAP_SHORT(*base);
        cpucnt[j].count32 = cpucnt[j].count16;
        i += 2;
      }

      if (cpucnt[j].count32 == 0 && i != zcpurecsize && i != 1) {
        uint32_t *base= (uint32_t *)&zbuf[i];
        cpucnt[j].count32 = SWAP_WORD(*base);
        i += 4;
      }

      j++;
    }

    return j;
  }

  void writeFooter() {
    zfwrite(footer, sizeof(RstzipFooter));
  }

  RstzipFooter* calcFooter(RstzipFooter* foot) {

    foot->init();

    if (decompress_done == 0) {
      for (int32_t i = 0; i < RSTZIP_MAXCPUS; i++) {
        if (rstzip[i] != NULL) {
          foot->max_chunksize += rstzip[i]->max_chunksize;
          foot->total_instr += rstzip[i]->total_instr;
          foot->total_noninstr += rstzip[i]->total_noninstr;
          foot->total_loop_chunk += rstzip[i]->total_loop_chunk;
          foot->total_nonloop_chunk += rstzip[i]->total_nonloop_chunk;
          foot->total_zpavadiff += rstzip[i]->total_zpavadiff;
          foot->total_pavadiff += rstzip[i]->total_pavadiff;
          foot->zero_offset_count += rstzip[i]->zero_offset_count;

          for (int32_t j = 0; j < OFFSET_64BITS_IDX + 1; j++) { 
            foot->offset_count[j] += rstzip[i]->offset_count[j];
          }

          for (int32_t j = 0; j < CHUNKSIZE_RES; j++) { 
            foot->chunksize_count[j] += rstzip[i]->chunksize_count[j];
          }
        }
      }
    } else {
      for (int32_t i = 0; i < RSTZIP_MAXCPUS; i++) {
        if (rstunzip[i] != NULL) {
          foot->max_chunksize += rstunzip[i]->max_chunksize;
          foot->total_instr += rstunzip[i]->total_instr;
          foot->total_noninstr += rstunzip[i]->total_noninstr;
          foot->total_loop_chunk += rstunzip[i]->total_loop_chunk;
          foot->total_nonloop_chunk += rstunzip[i]->total_nonloop_chunk;
          foot->total_zpavadiff += rstunzip[i]->total_zpavadiff;
          foot->total_pavadiff += rstunzip[i]->total_pavadiff;
          foot->zero_offset_count += rstunzip[i]->zero_offset_count;

          for (int32_t j = 0; j < OFFSET_64BITS_IDX + 1; j++) { 
            foot->offset_count[j] += rstunzip[i]->offset_count[j];
          }

          for (int32_t j = 0; j < CHUNKSIZE_RES; j++) { 
            foot->chunksize_count[j] += rstunzip[i]->chunksize_count[j];
          }
        }
      }
    }

    return foot;
  }

  int32_t checkFooter() {
    RstzipFooter calc_footer;
    int32_t numerr = 0;

    calcFooter(&calc_footer);

    if (footer->max_chunksize != calc_footer.max_chunksize) {
      fprintf(stderr, "Warning: decompressed max chunksize (%d) != compressed (%d).\n",
              calc_footer.max_chunksize, footer->max_chunksize);
      numerr++;
    }
    if (footer->total_instr != calc_footer.total_instr) {
      fprintf(stderr, "Warning: decompressed instr recs (%llu) != compressed (%llu).\n",
              calc_footer.total_instr, footer->total_instr);
      numerr++;
    }
    if (footer->total_noninstr != calc_footer.total_noninstr) {
      fprintf(stderr, "Warning: decompressed non-instr recs (%llu) != compressed (%llu).\n",
              calc_footer.total_noninstr, footer->total_noninstr);
      numerr++;
    }
    if (footer->total_loop_chunk != calc_footer.total_loop_chunk) {
      fprintf(stderr, "Warning: decompressed loop chunks (%llu) != compressed (%llu).\n",
              calc_footer.total_loop_chunk, footer->total_loop_chunk);
      numerr++;
    }
    if (footer->total_nonloop_chunk != calc_footer.total_nonloop_chunk) {
      fprintf(stderr, "Warning: decompressed non-loop chunks (%llu)  != compressed (%llu).\n",
              calc_footer.total_nonloop_chunk, footer->total_nonloop_chunk);
      numerr++;
    }
    if (footer->total_zpavadiff != calc_footer.total_zpavadiff) {
      fprintf(stderr, "Warning: decompressed compressed pavadiff recs (%llu) != compressed (%llu).\n",
              calc_footer.total_zpavadiff, footer->total_zpavadiff);
      numerr++;
    }
    if (footer->total_pavadiff != calc_footer.total_pavadiff) {
      fprintf(stderr, "Warning: decompressed pavadiffs (%llu) != compressed (%llu).\n",
              calc_footer.total_pavadiff, footer->total_pavadiff);
      numerr++;
    }
    if (footer->zero_offset_count != calc_footer.zero_offset_count) {
      fprintf(stderr, "Warning: decompressed zero EA offsets (%llu) != compressed (%llu).\n",
              calc_footer.zero_offset_count, footer->zero_offset_count);
      numerr++;
    }

    return numerr;
  }
};  // class RstzipIF

#endif  // __cplusplus

#endif  // _RSTZIPIF_H
