/*
 * Defines structures for describing each opcode.
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
#include "icode.h"
#include "opcodes.h"

PFPI reserved_op[] = { reserved_op1, reserved_op1 };
struct op_desc reserved_ops = {
  "reserved", reserved_op, { 0, 0, 0, 0 }, 0, 0, 0
};

/* The order of the entries in the following tables must not be changed.
 * Bit fields of an instruction are used to index these tables to extract
 * an "opnum" (defined in opcodes.h) and this opnum is used to index
 * the "desc_table" array.
 */
opnum_t normal_opnums[] = {
    reserved_opn, invalid_opn, j_opn, jal_opn,
    beq_opn, bne_opn, blez_opn, bgtz_opn,
    addi_opn, addiu_opn, slti_opn, sltiu_opn,
    andi_opn, ori_opn, xori_opn, lui_opn,
    cop0_opn, cop1_opn, cop2_opn, cop3_opn,
    beql_opn, bnel_opn, blezl_opn, bgtzl_opn,
    reserved_opn, reserved_opn, reserved_opn, reserved_opn,
    reserved_opn, reserved_opn, reserved_opn, reserved_opn,
    lb_opn, lh_opn, lwl_opn, lw_opn,
    lbu_opn, lhu_opn, lwr_opn, reserved_opn,
    sb_opn, sh_opn, swl_opn, sw_opn,
    reserved_opn, reserved_opn, swr_opn, cache_opn,
    ll_opn, lwc1_opn, lwc2_opn, lwc3_opn,
    reserved_opn, ldc1_opn, ldc2_opn, ldc3_opn,
    sc_opn, swc1_opn, swc2_opn, swc3_opn,
    reserved_opn, sdc1_opn, sdc2_opn, sdc3_opn
};

opnum_t special_opnums[] = {
    sll_opn, reserved_opn, srl_opn, sra_opn,
    sllv_opn, reserved_opn, srlv_opn, srav_opn,
    jr_opn, jalr_opn, reserved_opn, reserved_opn,
    syscall_opn, break_opn, reserved_opn, sync_opn,
    mfhi_opn, mthi_opn, mflo_opn, mtlo_opn,
    reserved_opn, reserved_opn, reserved_opn, reserved_opn,
    mult_opn, multu_opn, div_opn, divu_opn,
    reserved_opn, reserved_opn, reserved_opn, reserved_opn,
    add_opn, addu_opn, sub_opn, subu_opn,
    and_opn, or_opn, xor_opn, nor_opn,
    reserved_opn, reserved_opn, slt_opn, sltu_opn,
    reserved_opn, reserved_opn, reserved_opn, reserved_opn,
    tge_opn, tgeu_opn, tlt_opn, tltu_opn,
    teq_opn, reserved_opn, tne_opn, reserved_opn,
    reserved_opn, reserved_opn, reserved_opn, reserved_opn,
    reserved_opn, reserved_opn, reserved_opn, reserved_opn
};

opnum_t regimm_opnums[] = {
    bltz_opn, bgez_opn, bltzl_opn, bgezl_opn,
    reserved_opn, reserved_opn, reserved_opn, reserved_opn,
    tgei_opn, tgeiu_opn, tlti_opn, tltiu_opn,
    teqi_opn, reserved_opn, tnei_opn, reserved_opn,
    bltzal_opn, bgezal_opn, bltzall_opn, bgezall_opn,
    reserved_opn, reserved_opn, reserved_opn, reserved_opn,
    reserved_opn, reserved_opn, reserved_opn, reserved_opn,
    reserved_opn, reserved_opn, reserved_opn, reserved_opn,
};

