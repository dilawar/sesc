#define tt6_isFlowAltering(majorOp, minorOp) (			\
(majorOp==18) ||                /* b, ba, bl, bla */		\
(majorOp==16) ||                /* bc, bca, bcl, bcla */	\
(majorOp==19 && minorOp==528) ||/* bcctr, bcctrl */			\
(majorOp==19 && minorOp==16)  ||/* bclr, bclrl*/    \
(majorOp==19 && minorOp==50)  ||    /* rfi */				\
(majorOp==17) ||               /* sc */					\
(majorOp==19 && minorOp==18)/* PPC64: rfid */				\
  )


#define tt6_isMemory(majorOp, minorOp) ( \
    (majorOp==34) ||/* lbz*/		 \
      (majorOp==35) ||/* lbzu*/		 \
      (majorOp==50) ||/* lfd*/		 \
      (majorOp==51) ||/* lfdu*/		 \
      (majorOp==48) ||/* lfs*/		 \
      (majorOp==49) ||/* lfsu*/		 \
      (majorOp==42) ||/* lha*/		 \
      (majorOp==43) ||/* lhau*/		 \
      (majorOp==40) ||/* lhz*/		 \
      (majorOp==41) ||/* lhzu*/		 \
      (majorOp==46) ||/* lmw*/		 \
      (majorOp==32) ||/* lwz*/		 \
      (majorOp==33) ||/* lwzu*/			\
      (majorOp==31 && minorOp==597) ||/* lswi*/	\
      (majorOp==31 && minorOp==119) ||/* lbzux */	\
      (majorOp==31 && minorOp==87)  ||/* lbzx*/		\
      (majorOp==31 && minorOp==631) ||/* lfdux */	\
      (majorOp==31 && minorOp==599) ||/* lfdx*/		\
      (majorOp==31 && minorOp==567) ||/* lfsux */	\
      (majorOp==31 && minorOp==535) ||/* lfsx*/		\
      (majorOp==31 && minorOp==375) ||/* lhaux */	\
      (majorOp==31 && minorOp==343) ||/* lhax*/		\
      (majorOp==31 && minorOp==790) ||/* lhbrx */	\
      (majorOp==31 && minorOp==311) ||/* lhzux */	\
      (majorOp==31 && minorOp==279) ||/* lhzx*/		\
      (majorOp==31 && minorOp==597) ||/* lswi*/		\
      (majorOp==31 && minorOp==20)  ||/* lwarx */	\
      (majorOp==31 && minorOp==534) ||/* lwbrx */	\
      (majorOp==31 && minorOp==55)  ||/* lwzux */	\
      (majorOp==31 && minorOp==23)  ||/* lwzx*/		\
      (majorOp==31 && minorOp==7)   ||/* lvebx */	\
      (majorOp==31 && minorOp==39)  ||/* lvehx */	\
      (majorOp==31 && minorOp==71)  ||/* lvewx */	\
      (majorOp==31 && minorOp==103) ||/* lvx*/		\
      (majorOp==31 && minorOp==359) ||    /* lvxl*/			\
      (majorOp==31 && minorOp==310) ||/* eciwx */			\
      (majorOp==31 && minorOp==438) ||/* ecowx */     \
(majorOp==38) ||/* stb*/						\
(majorOp==39) ||/* stbu*/						\
(majorOp==54) ||/* stfd*/						\
(majorOp==55) ||/* stfdu */						\
(majorOp==52) ||/* stfs*/						\
(majorOp==53) ||/* stfsu */						\
(majorOp==44) ||/* sth*/						\
(majorOp==45) ||/* sthu*/						\
(majorOp==47) ||/* stmw*/      \
(majorOp==36) ||/* stw*/			\
(majorOp==37) ||/* stwu*/      \
(majorOp==31 && minorOp==247) ||/* stbux */				\
(majorOp==31 && minorOp==215) ||/* stbx*/				\
(majorOp==31 && minorOp==759) ||/* stfdux */				\
(majorOp==31 && minorOp==727) ||/* stfdx */				\
(majorOp==31 && minorOp==983) ||/* stfiwx */				\
(majorOp==31 && minorOp==695) ||/* stfsux */    \
(majorOp==31 && minorOp==663) ||/* stfsx */				\
(majorOp==31 && minorOp==918) ||/* sthbrx */				\
(majorOp==31 && minorOp==439) ||/* sthux */	\
(majorOp==31 && minorOp==407) ||/* sthx*/	\
(majorOp==31 && minorOp==725) ||/* stswi */	\
(majorOp==31 && minorOp==662) ||/* stwbrx */    \
(majorOp==31 && minorOp==150) ||/* stwcx. */    \
(majorOp==31 && minorOp==183) ||/* stwux */ \
(majorOp==31 && minorOp==151) ||/* stwx*/		\
(majorOp==31 && minorOp==135) ||/* stvebx */    \
(majorOp==31 && minorOp==167) ||/* stvehx */		\
(majorOp==31 && minorOp==199) ||/* stvewx */    \
(majorOp==31 && minorOp==231) ||/* stvx*/	\
(majorOp==31 && minorOp==487) ||/* stvxl */	\
(majorOp==58) ||/* PPC64: ld, ldu, lwa*/	\
(majorOp==62) ||/* PPC64: std, stdu*/		\
(majorOp==31 && minorOp==21) ||/* PPC64: ldx*/	  \
(majorOp==31 && minorOp==53) ||/* PPC64: ldux*/	  \
(majorOp==31 && minorOp==84) ||/* PPC64: ldarx*/  \
(majorOp==31 && minorOp==341) ||/* PPC64: lwax*/  \
(majorOp==31 && minorOp==373) ||/* PPC64: lwaux*/ \
(majorOp==31 && minorOp==149) ||/* PPC64: stdx*/  \
(majorOp==31 && minorOp==181) ||/* PPC64: stdux*/ \
(majorOp==31 && minorOp==214)/* PPC64: stdcx.*/	  \
    )

#define tt6_isMemoryExtended(majorOp, minorOp) ( \
    (majorOp==31 && minorOp==533) ||/* lswx  emit XER */		\
  (majorOp==31 && minorOp==661) ||    /* stswx  emit XER */		\
  (majorOp==31 && minorOp==342) ||/* dst, dstt  emit rB */    \
(majorOp==31 && minorOp==374) /* dstst, dststt  emit rB */	\
     )
