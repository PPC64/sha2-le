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

dnl SAVE_NVOLATILE(gpr_used, vr_used)
define(<SAVE_NVOLATILE>, <
  changequote([,])dnl
  # Saving non volatile registers
  ifelse(<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ignored; only for balancing)dnl
  dnl Save r14-r31 and vr20-vr31 in the red zone, if needed
  dnl Saving GP regs first and then Vectors
  ifelse(eval($1 > 17), 1, [
    std r31, -eval(8*(18))(r1) ])dnl
  ifelse(eval($1 > 16), 1, [
    std r30, -eval(8*(17))(r1) ])dnl
  ifelse(eval($1 > 15), 1, [
    std r29, -eval(8*(16))(r1) ])dnl
  ifelse(eval($1 > 14), 1, [
    std r28, -eval(8*(15))(r1) ])dnl
  ifelse(eval($1 > 13), 1, [
    std r27, -eval(8*(14))(r1) ])dnl
  ifelse(eval($1 > 12), 1, [
    std r26, -eval(8*(13))(r1) ])dnl
  ifelse(eval($1 > 11), 1, [
    std r25, -eval(8*(12))(r1) ])dnl
  ifelse(eval($1 > 10), 1, [
    std r24, -eval(8*(11))(r1) ])dnl
  ifelse(eval($1 >  9), 1, [
    std r23, -eval(8*(10))(r1) ])dnl
  ifelse(eval($1 >  8), 1, [
    std r22, -eval(8*( 9))(r1) ])dnl
  ifelse(eval($1 >  7), 1, [
    std r21, -eval(8*( 8))(r1) ])dnl
  ifelse(eval($1 >  6), 1, [
    std r20, -eval(8*( 7))(r1) ])dnl
  ifelse(eval($1 >  5), 1, [
    std r19, -eval(8*( 6))(r1) ])dnl
  ifelse(eval($1 >  4), 1, [
    std r18, -eval(8*( 5))(r1) ])dnl
  ifelse(eval($1 >  3), 1, [
    std r17, -eval(8*( 4))(r1) ])dnl
  ifelse(eval($1 >  2), 1, [
    std r16, -eval(8*( 3))(r1) ])dnl
  ifelse(eval($1 >  1), 1, [
    std r15, -eval(8*( 2))(r1) ])dnl
  ifelse(eval($1 >  0), 1, [
    std r14, -eval(8*( 1))(r1) ])dnl

  dnl must be 16 bytes aligned: ((((n-1)/16)+1)*16)
  ifelse(eval($2 > 11), 1, [
    li r0, -eval((((((8*$1)-1)/16)+1)*16) + 16*12); stvx v31, r1, r0 ])dnl
  ifelse(eval($2 > 10), 1, [
    li r0, -eval((((((8*$1)-1)/16)+1)*16) + 16*11); stvx v30, r1, r0 ])dnl
  ifelse(eval($2 >  9), 1, [
    li r0, -eval((((((8*$1)-1)/16)+1)*16) + 16*10); stvx v29, r1, r0 ])dnl
  ifelse(eval($2 >  8), 1, [
    li r0, -eval((((((8*$1)-1)/16)+1)*16) + 16* 9); stvx v28, r1, r0 ])dnl
  ifelse(eval($2 >  7), 1, [
    li r0, -eval((((((8*$1)-1)/16)+1)*16) + 16* 8); stvx v27, r1, r0 ])dnl
  ifelse(eval($2 >  6), 1, [
    li r0, -eval((((((8*$1)-1)/16)+1)*16) + 16* 7); stvx v26, r1, r0 ])dnl
  ifelse(eval($2 >  5), 1, [
    li r0, -eval((((((8*$1)-1)/16)+1)*16) + 16* 6); stvx v25, r1, r0 ])dnl
  ifelse(eval($2 >  4), 1, [
    li r0, -eval((((((8*$1)-1)/16)+1)*16) + 16* 5); stvx v24, r1, r0 ])dnl
  ifelse(eval($2 >  3), 1, [
    li r0, -eval((((((8*$1)-1)/16)+1)*16) + 16* 4); stvx v23, r1, r0 ])dnl
  ifelse(eval($2 >  2), 1, [
    li r0, -eval((((((8*$1)-1)/16)+1)*16) + 16* 3); stvx v22, r1, r0 ])dnl
  ifelse(eval($2 >  1), 1, [
    li r0, -eval((((((8*$1)-1)/16)+1)*16) + 16* 2); stvx v21, r1, r0 ])dnl
  ifelse(eval($2 >  0), 1, [
    li r0, -eval((((((8*$1)-1)/16)+1)*16) + 16* 1); stvx v20, r1, r0 ])dnl
  changequote(<,>)dnl
>)