opnum_t cop1func_opnums[][64] = {
    {
        /* single precision ops */
        add_s_opn, sub_s_opn, mul_s_opn, div_s_opn,
        sqrt_s_opn, abs_s_opn, mov_s_opn, neg_s_opn,
        cop_reserved_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        round_w_s_opn, trunc_w_s_opn, ceil_w_s_opn, floor_w_s_opn,
        cop_reserved_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        cop_reserved_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        cop_reserved_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        cop_reserved_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        cop_invalid_opn, cvt_d_s_opn, cop_reserved_opn, cop_reserved_opn,
        cvt_w_s_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        cop_reserved_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        cop_reserved_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        c_f_s_opn, c_un_s_opn, c_eq_s_opn, c_ueq_s_opn,
        c_olt_s_opn, c_ult_s_opn, c_ole_s_opn, c_ule_s_opn,
        c_sf_s_opn, c_ngle_s_opn, c_seq_s_opn, c_ngl_s_opn,
        c_lt_s_opn, c_nge_s_opn, c_le_s_opn, c_ngt_s_opn,
    },
    {
        add_d_opn, sub_d_opn, mul_d_opn, div_d_opn,
        sqrt_d_opn, abs_d_opn, mov_d_opn, neg_d_opn,
        cop_reserved_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        round_w_d_opn, trunc_w_d_opn, ceil_w_d_opn, floor_w_d_opn,
        cop_reserved_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        cop_reserved_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        cop_reserved_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        cop_reserved_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        cvt_s_d_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        cvt_w_d_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        cop_reserved_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        cop_reserved_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        c_f_d_opn, c_un_d_opn, c_eq_d_opn, c_ueq_d_opn,
        c_olt_d_opn, c_ult_d_opn, c_ole_d_opn, c_ule_d_opn,
        c_sf_d_opn, c_ngle_d_opn, c_seq_d_opn, c_ngl_d_opn,
        c_lt_d_opn, c_nge_d_opn, c_le_d_opn, c_ngt_d_opn,
    },
    {
        cop_invalid_opn, cop_invalid_opn, cop_invalid_opn, cop_invalid_opn,
        cop_invalid_opn, cop_invalid_opn, cop_invalid_opn, cop_invalid_opn,
        cop_reserved_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        cop_invalid_opn, cop_invalid_opn, cop_invalid_opn, cop_invalid_opn,
        cop_reserved_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        cop_reserved_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        cop_reserved_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        cop_reserved_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        cvt_s_w_opn, cvt_d_w_opn, cop_reserved_opn, cop_reserved_opn,
        cop_invalid_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        cop_reserved_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        cop_reserved_opn, cop_reserved_opn, cop_reserved_opn, cop_reserved_opn,
        cop_invalid_opn, cop_invalid_opn, cop_invalid_opn, cop_invalid_opn,
        cop_invalid_opn, cop_invalid_opn, cop_invalid_opn, cop_invalid_opn,
        cop_invalid_opn, cop_invalid_opn, cop_invalid_opn, cop_invalid_opn,
        cop_invalid_opn, cop_invalid_opn, cop_invalid_opn, cop_invalid_opn,
    }
};

opnum_t mfc_opnums[] = {
    mfc0_opn, mfc1_opn, mfc2_opn, mfc3_opn
};

opnum_t mtc_opnums[] = {
    mtc0_opn, mtc1_opn, mtc2_opn, mtc3_opn
};

opnum_t cfc_opnums[] = {
    cfc0_opn, cfc1_opn, cfc2_opn, cfc3_opn
};

opnum_t ctc_opnums[] = {
    ctc0_opn, ctc1_opn, ctc2_opn, ctc3_opn
};

opnum_t bc_opnums[][4] = {
  {bc0f_opn, bc0t_opn, bc0fl_opn, bc0tl_opn},
  {bc1f_opn, bc1t_opn, bc1fl_opn, bc1tl_opn},
  {bc2f_opn, bc2t_opn, bc2fl_opn, bc2tl_opn},
  {bc3f_opn, bc3t_opn, bc3fl_opn, bc3tl_opn}
};

opnum_t user_opnums[] = {
  swallow_opn,  call_opn,     spawn_opn,    reserved_opn, 
  reserved_opn, reserved_opn, reserved_opn, reserved_opn,
  reserved_opn, reserved_opn, reserved_opn, reserved_opn,
  reserved_opn, reserved_opn, reserved_opn, reserved_opn,
  reserved_opn, reserved_opn, reserved_opn, reserved_opn,
  reserved_opn, reserved_opn, reserved_opn, reserved_opn,
  reserved_opn, reserved_opn, reserved_opn, reserved_opn,
  reserved_opn, reserved_opn, reserved_opn, reserved_opn
};

