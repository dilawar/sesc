#ifndef OPCODES_H
#define OPCODES_H
/*
 * Definitions of opnums for MIPS opcodes so that all instructions can be
 * treated uniformly by using the "desc_table" array.
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

/* macros for defining functions to be called through pointers */
#define OP(NAME) \
    icode_ptr \
    NAME(icode_ptr picode, thread_ptr pthread)


struct op_desc {
  const char *opname;
  PFPI *func;
  char regflags[4];
  int32_t iflags;           /* extra instruction flags */
  short opflags;        /* opcode flags passed through to the event type */
  short cycles;
};

/* flags for the registers */
#define REG0    0x1     /* general register used */
#define REG1    0x2     /* coprocessor 1 register used */
#define DREG1   0x4     /* FPU double */
#define CREG1   0x8     /* FPU control */
#define MOD0    0x10    /* general register modified */
#define MOD1    0x20    /* FPU register modified */

/* Flags for the opcodes that are not defined in event.h.
 * Note: these flags no longer need to be distinct from
 * the flags used in event.h.
 */
#define IS_BRANCH       0x200   /* is a branch */
#define IS_JUMP         0x400   /* is an absolute jump (j or jal) */
#define IS_JUMPREG      0x800   /* is a register jump (jr or jalr) */
#define MOD0_IS_NOP     0x1000  /* if write to r0 is the same as a nop */
#define SIDE_EFFECT     0x2000  /* if instruction has side effect */
#define FPU_FCR         0x4000  /* if instr requires restoring fcr */

/* bit mask for branches and jumps */
#define BRANCH_OR_JUMP  (IS_BRANCH | IS_JUMP | IS_JUMPREG)

/* The order of these opnums is important. They are used to index the
 * "desc_table" array. New opnums should be added at the end.
 */
typedef enum opnum_e {
    /* normal ops */
    reserved_opn, invalid_opn, j_opn, jal_opn,
    beq_opn, bne_opn, blez_opn, bgtz_opn,
    addi_opn, addiu_opn, slti_opn, sltiu_opn,
    andi_opn, ori_opn, xori_opn, lui_opn,
    cop0_opn, cop1_opn, cop2_opn, cop3_opn,
    beql_opn, bnel_opn, blezl_opn, bgtzl_opn,
    lb_opn, lh_opn, lwl_opn, lw_opn,
    lbu_opn, lhu_opn, lwr_opn,
    sb_opn, sh_opn, swl_opn, sw_opn,
    swr_opn, cache_opn,
    ll_opn, lwc1_opn, lwc2_opn, lwc3_opn,
    ldc1_opn, ldc2_opn, ldc3_opn,
    sc_opn, swc1_opn, swc2_opn, swc3_opn,
    sdc1_opn, sdc2_opn, sdc3_opn,

    /* special ops */
    sll_opn, srl_opn, sra_opn,
    sllv_opn, srlv_opn, srav_opn,
    jr_opn, jalr_opn,
    syscall_opn, break_opn, sync_opn,
    mfhi_opn, mthi_opn, mflo_opn, mtlo_opn,
    mult_opn, multu_opn, div_opn, divu_opn,
    add_opn, addu_opn, sub_opn, subu_opn,
    and_opn, or_opn, xor_opn, nor_opn,
    slt_opn, sltu_opn,
    tge_opn, tgeu_opn, tlt_opn, tltu_opn,
    teq_opn, tne_opn,

    /* regimm ops */
    bltz_opn, bgez_opn, bltzl_opn, bgezl_opn,
    tgei_opn, tgeiu_opn, tlti_opn, tltiu_opn,
    teqi_opn, tnei_opn,
    bltzal_opn, bgezal_opn, bltzall_opn, bgezall_opn,

    /* single precision ops */
    add_s_opn, sub_s_opn, mul_s_opn, div_s_opn,
    sqrt_s_opn, abs_s_opn, mov_s_opn, neg_s_opn,
    round_w_s_opn, trunc_w_s_opn, ceil_w_s_opn, floor_w_s_opn,
    cvt_d_s_opn, cvt_w_s_opn,
    c_f_s_opn, c_un_s_opn, c_eq_s_opn, c_ueq_s_opn,
    c_olt_s_opn, c_ult_s_opn, c_ole_s_opn, c_ule_s_opn,
    c_sf_s_opn, c_ngle_s_opn, c_seq_s_opn, c_ngl_s_opn,
    c_lt_s_opn, c_nge_s_opn, c_le_s_opn, c_ngt_s_opn,

    /* double precision ops */
    add_d_opn, sub_d_opn, mul_d_opn, div_d_opn,
    sqrt_d_opn, abs_d_opn, mov_d_opn, neg_d_opn,
    round_w_d_opn, trunc_w_d_opn, ceil_w_d_opn, floor_w_d_opn,
    cvt_s_d_opn, cvt_w_d_opn,
    c_f_d_opn, c_un_d_opn, c_eq_d_opn, c_ueq_d_opn,
    c_olt_d_opn, c_ult_d_opn, c_ole_d_opn, c_ule_d_opn,
    c_sf_d_opn, c_ngle_d_opn, c_seq_d_opn, c_ngl_d_opn,
    c_lt_d_opn, c_nge_d_opn, c_le_d_opn, c_ngt_d_opn,

    /* fixed-point precision ops */
    cvt_s_w_opn, cvt_d_w_opn,

    mfc0_opn, mfc1_opn, mfc2_opn, mfc3_opn,
    mtc0_opn, mtc1_opn, mtc2_opn, mtc3_opn,
    cfc0_opn, cfc1_opn, cfc2_opn, cfc3_opn,
    ctc0_opn, ctc1_opn, ctc2_opn, ctc3_opn,
    bc0f_opn, bc0t_opn, bc0fl_opn, bc0tl_opn,
    bc1f_opn, bc1t_opn, bc1fl_opn, bc1tl_opn,
    bc2f_opn, bc2t_opn, bc2fl_opn, bc2tl_opn,
    bc3f_opn, bc3t_opn, bc3fl_opn, bc3tl_opn,

    cop_reserved_opn, cop_invalid_opn, terminate_opn,

    /* synthesized ops */
    b_opn, li_opn, move_opn, nop_opn,

    /* user defined ops */
    swallow_opn, call_opn, spawn_opn, 

#if (defined TLS)
    // TLS user-defined opcodes
    aspectReductionBegin_opn,
    aspectReductionEnd_opn,
    aspectAtomicBegin_opn,
    aspectAcquireBegin_opn,
    aspectAcquireRetry_opn,
    aspectAcquireExit_opn,
    aspectAcquire2Release_opn,
    aspectReleaseBegin_opn,
    aspectReleaseEnter_opn,
    aspectAtomicEnd_opn,
#endif

    /* this one must be last */
    max_opnum
} opnum_t;


