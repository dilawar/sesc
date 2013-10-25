#ifndef SETUP_H
#define SETUP_H
extern double Mhz ;

extern int decode_stages;
extern int rename_stages;
extern int wakeup_stages;
extern int ruu_fetch_width;
extern int ruu_decode_width;
extern int ruu_issue_width;
extern int ruu_commit_width;
extern double areaFactor;
extern double dieLenght;
extern int RUU_size;
extern int REG_size;
extern int LDQ_size;
extern int STQ_size;
extern int data_width;
extern int res_ialu;
extern int res_fpalu;
extern int res_memport;
extern int verbose ;
extern int ras_size ;
extern int rob_size ;

extern struct cache_t *cache_dl1;
extern struct cache_t *cache_il1;
extern struct cache_t *cache_dl2;

extern struct cache_t *dtlb;
extern struct cache_t *itlb;

extern int bimod_config[];
extern int twolev_config[];
extern int comb_config[];
extern int btb_config[];

enum EPREDTYPE{
  ORACLE,
  STATIC,
  BIMOD,
  TWOLEV,
  COMB,
  OTHER
};
extern enum EPREDTYPE bp_type ;

typedef struct {
  double rasPower ;
  double btbPower ;
  double bpredPower ;
}bpower_result_type ;
#endif