#if (defined TLS)
opnum_t tls_opnums[] = {
  aspectReductionBegin_opn, // 0x70000000
  aspectReductionEnd_opn,   // 0x70000001
  aspectAtomicBegin_opn,    // 0x70000002
  aspectAcquireBegin_opn,   // 0x70000003
  aspectAcquireRetry_opn,   // 0x70000004
  aspectAcquireExit_opn,    // 0x70000005
  aspectAcquire2Release_opn,// 0x70000006
  aspectReleaseBegin_opn,   // 0x70000007
  aspectReleaseEnter_opn,   // 0x70000008
  aspectAtomicEnd_opn,      // 0x70000009
  reserved_opn,
  reserved_opn,
  reserved_opn,
  reserved_opn,       reserved_opn,       reserved_opn,       reserved_opn,
  reserved_opn,       reserved_opn,       reserved_opn,       reserved_opn,
  reserved_opn,       reserved_opn,       reserved_opn,       reserved_opn,
  reserved_opn,       reserved_opn,       reserved_opn,       reserved_opn,
  reserved_opn,       reserved_opn,       reserved_opn,       reserved_opn
};
#endif

/* The order of the entries in this table is important. The opnums
 * defined in opcodes.h are used to index this table. New entries
 * should be added at the end.
 */
