
#include <stdint.h>
#include <string.h>

#include "icode.h"
#include "opcodes.h"

void icode::dump()
{
  printf("icode 0x%p:\n", this);
  printf("  next: 0x%p\n", next);
  printf("  addr: 0x%x\n", addr);
  printf("  func: 0x%p\n", func);
  printf("  instr: 0x%x\n", instr);
  /*CONSTCOND*/
  printf("  rs: 0x%x (r%d) (fmt %d)\n",
         args[RS], getRN(RS), args[RS]);
  /*CONSTCOND*/
  printf("  rt: 0x%x (r%d) (ft $f%d)\n",
         args[RT], getRN(RT), getFPN(ICODEFT));
  /*CONSTCOND*/
  printf("  rd: 0x%x (r%d) (fs $f%d)\n",
         args[RD], getRN(RD), getFPN(ICODEFS));
  printf("  sa: 0x%x (fd $f%d)\n", args[SA], getFPN(ICODEFD));
  printf("  immed: 0x%x (%d)\n", immed, immed);
  printf("  target: 0x%p\n", (void*) target);
}

const char *icode::dis_instr()
{
  const char *opname;
  struct op_desc *pdesc;
  static char buf[1024];
  static char *str;
  
  pdesc = &desc_table[opnum];
  opname = pdesc->opname;
  str = buf;
  
  sprintf(str, "0x%x: %08x\t"
          ,(unsigned) addr, (unsigned) instr);
  str += strlen(str);
  switch(opnum) {
  case move_opn:
  case addu_opn:
    if (args[RT] == 0)
      /*CONSTCOND*/
      sprintf(str, "move\tr%d, r%d",
              getRN(RD), getRN(RS));
    else
      /*CONSTCOND*/
      sprintf(str, "addu\tr%d, r%d, r%d",
              getRN(RD), getRN(RS), getRN(RT));
    break;
  case add_opn:
  case and_opn:
  case nor_opn:
  case or_opn:
  case sllv_opn:
  case slt_opn:
  case sltu_opn:
  case srlv_opn:
  case srav_opn:
  case sub_opn:
  case subu_opn:
  case xor_opn:
    /*CONSTCOND*/
    sprintf(str, "%s\tr%d, r%d, r%d",
            opname, getRN(RD), getRN(RS), getRN(RT));
    break;
  case li_opn:
  case addiu_opn:
    if (args[RS] == 0)
      /*CONSTCOND*/
      sprintf(str, "li\tr%d, %d",
              getRN(RT), immed);
    else
      /*CONSTCOND*/
      sprintf(str, "addiu\tr%d, r%d, %d",
              getRN(RT), getRN(RS), immed);
    break;
  case addi_opn:
  case slti_opn:
    /*CONSTCOND*/
    sprintf(str, "%s\tr%d, r%d, %d",
            opname, getRN(RT), getRN(RS), immed);
    break;
  case sltiu_opn:
    /*CONSTCOND*/
    sprintf(str, "%s\tr%d, r%d, %u",
            opname, getRN(RT), getRN(RS),
            (unsigned) immed);
    break;
  case andi_opn:
  case ori_opn:
  case xori_opn:
    /*CONSTCOND*/
    sprintf(str, "%s\tr%d, r%d, 0x%x",
            opname, getRN(RT), getRN(RS),
            (uint16_t) immed);
    break;
  case beq_opn:
  case b_opn:
    if (args[RS] == 0 && args[RT] == 0)
      sprintf(str, "b\t0x%x",
              (unsigned) (addr + (immed << 2) + 4));
    else
      /*CONSTCOND*/
      sprintf(str, "beq\tr%d, r%d, 0x%x",
              getRN(RS), getRN(RT),
              (unsigned)  (addr + (immed << 2) + 4));
    break;
  case beql_opn:
  case bne_opn:
  case bnel_opn:
    /*CONSTCOND*/
    sprintf(str, "%s\tr%d, r%d 0x%x", opname,
            getRN(RS), getRN(RT),
            (unsigned)  (addr + 4 + (immed << 2)));
    break;
  case bgez_opn:
  case bgezal_opn:
  case bgezall_opn:
  case bgezl_opn:
  case bgtz_opn:
  case bgtzl_opn:
  case blez_opn:
  case blezl_opn:
  case bltz_opn:
  case bltzal_opn:
  case bltzall_opn:
  case bltzl_opn:
    /*CONSTCOND*/
    sprintf(str, "%s\tr%d, 0x%x", opname,
            getRN(RS),
            (unsigned)  (addr + 4 + (immed << 2)));
    break;
  case break_opn:
  case cache_opn:
    /*CONSTCOND*/
    sprintf(str, "%s\t%d, %d(r%d)",
            opname, getRN(RT), immed, getRN(RS));
    break;
  case cop0_opn:
  case cop1_opn:
  case cop2_opn:
  case cop3_opn:
    sprintf(str, "%s\t", opname);
    break;
  case div_opn:
  case divu_opn:
  case mult_opn:
  case multu_opn:
    /*CONSTCOND*/
    sprintf(str, "%s\tr%d, r%d", opname, getRN(RS), getRN(RT));
    break;
  case j_opn:
  case jal_opn:
    if (target == 0)
      sprintf(str, "%s\t0", opname);
    else
      sprintf(str, "%s\t0x%x", opname,(unsigned)icode2addr(target));
        break;
  case jalr_opn:
    /*CONSTCOND*/
    sprintf(str, "%s\tr%d, r%d", opname, getRN(RD), getRN(RS));
    break;
  case jr_opn:
    /*CONSTCOND*/
    sprintf(str, "%s\tr%d", opname, getRN(RS));
    break;
  case lb_opn:
  case lbu_opn:
  case lh_opn:
  case lhu_opn:
  case ll_opn:
  case lw_opn:
  case lwl_opn:
  case lwr_opn:
  case sb_opn:
  case sc_opn:
  case sh_opn:
  case sw_opn:
  case swl_opn:
  case swr_opn:
    /*CONSTCOND*/
    sprintf(str, "%s\tr%d, %d(r%d)",
            opname, getRN(RT), immed, getRN(RS));
    break;
    
    /* VK: changed FPN to DPN for double precision register load/store */
  case ldc1_opn:
  case ldc2_opn:
  case ldc3_opn:
  case sdc1_opn:
      case sdc2_opn:
  case sdc3_opn:
    /*CONSTCOND*/
    sprintf(str, "%s\t$f%d, %d(r%d)",
            opname, getDPN(RT), immed, getRN(RS));
    break;
  case lwc1_opn:
  case lwc2_opn:
  case lwc3_opn:
  case swc1_opn:
  case swc2_opn:
  case swc3_opn:
    /*CONSTCOND*/
    sprintf(str, "%s\t$f%d, %d(r%d)",
            opname, getFPN(RT), immed, getRN(RS));
    break;
  case lui_opn:
    /*CONSTCOND*/
    sprintf(str, "%s\tr%d, 0x%x",
            opname, getRN(RT), (uint16_t) immed);
    break;
  case mfhi_opn:
  case mflo_opn:
    /*CONSTCOND*/
    sprintf(str, "%s\tr%d", opname, getRN(RD));
    break;
  case mthi_opn:
  case mtlo_opn:
    /*CONSTCOND*/
    sprintf(str, "%s\tr%d", opname, getRN(RS));
    break;
  case nop_opn:
  case sll_opn:
    /*CONSTCOND*/
    if (getRN(RD) == 0 && getRN(RT) == 0 && getRN(SA) == 0)
      sprintf(str, "nop");
    else
      /*CONSTCOND*/
      sprintf(str, "sll\tr%d, r%d, %d",
              getRN(RD), getRN(RT), getRN(SA));
    break;
  case sra_opn:
  case srl_opn:
    /*CONSTCOND*/
    sprintf(str, "%s\tr%d, r%d, %d",
            opname, getRN(RD), getRN(RT), getRN(SA));
    break;
  case sync_opn:
  case syscall_opn:
    sprintf(str, "%s\t", opname);
    break;
  case teq_opn:
  case tge_opn:
  case tgeu_opn:
  case tlt_opn:
  case tltu_opn:
  case tne_opn:
    /*CONSTCOND*/
    sprintf(str, "%s\tr%d, r%d", opname, getRN(RS), getRN(RT));
    break;
  case teqi_opn:
  case tgei_opn:
  case tgeiu_opn:
  case tlti_opn:
  case tltiu_opn:
  case tnei_opn:
    /*CONSTCOND*/
    sprintf(str, "%s\tr%d, %d", opname, getRN(RS), immed);
    break;
    
    /* coprocessor instructions */
  case add_s_opn:
  case sub_s_opn:
  case mul_s_opn:
  case div_s_opn:
    sprintf(str, "%s\t$f%d, $f%d, $f%d",
            opname, getFPN(ICODEFD), getFPN(ICODEFS), getFPN(ICODEFT));
    break;
  case sqrt_s_opn:
  case abs_s_opn:
  case mov_s_opn:
  case neg_s_opn:
  case round_w_s_opn:
  case trunc_w_s_opn:
  case ceil_w_s_opn:
  case floor_w_s_opn:
  case cvt_w_s_opn:
    sprintf(str, "%s\t$f%d, $f%d",
            opname, getFPN(ICODEFD), getFPN(ICODEFS));
    break;
  case cvt_d_s_opn:
    sprintf(str, "%s\t$f%d, $f%d",
               opname, getDPN(ICODEFD), getFPN(ICODEFS));
    break;
  case c_f_s_opn:
  case c_un_s_opn:
  case c_eq_s_opn:
  case c_ueq_s_opn:
  case c_olt_s_opn:
  case c_ult_s_opn:
  case c_ole_s_opn:
  case c_ule_s_opn:
  case c_sf_s_opn:
  case c_ngle_s_opn:
  case c_seq_s_opn:
  case c_ngl_s_opn:
  case c_lt_s_opn:
  case c_nge_s_opn:
  case c_le_s_opn:
  case c_ngt_s_opn:
    sprintf(str, "%s\t$f%d, $f%d",
            opname, getFPN(ICODEFS), getFPN(ICODEFT));
        break;
        /* double precision */
  case add_d_opn:
  case sub_d_opn:
  case mul_d_opn:
  case div_d_opn:
    sprintf(str, "%s\t$f%d, $f%d, $f%d",
            opname, getDPN(ICODEFD), getDPN(ICODEFS), getDPN(ICODEFT));
    break;
  case sqrt_d_opn:
  case abs_d_opn:
  case mov_d_opn:
  case neg_d_opn:
  case round_w_d_opn:
  case trunc_w_d_opn:
  case ceil_w_d_opn:
  case floor_w_d_opn:
    sprintf(str, "%s\t$f%d, $f%d", opname, getDPN(ICODEFD), getDPN(ICODEFS));
    break;
  case cvt_s_d_opn:
  case cvt_w_d_opn:
    sprintf(str, "%s\t$f%d, $f%d",
            opname, getFPN(ICODEFD), getDPN(ICODEFS));
    break;
  case c_f_d_opn:
  case c_un_d_opn:
  case c_eq_d_opn:
  case c_ueq_d_opn:
  case c_olt_d_opn:
  case c_ult_d_opn:
  case c_ole_d_opn:
  case c_ule_d_opn:
  case c_sf_d_opn:
  case c_ngle_d_opn:
  case c_seq_d_opn:
  case c_ngl_d_opn:
  case c_lt_d_opn:
  case c_nge_d_opn:
  case c_le_d_opn:
  case c_ngt_d_opn:
    sprintf(str, "%s\t$f%d, $f%d",
            opname, getDPN(ICODEFS), getDPN(ICODEFT));
    break;
    
    /* fixed-point precision */
  case cvt_s_w_opn:
    sprintf(str, "%s\t$f%d, $f%d",
            opname, getFPN(ICODEFD), getFPN(ICODEFS));
    break;
  case cvt_d_w_opn:
    sprintf(str, "%s\t$f%d, $f%d",
            opname, getDPN(ICODEFD), getFPN(ICODEFS));
    break;
    
    /* other coprocessor instructions */
  case mfc1_opn:
  case mtc1_opn:
    /*CONSTCOND*/
    sprintf(str, "%s\tr%d, $f%d", opname, getRN(RT), getFPN(ICODEFS));
    break;
  case cfc1_opn:
  case ctc1_opn:
    /*CONSTCOND*/
    sprintf(str, "%s\tr%d, $%d",
            opname, getRN(RT), (int) args[RD]);
    break;
    
    /* coprocessor branch codes */
  case bc0f_opn:
  case bc0t_opn:
  case bc0fl_opn:
  case bc0tl_opn:
  case bc1f_opn:
  case bc1t_opn:
  case bc1fl_opn:
  case bc1tl_opn:
  case bc2f_opn:
  case bc2t_opn:
  case bc2fl_opn:
  case bc2tl_opn:
  case bc3f_opn:
  case bc3t_opn:
  case bc3fl_opn:
  case bc3tl_opn:
    sprintf(str, "%s\t0x%x", opname,
            (unsigned)  addr + 4 + (immed << 2));
    break;
    
  case terminate_opn:
    sprintf(str, "terminate()");
    break;
  case reserved_opn:
  case invalid_opn:
  default:
    sprintf(str, "Error: bad instruction.");
    break;
  }
  return buf;
}