#define MAX_UD_PARAMS 10

typedef enum ud_class_e {
  ud_call, ud_spawn, ud_fork, ud_regsynch, ud_compute
} ud_class_t;


/* function prototypes */
  OP(mint_sesc_fetch_op);
  OP(mint_sesc_unlock_op);
  OP(mint_sesc_spawn);
  OP(mint_sesc_sysconf);
  OP(mint_sesc_wait);
  OP(mint_sesc_pseudoreset);
  OP(mint_sesc_yield);
  OP(mint_sesc_suspend);
  OP(mint_sesc_resume);
  OP(mint_sesc_simulation_mark);
  OP(mint_sesc_simulation_mark_id);
  OP(mint_sesc_fast_sim_begin);
  OP(mint_sesc_fast_sim_end);
  OP(mint_sesc_preevent);
  OP(mint_sesc_postevent);
  OP(mint_sesc_memfence);
  OP(mint_sesc_acquire);
  OP(mint_sesc_release);
  OP(mint_exit);
#ifdef TASKSCALAR
  OP(mint_sesc_commit);
  OP(mint_sesc_fork_successor);
#endif
  OP(reserved_op1);
  OP(cop_reserved);
  OP(terminator1);
  OP(unimplemented_op);

extern PPFPI6
    lb_op, lh_op, lwl_op, lw_op,
    lbu_op, lhu_op, lwr_op,
    ll_op, lwc1_op, lwc2_op, lwc3_op,
    ldc1_op, ldc2_op, ldc3_op;

extern PPFPI12
    sb_op, sh_op, swl_op, sw_op,
    swr_op, cache_op,
    sc_op, swc1_op, swc2_op, swc3_op,
    sdc1_op, sdc2_op, sdc3_op;