struct op_desc desc_table[] = {
    /* char *opname; PFPI func; char regflags[4];
     * int32_t iflags; short opflags; short cycles;
     */
    { "reserved", reserved_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "regimm", NULL, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "j", j_op, { 0, 0, 0, 0 }, IS_JUMP, 0, 1 },
    { "jal", jal_op, { 0, 0, 0, 0 }, IS_JUMP, 0, 1 },
    { "beq", beq_op, { REG0, REG0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bne", bne_op, { REG0, REG0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "blez", blez_op, { REG0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bgtz", bgtz_op, { REG0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "addi", addi_op, { REG0, REG0 | MOD0, 0, 0 }, 0, 0, 1 },
    { "addiu", addiu_op, { REG0, REG0 | MOD0, 0, 0 }, MOD0_IS_NOP, 0, 1 },
    { "slti", slti_op, { REG0, REG0 | MOD0, 0, 0 }, MOD0_IS_NOP, 0, 1 },
    { "sltiu", sltiu_op, { REG0, REG0 | MOD0, 0, 0 }, MOD0_IS_NOP, 0, 1 },
    { "andi", andi_op, { REG0, REG0 | MOD0, 0, 0 }, MOD0_IS_NOP, 0, 1 },
    { "ori", ori_op, { REG0, REG0 | MOD0, 0, 0 }, MOD0_IS_NOP, 0, 1 },
    { "xori", xori_op, { REG0, REG0 | MOD0, 0, 0 }, MOD0_IS_NOP, 0, 1 },
    { "lui", lui_op, { 0, REG0 | MOD0, 0, 0 }, MOD0_IS_NOP, 0, 1 },
    { "cop0", cop0_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "cop1", cop1_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "cop2", cop2_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "cop3", cop3_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "beql", beql_op, { REG0, REG0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bnel", bnel_op, { REG0, REG0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "blezl", blezl_op, { REG0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bgtzl", bgtzl_op, { REG0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "lb", lb_op, { REG0, REG0 | MOD0, 0, 0 }, 0, E_READBS, 1 },
    { "lh", lh_op, { REG0, REG0 | MOD0, 0, 0 }, 0, E_READHS, 1 },
    { "lwl", lwl_op, { REG0, REG0 | MOD0, 0, 0 }, 0, E_LWL, 1 },
    { "lw", lw_op, { REG0, REG0 | MOD0, 0, 0 }, 0, E_READW, 1 },
    { "lbu", lbu_op, { REG0, REG0 | MOD0, 0, 0 }, 0, E_READB, 1 },
    { "lhu", lhu_op, { REG0, REG0 | MOD0, 0, 0 }, 0, E_READH, 1 },
    { "lwr", lwr_op, { REG0, REG0 | MOD0, 0, 0 }, 0, E_LWR, 1 },
    { "sb", sb_op, { REG0, REG0, 0, 0 }, 0, E_WRITEB, 1 },
    { "sh", sh_op, { REG0, REG0, 0, 0 }, 0, E_WRITEH, 1 },
    { "swl", swl_op, { REG0, REG0, 0, 0 }, 0, E_SWL, 1 },
    { "sw", sw_op, { REG0, REG0, 0, 0 }, 0, E_WRITEW, 1 },
    { "swr", swr_op, { REG0, REG0, 0, 0 }, 0, E_SWR, 1 },
    { "cache", cache_op, { 0, 0, 0, 0 }, SIDE_EFFECT, 0, 1 },
    { "ll", ll_op, { REG0, REG0 | MOD0, 0, 0 }, 0, E_READW | E_SYNC, 1 },
    { "lwc1", lwc1_op, { REG0, REG1 | MOD1, 0, 0 }, 0, E_FREADW, 1 },
    { "lwc2", lwc2_op, { REG0, 0, 0, 0 }, 0, E_READW, 1 },
    { "lwc3", lwc3_op, { REG0, 0, 0, 0 }, 0, E_READW, 1 },
    { "ldc1", ldc1_op, { REG0, DREG1 | MOD1, 0, 0 }, 0, E_FREADD, 1 },
    { "ldc2", ldc2_op, { REG0, 0, 0, 0 }, 0, E_READD, 1 },
    { "ldc3", ldc3_op, { REG0, 0, 0, 0 }, 0, E_READD, 1 },
    { "sc", sc_op, { REG0, REG0 | MOD0, 0, 0 }, 0, E_WRITEW | E_SYNC, 1 },
    { "swc1", swc1_op, { REG0, REG1, 0, 0 }, 0, E_FWRITEW, 1 },
    { "swc2", swc2_op, { REG0, 0, 0, 0 }, 0, E_WRITEW, 1 },
    { "swc3", swc3_op, { REG0, 0, 0, 0 }, 0, E_WRITEW, 1 },
    { "sdc1", sdc1_op, { REG0, DREG1, 0, 0 }, 0, E_FWRITED, 1 },
    { "sdc2", sdc2_op, { REG0, 0, 0, 0 }, 0, E_WRITED, 1 },
    { "sdc3", sdc3_op, { REG0, 0, 0, 0 }, 0, E_WRITED, 1 },
    { "sll", sll_op, { 0, REG0, REG0 | MOD0, 0 }, MOD0_IS_NOP, 0, 1 },
    { "srl", srl_op, { 0, REG0, REG0 | MOD0, 0 }, MOD0_IS_NOP, 0, 1 },
    { "sra", sra_op, { 0, REG0, REG0 | MOD0, 0 }, MOD0_IS_NOP, 0, 1 },
    { "sllv", sllv_op, { REG0, REG0, REG0 | MOD0, 0 }, MOD0_IS_NOP, 0, 1 },
    { "srlv", srlv_op, { REG0, REG0, REG0 | MOD0, 0 }, MOD0_IS_NOP, 0, 1 },
    { "srav", srav_op, { REG0, REG0, REG0 | MOD0, 0 }, MOD0_IS_NOP, 0, 1 },
    { "jr", jr_op, { REG0, 0, 0, 0 }, IS_JUMPREG, 0, 1 },
    { "jalr", jalr_op, { REG0, 0, REG0 | MOD0, 0 }, IS_JUMPREG, 0, 1 },
    { "syscall", syscall_op, { 0, 0, 0, 0 }, SIDE_EFFECT, 0, 1 },
    { "break", break_op, { 0, 0, 0, 0 }, SIDE_EFFECT, 0, 1 },
    { "sync", sync_op, { 0, 0, 0, 0 }, 0, E_SYNC, 1 },
    { "mfhi", mfhi_op, { 0, 0, REG0 | MOD0, 0 }, SIDE_EFFECT, 0, 1 },
    { "mthi", mthi_op, { REG0, 0, 0, 0 }, SIDE_EFFECT, 0, 1 },
    { "mflo", mflo_op, { 0, 0, REG0 | MOD0, 0 }, SIDE_EFFECT, 0, 1 },
    { "mtlo", mtlo_op, { REG0, 0, 0, 0 }, SIDE_EFFECT, 0, 1 },
    { "mult", mult_op, { REG0, REG0, 0, 0 }, SIDE_EFFECT, 0, 1 },
    { "multu", multu_op, { REG0, REG0, 0, 0 }, SIDE_EFFECT, 0, 1 },
    { "div", div_op, { REG0, REG0, 0, 0 }, SIDE_EFFECT, 0, 1 },
    { "divu", divu_op, { REG0, REG0, 0, 0 }, SIDE_EFFECT, 0, 1 },
    { "add", add_op, { REG0, REG0, REG0 | MOD0, 0 }, 0, 0, 1 },
    { "addu", addu_op, { REG0, REG0, REG0 | MOD0, 0 }, MOD0_IS_NOP, 0, 1 },
    { "sub", sub_op, { REG0, REG0, REG0 | MOD0, 0 }, 0, 0, 1 },
    { "subu", subu_op, { REG0, REG0, REG0 | MOD0, 0 }, MOD0_IS_NOP, 0, 1 },
    { "and", and_op, { REG0, REG0, REG0 | MOD0, 0 }, MOD0_IS_NOP, 0, 1 },
    { "or", or_op, { REG0, REG0, REG0 | MOD0, 0 }, MOD0_IS_NOP, 0, 1 },
    { "xor", xor_op, { REG0, REG0, REG0 | MOD0, 0 }, MOD0_IS_NOP, 0, 1 },
    { "nor", nor_op, { REG0, REG0, REG0 | MOD0, 0 }, MOD0_IS_NOP, 0, 1 },
    { "slt", slt_op, { REG0, REG0, REG0 | MOD0, 0 }, MOD0_IS_NOP, 0, 1 },
    { "sltu", sltu_op, { REG0, REG0, REG0 | MOD0, 0 }, MOD0_IS_NOP, 0, 1 },
    { "tge", tge_op, { REG0, REG0, 0, 0 }, 0, 0, 1 },
    { "tgeu", tgeu_op, { REG0, REG0, 0, 0 }, 0, 0, 1 },
    { "tlt", tlt_op, { REG0, REG0, 0, 0 }, 0, 0, 1 },
    { "tltu", tltu_op, { REG0, REG0, 0, 0 }, 0, 0, 1 },
    { "teq", teq_op, { REG0, REG0, 0, 0 }, 0, 0, 1 },
    { "tne", tne_op, { REG0, REG0, 0, 0 }, 0, 0, 1 },
    { "bltz", bltz_op, { REG0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bgez", bgez_op, { REG0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bltzl", bltzl_op, { REG0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bgezl", bgezl_op, { REG0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "tgei", tgei_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "tgeiu", tgeiu_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "tlti", tlti_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "tltiu", tltiu_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "teqi", teqi_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "tnei", tnei_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "bltzal", bltzal_op, { REG0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bgezal", bgezal_op, { REG0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bltzall", bltzall_op, { REG0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bgezall", bgezall_op, { REG0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "add.s", c1_add_s, { 0, REG1, REG1, REG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "sub.s", c1_sub_s, { 0, REG1, REG1, REG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "mul.s", c1_mul_s, { 0, REG1, REG1, REG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "div.s", c1_div_s, { 0, REG1, REG1, REG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "sqrt.s", c1_sqrt_s, { 0, 0, REG1, REG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "abs.s", c1_abs_s, { 0, 0, REG1, REG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "mov.s", c1_mov_s, { 0, 0, REG1, REG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "neg.s", c1_neg_s, { 0, 0, REG1, REG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "round.w.s", c1_round_w_s, { 0, 0, REG1, REG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "trunc.w.s", c1_trunc_w_s, { 0, 0, REG1, REG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "ceil.w.s", c1_ceil_w_s, { 0, 0, REG1, REG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "floor.w.s", c1_floor_w_s, { 0, 0, REG1, REG1 | MOD1 }, FPU_FCR, 0, 1 },
    /* { "cvt.s.s", invalid_op, { 0, 0, 0, 0 }, FPU_FCR, 0, 1 }, */
    { "cvt.d.s", c1_cvt_d_s, { 0, 0, REG1, DREG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "cvt.w.s", c1_cvt_w_s, { 0, 0, REG1, REG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "c.f.s", c1_c_f_s, { 0, REG1, REG1, 0 }, 0, 0, 1 },
    { "c.un.s", c1_c_un_s, { 0, REG1, REG1, 0 }, 0, 0, 1 },
    { "c.eq.s", c1_c_eq_s, { 0, REG1, REG1, 0 }, 0, 0, 1 },
    { "c.ueq.s", c1_c_ueq_s, { 0, REG1, REG1, 0 }, 0, 0, 1 },
    { "c.olt.s", c1_c_olt_s, { 0, REG1, REG1, 0 }, 0, 0, 1 },
    { "c.ult.s", c1_c_ult_s, { 0, REG1, REG1, 0 }, 0, 0, 1 },
    { "c.ole.s", c1_c_ole_s, { 0, REG1, REG1, 0 }, 0, 0, 1 },
    { "c.ule.s", c1_c_ule_s, { 0, REG1, REG1, 0 }, 0, 0, 1 },
    { "c.sf.s", c1_c_sf_s, { 0, REG1, REG1, 0 }, 0, 0, 1 },
    { "c.ngle.s", c1_c_ngle_s, { 0, REG1, REG1, 0 }, 0, 0, 1 },
    { "c.seq.s", c1_c_seq_s, { 0, REG1, REG1, 0 }, 0, 0, 1 },
    { "c.ngl.s", c1_c_ngl_s, { 0, REG1, REG1, 0 }, 0, 0, 1 },
    { "c.lt.s", c1_c_lt_s, { 0, REG1, REG1, 0 }, 0, 0, 1 },
    { "c.nge.s", c1_c_nge_s, { 0, REG1, REG1, 0 }, 0, 0, 1 },
    { "c.le.s", c1_c_le_s, { 0, REG1, REG1, 0 }, 0, 0, 1 },
    { "c.ngt.s", c1_c_ngt_s, { 0, REG1, REG1, 0 }, 0, 0, 1 },
    { "add.d", c1_add_d, { 0, DREG1, DREG1, DREG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "sub.d", c1_sub_d, { 0, DREG1, DREG1, DREG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "mul.d", c1_mul_d, { 0, DREG1, DREG1, DREG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "div.d", c1_div_d, { 0, DREG1, DREG1, DREG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "sqrt.d", c1_sqrt_d, { 0, 0, DREG1, DREG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "abs.d", c1_abs_d, { 0, 0, DREG1, DREG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "mov.d", c1_mov_d, { 0, 0, DREG1, DREG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "neg.d", c1_neg_d, { 0, 0, DREG1, DREG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "round.w.d", c1_round_w_d, { 0, 0, DREG1, DREG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "trunc.w.d", c1_trunc_w_d, { 0, 0, DREG1, DREG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "ceil.w.d", c1_ceil_w_d, { 0, 0, DREG1, DREG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "floor.w.d", c1_floor_w_d, { 0, 0, DREG1, DREG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "cvt.s.d", c1_cvt_s_d, { 0, 0, DREG1, REG1 | MOD1 }, FPU_FCR, 0, 1 },
    /* { "cvt.d.d", c1_cvt_d_d, { 0, 0, DREG1, DREG1 | MOD1 }, FPU_FCR, 0, 1 }, */
    { "cvt.w.d", c1_cvt_w_d, { 0, 0, DREG1, REG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "c.f.d", c1_c_f_d, { 0, DREG1, DREG1, 0 }, 0, 0, 1 },
    { "c.un.d", c1_c_un_d, { 0, DREG1, DREG1, 0 }, 0, 0, 1 },
    { "c.eq.d", c1_c_eq_d, { 0, DREG1, DREG1, 0 }, 0, 0, 1 },
    { "c.ueq.d", c1_c_ueq_d, { 0, DREG1, DREG1, 0 }, 0, 0, 1 },
    { "c.olt.d", c1_c_olt_d, { 0, DREG1, DREG1, 0 }, 0, 0, 1 },
    { "c.ult.d", c1_c_ult_d, { 0, DREG1, DREG1, 0 }, 0, 0, 1 },
    { "c.ole.d", c1_c_ole_d, { 0, DREG1, DREG1, 0 }, 0, 0, 1 },
    { "c.ule.d", c1_c_ule_d, { 0, DREG1, DREG1, 0 }, 0, 0, 1 },
    { "c.sf.d", c1_c_sf_d, { 0, DREG1, DREG1, 0 }, 0, 0, 1 },
    { "c.ngle.d", c1_c_ngle_d, { 0, DREG1, DREG1, 0 }, 0, 0, 1 },
    { "c.seq.d", c1_c_seq_d, { 0, DREG1, DREG1, 0 }, 0, 0, 1 },
    { "c.ngl.d", c1_c_ngl_d, { 0, DREG1, DREG1, 0 }, 0, 0, 1 },
    { "c.lt.d", c1_c_lt_d, { 0, DREG1, DREG1, 0 }, 0, 0, 1 },
    { "c.nge.d", c1_c_nge_d, { 0, DREG1, DREG1, 0 }, 0, 0, 1 },
    { "c.le.d", c1_c_le_d, { 0, DREG1, DREG1, 0 }, 0, 0, 1 },
    { "c.ngt.d", c1_c_ngt_d, { 0, DREG1, DREG1, 0 }, 0, 0, 1 },
    { "cvt.s.w", c1_cvt_s_w, { 0, 0, REG1, REG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "cvt.d.w", c1_cvt_d_w, { 0, 0, REG1, DREG1 | MOD1 }, FPU_FCR, 0, 1 },
    { "mfc0", mfc0_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "mfc1", mfc1_op, { 0, REG0 | MOD0, REG1, 0 }, 0, 0, 1 },
    { "mfc2", mfc2_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "mfc3", mfc3_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "mtc0", mtc0_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "mtc1", mtc1_op, { 0, REG0, REG1 | MOD1, 0 }, 0, 0, 1 },
    { "mtc2", mtc2_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "mtc3", mtc3_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "cfc0", cfc0_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "cfc1", cfc1_op, { 0, REG0 | MOD0, CREG1, 0 }, SIDE_EFFECT, 0, 1 },
    { "cfc2", cfc2_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "cfc3", cfc3_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "ctc0", ctc0_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "ctc1", ctc1_op, { 0, REG0, CREG1, 0 }, SIDE_EFFECT, 0, 1 },
    { "ctc2", ctc2_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "ctc3", ctc3_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "bc0f", bc0f_op, { 0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bc0t", bc0t_op, { 0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bc0fl", bc0fl_op, { 0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bc0tl", bc0tl_op, { 0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bc1f", bc1f_op, { 0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bc1t", bc1t_op, { 0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bc1fl", bc1fl_op, { 0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bc1tl", bc1tl_op, { 0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bc2f", bc2f_op, { 0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bc2t", bc2t_op, { 0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bc2fl", bc2fl_op, { 0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bc2tl", bc2tl_op, { 0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bc3f", bc3f_op, { 0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bc3t", bc3t_op, { 0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bc3fl", bc3fl_op, { 0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "bc3tl", bc3tl_op, { 0, 0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "cop_reserved", reserved_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "cop_invalid", reserved_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "terminate", reserved_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "b", b_op, { REG0, REG0, 0, 0 }, IS_BRANCH, 0, 1 },
    { "li", li_op, { 0, REG0 | MOD0, 0, 0 }, MOD0_IS_NOP, 0, 1 },
    { "move", move_op, { REG0, 0, REG0 | MOD0, 0 }, MOD0_IS_NOP, 0, 1 },
    { "nop", nop_op, { 0, 0, 0, 0 }, 0, 0, 1 },
    { "swallow_op", swallow_op, { 0, 0, 0, 0}, 0, 0, 1 },
    { "call_op", call_op, {0, 0, 0, 0}, IS_JUMP, 0, 1 },
    { "spawn_op", spawn_op, { 0, 0, 0, 0}, IS_JUMP, 0, 1 }
#if (defined TLS)
    ,
    { "aspectReductionBegin",  aspectReductionBegin_op,  { 0, 0, 0, 0}, 0, 0, 1},
    { "aspectReductionEnd",    aspectReductionEnd_op,    { 0, 0, 0, 0}, 0, 0, 1},
    { "aspectAtomicBegin",     aspectAtomicBegin_op,     { 0, 0, 0, 0}, 0, 0, 1},
    { "aspectAcquireBegin",    aspectAcquireBegin_op,    { 0, 0, 0, 0}, 0, 0, 1},
    { "aspectAcquireRetry",    aspectAcquireRetry_op,    { 0, 0, 0, 0}, 0, 0, 1},
    { "aspectAcquireExit",     aspectAcquireExit_op,     { 0, 0, 0, 0}, 0, 0, 1},
    { "aspectAcquire2Release", aspectAcquire2Release_op, { 0, 0, 0, 0}, 0, 0, 1},
    { "aspectReleaseBegin",    aspectReleaseBegin_op,    { 0, 0, 0, 0}, 0, 0, 1},
    { "aspectReleaseEnter",    aspectReleaseEnter_op,    { 0, 0, 0, 0}, 0, 0, 1},
    { "aspectAtomicEnd",       aspectAtomicEnd_op,       { 0, 0, 0, 0}, 0, 0, 1}
#endif
};
