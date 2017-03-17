dnl Common m4 macros used between sha256 and sha512
dnl
dnl Comments relevant to the .s output file should start with "#"
dnl Comments only for thie m4 file should start with "dnl"
divert(-1)
changequote(<,>)dnl

dnl Defining register names
define(<r0>, <0>)
define(<r1>, <1>)
define(<r2>, <2>)
define(<r3>, <3>)
define(<r4>, <4>)
define(<r5>, <5>)
define(<r6>, <6>)
define(<r7>, <7>)
define(<r8>, <8>)
define(<r9>, <9>)
define(<r10>, <10>)
define(<r11>, <11>)
define(<r12>, <12>)
define(<r13>, <13>)
define(<r14>, <14>)
define(<r15>, <15>)
define(<r16>, <16>)
define(<r17>, <17>)
define(<r18>, <18>)
define(<r19>, <19>)
define(<r20>, <20>)
define(<r21>, <21>)
define(<r22>, <22>)
define(<r23>, <23>)
define(<r24>, <24>)
define(<r25>, <25>)
define(<r26>, <26>)
define(<r27>, <27>)
define(<r28>, <28>)
define(<r29>, <29>)
define(<r30>, <30>)
define(<r31>, <31>)
define(<v0>, <0>)
define(<v1>, <1>)
define(<v2>, <2>)
define(<v3>, <3>)
define(<v4>, <4>)
define(<v5>, <5>)
define(<v6>, <6>)
define(<v7>, <7>)
define(<v8>, <8>)
define(<v9>, <9>)
define(<v10>, <10>)
define(<v11>, <11>)
define(<v12>, <12>)
define(<v13>, <13>)
define(<v14>, <14>)
define(<v15>, <15>)
define(<v16>, <16>)
define(<v17>, <17>)
define(<v18>, <18>)
define(<v19>, <19>)
define(<v20>, <20>)
define(<v21>, <21>)
define(<v22>, <22>)
define(<v23>, <23>)
define(<v24>, <24>)
define(<v25>, <25>)
define(<v26>, <26>)
define(<v27>, <27>)
define(<v28>, <28>)
define(<v29>, <29>)
define(<v30>, <30>)
define(<v31>, <31>)

dnl Now the VSX registers which are related to the altivec registers
define(<vs32>, <32>)
define(<vs33>, <33>)
define(<vs34>, <34>)
define(<vs35>, <35>)
define(<vs36>, <36>)
define(<vs37>, <37>)
define(<vs38>, <38>)
define(<vs39>, <39>)
define(<vs40>, <40>)
define(<vs41>, <41>)
define(<vs42>, <42>)
define(<vs43>, <43>)
define(<vs44>, <44>)
define(<vs45>, <45>)
define(<vs46>, <46>)
define(<vs47>, <47>)
define(<vs48>, <48>)
define(<vs49>, <49>)
define(<vs50>, <50>)
define(<vs51>, <51>)
define(<vs52>, <52>)
define(<vs53>, <53>)
define(<vs54>, <54>)
define(<vs55>, <55>)
define(<vs56>, <56>)
define(<vs57>, <57>)
define(<vs58>, <58>)
define(<vs59>, <59>)
define(<vs60>, <60>)
define(<vs61>, <61>)
define(<vs62>, <62>)
define(<vs63>, <63>)

dnl Pseudo ops

dnl Argument to ALIGN is always in bytes, and converted to a logarithmic .align
dnl if necessary.
define(<m4_log2>, <m4_log2_internal($1,1,0) >)
define(<m4_log2_internal>,
  <ifelse($3, 10, <not-a-power-of-two>,
  $1, $2, $3,
  <m4_log2_internal($1, eval(2*$2), eval(1 + $3))>)
>)
define(<ALIGN>, <.align m4_log2($1) >)

dnl PROLOGUE(function_name, gpr_used, vr_used)
define(<PROLOGUE>,
<.globl $1
.type $1,%function
$1:
SAVE_NVOLATILE($2,$3)>)

dnl EPILOGUE(function_name, gpr_used, vr_used)
define(<EPILOGUE>,
RESTORE_NVOLATILE($2,$3)
<  blr
.size $1, . - $1>)

dnl Ceiling operation of $1 to $2.
dnl E.g: ceil(16,16) => 16; ceil(17,16) => 32;
define(<ceil>, <eval(((((8*$1)-1)/$2)+1)*$2)>)

dnl Parameter of std/ld to the location where gpr $1 should be saved
define(<gpr_loc>, <r$1, -eval(8*($1-13))(r1)
>)

dnl To save vectors, memory should be 16 bytes aligned.
dnl Uses stvx to save vr $1 after saving $2 gprs
define(<vr_save>, <li r0, -eval(ceil($2, 16) + 16*($1-19)); stvx v$1, r1, r0
>)
dnl Uses lvx to loads vr $1 after loading $2 gprs
define(<vr_load>, <li r0, -eval(ceil($2, 16) + 16*($1-19)); lvx v$1, r1, r0
>)