extern PPFPI
  /* normal opcodes */
  reserved_op,
  nop_op, b_op, beq0_op, bne0_op, li_op, move_op, move0_op, addiu_xx_op,
  sp_over_op,
  j_op, jal_op,
  beq_op, bne_op, blez_op, bgtz_op,
  addi_op, addiu_op, slti_op, sltiu_op,
  andi_op, ori_op, xori_op, lui_op,
  cop0_op, cop1_op, cop2_op, cop3_op,
  beql_op, bnel_op, blezl_op, bgtzl_op,
  /* special opcodes */
  sll_op, srl_op, sra_op,
  sllv_op, srlv_op, srav_op, jr_op,
  jalr_op, syscall_op, break_op, sync_op,
  mfhi_op, mthi_op, mflo_op, mtlo_op,
  mult_op, multu_op, div_op, divu_op,
  add_op, addu_op, sub_op, subu_op,
  and_op, or_op, xor_op, nor_op,
  slt_op, sltu_op, tge_op, tgeu_op,
  tlt_op, tltu_op, teq_op, tne_op,
  /* regimm opcodes */
  bltz_op, bgez_op,
  bltzl_op, bgezl_op, tgei_op, tgeiu_op,
  tlti_op, tltiu_op, teqi_op, tnei_op,
  bltzal_op, bgezal_op, bltzall_op, bgezall_op,
  /* coprocessor1 functions */
  /* single precision */
  c1_add_s, c1_sub_s, c1_mul_s, c1_div_s,
  c1_sqrt_s, c1_abs_s, c1_mov_s, c1_neg_s,
  c1_round_w_s, c1_trunc_w_s, c1_ceil_w_s, c1_floor_w_s,
  c1_cvt_d_s, c1_cvt_w_s,
  c1_c_f_s, c1_c_un_s, c1_c_eq_s, c1_c_ueq_s,
  c1_c_olt_s, c1_c_ult_s, c1_c_ole_s, c1_c_ule_s,
  c1_c_sf_s, c1_c_ngle_s, c1_c_seq_s, c1_c_ngl_s,
  c1_c_lt_s, c1_c_nge_s, c1_c_le_s, c1_c_ngt_s,
  /* double precision */
  c1_add_d, c1_sub_d, c1_mul_d, c1_div_d,
  c1_sqrt_d, c1_abs_d, c1_mov_d, c1_neg_d,
  c1_round_w_d, c1_trunc_w_d, c1_ceil_w_d, c1_floor_w_d,
  c1_cvt_s_d, c1_cvt_w_d,
  c1_c_f_d, c1_c_un_d, c1_c_eq_d, c1_c_ueq_d,
  c1_c_olt_d, c1_c_ult_d, c1_c_ole_d, c1_c_ule_d,
  c1_c_sf_d, c1_c_ngle_d, c1_c_seq_d, c1_c_ngl_d,
  c1_c_lt_d, c1_c_nge_d, c1_c_le_d, c1_c_ngt_d,
  /* fixed-point precision */
  c1_cvt_s_w, c1_cvt_d_w,
  /* coprocessor opcodes */
  mfc0_op, mfc1_op, mfc2_op, mfc3_op,
  mtc0_op, mtc1_op, mtc2_op, mtc3_op,
  cfc0_op, cfc1_op, cfc2_op, cfc3_op,
  ctc0_op, ctc1_op, ctc2_op, ctc3_op,
  bc0f_op, bc0t_op, bc0fl_op, bc0tl_op,
  bc1f_op, bc1t_op, bc1fl_op, bc1tl_op,
  bc2f_op, bc2t_op, bc2fl_op, bc2tl_op,
  bc3f_op, bc3t_op, bc3fl_op, bc3tl_op,
  /* user defined */
  swallow_op, spawn_op, call_op
#if (defined TLS)
  ,
  // TLS user-defined opcodes
  aspectReductionBegin_op,
  aspectReductionEnd_op,
  aspectAtomicBegin_op,
  aspectAcquireBegin_op,
  aspectAcquireRetry_op,
  aspectAcquireExit_op,
    aspectAcquire2Release_op,
  aspectReleaseBegin_op,
  aspectReleaseEnter_op,
  aspectAtomicEnd_op
#endif
  ;

extern opnum_t normal_opnums[];
extern opnum_t special_opnums[];
extern opnum_t regimm_opnums[];
extern opnum_t user_opnums[];
#if (defined TLS)
extern opnum_t tls_opnums[];
#endif
extern opnum_t cop1func_opnums[][64];
extern opnum_t mfc_opnums[];
extern opnum_t mtc_opnums[];
extern opnum_t cfc_opnums[];
extern opnum_t ctc_opnums[];
extern opnum_t bc_opnums[][4];
extern struct op_desc desc_table[];
extern struct op_desc reserved_ops;
extern struct op_desc nop_ops;

#endif /* OPCODES_H */