dnl RESTORE_NVOLATILE(gp_used, vr_used)
define(<RESTORE_NVOLATILE>, <
  changequote([,])dnl
  # Restoring non volatile registers
  ifelse(<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ignored; only for balancing)

  dnl Restore r14-r31 and vr20-vr31 if needed
  dnl Restore GP regs first and then Vectors
  ifelse(eval($1 > 17), 1, [
    ld r31, -eval(8*(18))(r1) ])dnl
  ifelse(eval($1 > 16), 1, [
    ld r30, -eval(8*(17))(r1) ])dnl
  ifelse(eval($1 > 15), 1, [
    ld r29, -eval(8*(16))(r1) ])dnl
  ifelse(eval($1 > 14), 1, [
    ld r28, -eval(8*(15))(r1) ])dnl
  ifelse(eval($1 > 13), 1, [
    ld r27, -eval(8*(14))(r1) ])dnl
  ifelse(eval($1 > 12), 1, [
    ld r26, -eval(8*(13))(r1) ])dnl
  ifelse(eval($1 > 11), 1, [
    ld r25, -eval(8*(12))(r1) ])dnl
  ifelse(eval($1 > 10), 1, [
    ld r24, -eval(8*(11))(r1) ])dnl
  ifelse(eval($1 >  9), 1, [
    ld r23, -eval(8*(10))(r1) ])dnl
  ifelse(eval($1 >  8), 1, [
    ld r22, -eval(8*( 9))(r1) ])dnl
  ifelse(eval($1 >  7), 1, [
    ld r21, -eval(8*( 8))(r1) ])dnl
  ifelse(eval($1 >  6), 1, [
    ld r20, -eval(8*( 7))(r1) ])dnl
  ifelse(eval($1 >  5), 1, [
    ld r19, -eval(8*( 6))(r1) ])dnl
  ifelse(eval($1 >  4), 1, [
    ld r18, -eval(8*( 5))(r1) ])dnl
  ifelse(eval($1 >  3), 1, [
    ld r17, -eval(8*( 4))(r1) ])dnl
  ifelse(eval($1 >  2), 1, [
    ld r16, -eval(8*( 3))(r1) ])dnl
  ifelse(eval($1 >  1), 1, [
    ld r15, -eval(8*( 2))(r1) ])dnl
  ifelse(eval($1 >  0), 1, [
    ld r14, -eval(8*( 1))(r1) ])dnl

  dnl must be 16 bytes aligned: ((((n-1)/16)+1)*16)
  ifelse(eval($2 > 11), 1, [
    li r0, -eval((((((8*$1)-1)/16)+1)*16) + 16*12); lvx v31, r1, r0 ])dnl
  ifelse(eval($2 > 10), 1, [
    li r0, -eval((((((8*$1)-1)/16)+1)*16) + 16*11); lvx v30, r1, r0 ])dnl
  ifelse(eval($2 >  9), 1, [
    li r0, -eval((((((8*$1)-1)/16)+1)*16) + 16*10); lvx v29, r1, r0 ])dnl
  ifelse(eval($2 >  8), 1, [
    li r0, -eval((((((8*$1)-1)/16)+1)*16) + 16* 9); lvx v28, r1, r0 ])dnl
  ifelse(eval($2 >  7), 1, [
    li r0, -eval((((((8*$1)-1)/16)+1)*16) + 16* 8); lvx v27, r1, r0 ])dnl
  ifelse(eval($2 >  6), 1, [
    li r0, -eval((((((8*$1)-1)/16)+1)*16) + 16* 7); lvx v26, r1, r0 ])dnl
  ifelse(eval($2 >  5), 1, [
    li r0, -eval((((((8*$1)-1)/16)+1)*16) + 16* 6); lvx v25, r1, r0 ])dnl
  ifelse(eval($2 >  4), 1, [
    li r0, -eval((((((8*$1)-1)/16)+1)*16) + 16* 5); lvx v24, r1, r0 ])dnl
  ifelse(eval($2 >  3), 1, [
    li r0, -eval((((((8*$1)-1)/16)+1)*16) + 16* 4); lvx v23, r1, r0 ])dnl
  ifelse(eval($2 >  2), 1, [
    li r0, -eval((((((8*$1)-1)/16)+1)*16) + 16* 3); lvx v22, r1, r0 ])dnl
  ifelse(eval($2 >  1), 1, [
    li r0, -eval((((((8*$1)-1)/16)+1)*16) + 16* 2); lvx v21, r1, r0 ])dnl
  ifelse(eval($2 >  0), 1, [
    li r0, -eval((((((8*$1)-1)/16)+1)*16) + 16* 1); lvx v20, r1, r0 ])dnl

  changequote(<,>)dnl
>)

divert(1)