dnl SAVE_NVOLATILE(gpr_used, vr_used)
define(<SAVE_NVOLATILE>, <
  changequote([,])dnl
  # Saving non volatile registers
  ifelse(<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ignored; only for balancing)dnl
  dnl Save r14-r31 and vr20-vr31 in the red zone, if needed
  dnl Saving GP regs first and then Vectors
  ifelse(eval($1 > 17), 1, [ std gpr_loc(31) ])dnl
  ifelse(eval($1 > 16), 1, [ std gpr_loc(30) ])dnl
  ifelse(eval($1 > 15), 1, [ std gpr_loc(29) ])dnl
  ifelse(eval($1 > 14), 1, [ std gpr_loc(28) ])dnl
  ifelse(eval($1 > 13), 1, [ std gpr_loc(27) ])dnl
  ifelse(eval($1 > 12), 1, [ std gpr_loc(26) ])dnl
  ifelse(eval($1 > 11), 1, [ std gpr_loc(25) ])dnl
  ifelse(eval($1 > 10), 1, [ std gpr_loc(24) ])dnl
  ifelse(eval($1 >  9), 1, [ std gpr_loc(23) ])dnl
  ifelse(eval($1 >  8), 1, [ std gpr_loc(22) ])dnl
  ifelse(eval($1 >  7), 1, [ std gpr_loc(21) ])dnl
  ifelse(eval($1 >  6), 1, [ std gpr_loc(20) ])dnl
  ifelse(eval($1 >  5), 1, [ std gpr_loc(19) ])dnl
  ifelse(eval($1 >  4), 1, [ std gpr_loc(18) ])dnl
  ifelse(eval($1 >  3), 1, [ std gpr_loc(17) ])dnl
  ifelse(eval($1 >  2), 1, [ std gpr_loc(16) ])dnl
  ifelse(eval($1 >  1), 1, [ std gpr_loc(15) ])dnl
  ifelse(eval($1 >  0), 1, [ std gpr_loc(14) ])dnl

  ifelse(eval($2 > 11), 1, [ vr_save(31, $1) ])dnl
  ifelse(eval($2 > 10), 1, [ vr_save(30, $1) ])dnl
  ifelse(eval($2 >  9), 1, [ vr_save(29, $1) ])dnl
  ifelse(eval($2 >  8), 1, [ vr_save(28, $1) ])dnl
  ifelse(eval($2 >  7), 1, [ vr_save(27, $1) ])dnl
  ifelse(eval($2 >  6), 1, [ vr_save(26, $1) ])dnl
  ifelse(eval($2 >  5), 1, [ vr_save(25, $1) ])dnl
  ifelse(eval($2 >  4), 1, [ vr_save(24, $1) ])dnl
  ifelse(eval($2 >  3), 1, [ vr_save(23, $1) ])dnl
  ifelse(eval($2 >  2), 1, [ vr_save(22, $1) ])dnl
  ifelse(eval($2 >  1), 1, [ vr_save(21, $1) ])dnl
  ifelse(eval($2 >  0), 1, [ vr_save(20, $1) ])dnl
  changequote(<,>)dnl
>)

dnl RESTORE_NVOLATILE(gp_used, vr_used)
define(<RESTORE_NVOLATILE>, <
  changequote([,])dnl
  # Restoring non volatile registers
  ifelse(<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ignored; only for balancing)

  dnl Restore r14-r31 and vr20-vr31 if needed
  dnl Restore GP regs first and then Vectors
  ifelse(eval($1 > 17), 1, [ ld gpr_loc(31) ])dnl
  ifelse(eval($1 > 16), 1, [ ld gpr_loc(30) ])dnl
  ifelse(eval($1 > 15), 1, [ ld gpr_loc(29) ])dnl
  ifelse(eval($1 > 14), 1, [ ld gpr_loc(28) ])dnl
  ifelse(eval($1 > 13), 1, [ ld gpr_loc(27) ])dnl
  ifelse(eval($1 > 12), 1, [ ld gpr_loc(26) ])dnl
  ifelse(eval($1 > 11), 1, [ ld gpr_loc(25) ])dnl
  ifelse(eval($1 > 10), 1, [ ld gpr_loc(24) ])dnl
  ifelse(eval($1 >  9), 1, [ ld gpr_loc(23) ])dnl
  ifelse(eval($1 >  8), 1, [ ld gpr_loc(22) ])dnl
  ifelse(eval($1 >  7), 1, [ ld gpr_loc(21) ])dnl
  ifelse(eval($1 >  6), 1, [ ld gpr_loc(20) ])dnl
  ifelse(eval($1 >  5), 1, [ ld gpr_loc(19) ])dnl
  ifelse(eval($1 >  4), 1, [ ld gpr_loc(18) ])dnl
  ifelse(eval($1 >  3), 1, [ ld gpr_loc(17) ])dnl
  ifelse(eval($1 >  2), 1, [ ld gpr_loc(16) ])dnl
  ifelse(eval($1 >  1), 1, [ ld gpr_loc(15) ])dnl
  ifelse(eval($1 >  0), 1, [ ld gpr_loc(14) ])dnl

  ifelse(eval($2 > 11), 1, [ vr_load(31, $1) ])dnl
  ifelse(eval($2 > 10), 1, [ vr_load(30, $1) ])dnl
  ifelse(eval($2 >  9), 1, [ vr_load(29, $1) ])dnl
  ifelse(eval($2 >  8), 1, [ vr_load(28, $1) ])dnl
  ifelse(eval($2 >  7), 1, [ vr_load(27, $1) ])dnl
  ifelse(eval($2 >  6), 1, [ vr_load(26, $1) ])dnl
  ifelse(eval($2 >  5), 1, [ vr_load(25, $1) ])dnl
  ifelse(eval($2 >  4), 1, [ vr_load(24, $1) ])dnl
  ifelse(eval($2 >  3), 1, [ vr_load(23, $1) ])dnl
  ifelse(eval($2 >  2), 1, [ vr_load(22, $1) ])dnl
  ifelse(eval($2 >  1), 1, [ vr_load(21, $1) ])dnl
  ifelse(eval($2 >  0), 1, [ vr_load(20, $1) ])dnl

  changequote(<,>)dnl
>)

divert(1)
