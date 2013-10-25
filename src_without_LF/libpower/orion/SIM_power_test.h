#ifdef __cplusplus
extern "C" {
#endif
/* anything needed to perform computation out of Liberty */
#ifndef	_SIM_POWER_TEST_H
#define	_SIM_POWER_TEST_H

#include <unistd.h>

#define __INSTANCE__ mainpe__power
#define GLOBDEF(t,n) t mainpe__power___ ## n
#define GLOB(n) mainpe__power___ ## n
#define FUNC(n, args...) mainpe__power___ ## n (args)
#define FUNCPTR(n)  mainpe__power___ ## n 
#define PARM(x) PARM_ ## x

#undef PARM_AF
#undef PARM_MAXN
#undef PARM_MAXSUBARRAYS
#undef PARM_MAXSPD
#undef PARM_VTHSENSEEXTDRV
#undef PARM_VTHOUTDRNOR
#undef PARM_res_fpalu
#undef PARM_VTHCOMPINV
#undef PARM_MD_NUM_IREGS
#undef PARM_die_length
#undef PARM_BITOUT
#undef PARM_Cndiffside
#undef PARM_ruu_decode_width
#undef PARM_ruu_issue_width
#undef PARM_amp_Idsat
#undef PARM_AF_TYPE
#undef PARM_VSINV
#undef PARM_Cpdiffovlp
#undef PARM_data_width
#undef PARM_Cgatepass
#undef PARM_Cpdiffarea
#undef PARM_GEN_POWER_FACTOR
#undef PARM_res_memport
#undef PARM_VTHNAND60x90
#undef PARM_Cpdiffside
#undef PARM_Cpoxideovlp
#undef PARM_opcode_length
#undef PARM_MD_NUM_FREGS
#undef PARM_FUDGEFACTOR
#undef PARM_ruu_commit_width
#undef PARM_Cndiffovlp
#undef PARM_VTHOUTDRIVE
#undef PARM_Cndiffarea
#undef PARM_VTHMUXDRV1
#undef PARM_inst_length
#undef PARM_VTHMUXDRV2
#undef PARM_NORMALIZE_SCALE
#undef PARM_ras_size
#undef PARM_VTHMUXDRV3
#undef PARM_ADDRESS_BITS
#undef PARM_RUU_size
#undef PARM_Cgate
#undef PARM_VTHNOR12x4x1
#undef PARM_VTHNOR12x4x2
#undef PARM_VTHOUTDRINV
#undef PARM_VTHNOR12x4x3
#undef PARM_VTHEVALINV
#undef PARM_crossover_scaling
#undef PARM_VTHNOR12x4x4
#undef PARM_turnoff_factor
#undef PARM_res_ialu
#undef PARM_Cnoxideovlp
#undef PARM_VTHOUTDRNAND
#undef PARM_VTHINV100x60
#undef PARM_LSQ_size

#ifndef PARM_AF
#define PARM_AF (5.000000e-01)
#endif /* PARM_AF */
#ifndef PARM_MAXN
#define PARM_MAXN (8)
#endif /* PARM_MAXN */
#ifndef PARM_MAXSUBARRAYS
#define PARM_MAXSUBARRAYS (8)
#endif /* PARM_MAXSUBARRAYS */
#ifndef PARM_MAXSPD
#define PARM_MAXSPD (8)
#endif /* PARM_MAXSPD */
#ifndef PARM_VTHSENSEEXTDRV
#define PARM_VTHSENSEEXTDRV (4.370000e-01)
#endif /* PARM_VTHSENSEEXTDRV */
#ifndef PARM_VTHOUTDRNOR
#define PARM_VTHOUTDRNOR (4.310000e-01)
#endif /* PARM_VTHOUTDRNOR */
#ifndef PARM_res_fpalu
#define PARM_res_fpalu (4)
#endif /* PARM_res_fpalu */
#ifndef PARM_VTHCOMPINV
#define PARM_VTHCOMPINV (4.370000e-01)
#endif /* PARM_VTHCOMPINV */
#ifndef PARM_MD_NUM_IREGS
#define PARM_MD_NUM_IREGS (32)
#endif /* PARM_MD_NUM_IREGS */
#ifndef PARM_die_length
#define PARM_die_length (1.800000e-02)
#endif /* PARM_die_length */
#ifndef PARM_BITOUT
#define PARM_BITOUT (64)
#endif /* PARM_BITOUT */
#ifndef PARM_Cndiffside
#define PARM_Cndiffside (2.750000e-16)
#endif /* PARM_Cndiffside */
#ifndef PARM_ruu_decode_width
#define PARM_ruu_decode_width (4)
#endif /* PARM_ruu_decode_width */
#ifndef PARM_ruu_issue_width
#define PARM_ruu_issue_width (4)
#endif /* PARM_ruu_issue_width */
#ifndef PARM_amp_Idsat
#define PARM_amp_Idsat (5.000000e-04)
#endif /* PARM_amp_Idsat */
#ifndef PARM_AF_TYPE
#define PARM_AF_TYPE (1)
#endif /* PARM_AF_TYPE */
#ifndef PARM_VSINV
#define PARM_VSINV (4.560000e-01)
#endif /* PARM_VSINV */
#ifndef PARM_Cpdiffovlp
#define PARM_Cpdiffovlp (1.380000e-16)
#endif /* PARM_Cpdiffovlp */
#ifndef PARM_Cgatepass
#define PARM_Cgatepass (1.450000e-15)
#endif /* PARM_Cgatepass */
#ifndef PARM_Cpdiffarea
#define PARM_Cpdiffarea (3.430000e-16)
#endif /* PARM_Cpdiffarea */
#ifndef PARM_GEN_POWER_FACTOR
#define PARM_GEN_POWER_FACTOR (1.310000e+00)
#endif /* PARM_GEN_POWER_FACTOR */
#ifndef PARM_res_memport
#define PARM_res_memport (2)
#endif /* PARM_res_memport */
#ifndef PARM_VTHNAND60x90
#define PARM_VTHNAND60x90 (5.610000e-01)
#endif /* PARM_VTHNAND60x90 */
#ifndef PARM_Cpdiffside
#define PARM_Cpdiffside (2.750000e-16)
#endif /* PARM_Cpdiffside */
#ifndef PARM_Cpoxideovlp
#define PARM_Cpoxideovlp (3.380000e-16)
#endif /* PARM_Cpoxideovlp */
#ifndef PARM_opcode_length
#define PARM_opcode_length (8)
#endif /* PARM_opcode_length */
#ifndef PARM_MD_NUM_FREGS
#define PARM_MD_NUM_FREGS (32)
#endif /* PARM_MD_NUM_FREGS */
#ifndef PARM_FUDGEFACTOR
#define PARM_FUDGEFACTOR (1.000000e+00)
#endif /* PARM_FUDGEFACTOR */
#ifndef PARM_ruu_commit_width
#define PARM_ruu_commit_width (4)
#endif /* PARM_ruu_commit_width */
#ifndef PARM_Cndiffovlp
#define PARM_Cndiffovlp (1.380000e-16)
#endif /* PARM_Cndiffovlp */
#ifndef PARM_VTHOUTDRIVE
#define PARM_VTHOUTDRIVE (4.250000e-01)
#endif /* PARM_VTHOUTDRIVE */
#ifndef PARM_Cndiffarea
#define PARM_Cndiffarea (1.370000e-16)
#endif /* PARM_Cndiffarea */
#ifndef PARM_VTHMUXDRV1
#define PARM_VTHMUXDRV1 (4.370000e-01)
#endif /* PARM_VTHMUXDRV1 */
#ifndef PARM_inst_length
#define PARM_inst_length (32)
#endif /* PARM_inst_length */
#ifndef PARM_VTHMUXDRV2
#define PARM_VTHMUXDRV2 (4.860000e-01)
#endif /* PARM_VTHMUXDRV2 */
#ifndef PARM_NORMALIZE_SCALE
#define PARM_NORMALIZE_SCALE (6.488730e-10)
#endif /* PARM_NORMALIZE_SCALE */
#ifndef PARM_ras_size
#define PARM_ras_size (8)
#endif /* PARM_ras_size */
#ifndef PARM_VTHMUXDRV3
#define PARM_VTHMUXDRV3 (4.370000e-01)
#endif /* PARM_VTHMUXDRV3 */
#ifndef PARM_ADDRESS_BITS
#define PARM_ADDRESS_BITS (64)
#endif /* PARM_ADDRESS_BITS */
#ifndef PARM_RUU_size
#define PARM_RUU_size (16)
#endif /* PARM_RUU_size */
#ifndef PARM_Cgate
#define PARM_Cgate (1.950000e-15)
#endif /* PARM_Cgate */
#ifndef PARM_VTHNOR12x4x1
#define PARM_VTHNOR12x4x1 (5.030000e-01)
#endif /* PARM_VTHNOR12x4x1 */
#ifndef PARM_VTHNOR12x4x2
#define PARM_VTHNOR12x4x2 (4.520000e-01)
#endif /* PARM_VTHNOR12x4x2 */
#ifndef PARM_VTHOUTDRINV
#define PARM_VTHOUTDRINV (4.370000e-01)
#endif /* PARM_VTHOUTDRINV */
#ifndef PARM_VTHNOR12x4x3
#define PARM_VTHNOR12x4x3 (4.170000e-01)
#endif /* PARM_VTHNOR12x4x3 */
#ifndef PARM_VTHEVALINV
#define PARM_VTHEVALINV (2.670000e-01)
#endif /* PARM_VTHEVALINV */
#ifndef PARM_crossover_scaling
#define PARM_crossover_scaling (1.200000e+00)
#endif /* PARM_crossover_scaling */
#ifndef PARM_VTHNOR12x4x4
#define PARM_VTHNOR12x4x4 (3.900000e-01)
#endif /* PARM_VTHNOR12x4x4 */
#ifndef PARM_turnoff_factor
#define PARM_turnoff_factor (1.000000e-01)
#endif /* PARM_turnoff_factor */
#ifndef PARM_res_ialu
#define PARM_res_ialu (4)
#endif /* PARM_res_ialu */
#ifndef PARM_Cnoxideovlp
#define PARM_Cnoxideovlp (2.630000e-16)
#endif /* PARM_Cnoxideovlp */
#ifndef PARM_VTHOUTDRNAND
#define PARM_VTHOUTDRNAND (4.410000e-01)
#endif /* PARM_VTHOUTDRNAND */
#ifndef PARM_VTHINV100x60
#define PARM_VTHINV100x60 (4.380000e-01)
#endif /* PARM_VTHINV100x60 */
#ifndef PARM_LSQ_size
#define PARM_LSQ_size (8)
#endif /* PARM_LSQ_size */

#endif /* _SIM_POWER_TEST_H */
#ifdef __cplusplus
}
#endif
