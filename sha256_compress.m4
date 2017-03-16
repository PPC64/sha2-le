
.file "sha256-compress.s"

# Keep in mind that vector loading/store is/would-be done directly by dealing
# with 16-bytes:
# +-------+----+----+----+----+
# |       |   Vector Words:   |
# | Name  | #1 | #2 | #3 | #4 |
# +-------+----+----+----+----+
# | Va    | a  | b  | c  | d  |
# +-------+----+----+----+----+
#
# But this algorithm will mainly deal with each data (a-h) separately on a
# vector:
# +-------+----+----+----+----+
# |       |   Vector Words:   |
# | Name  | #1 | #2 | #3 | #4 |
# +-------+----+----+----+----+
# | Va    | a  | -  | -  | -  |
# | Vb    | b  | -  | -  | -  |
# | Vc    | c  | -  | -  | -  |
# | Vd    | d  | -  | -  | -  |
# +-------+----+----+----+----+

divert(-1) dnl suppress output for now

dnl Input parameters
define(<STATE>,    <r3>)
define(<INPUT>,    <r4>)
define(<K>,        <r5>)
dnl Indicates if the input memory pointers are not aligned to 16bytes.
dnl If they are aligned, the code is smaller and possibly faster.
define(<STATE_MEM_16BYTE_ALIGNED>, 0) dnl 1 aligned, 0 not aligned
define(<INPUT_MEM_16BYTE_ALIGNED>, 0) dnl 1 aligned, 0 not aligned
define(<K_MEM_16BYTE_ALIGNED>,     0) dnl 1 aligned, 0 not aligned

dnl Main vector registers
define(<a>,        <v9>)
define(<b>,        <v10>)
define(<c>,        <v11>)
define(<d>,        <v12>)
define(<e>,        <v13>)
define(<f>,        <v14>)
define(<g>,        <v15>)
define(<h>,        <v16>)
define(<w0>,       <v17>)
define(<w1>,       <v18>)
define(<w2>,       <v19>)
define(<w3>,       <v20>)

dnl Temporary registers
define(<t0>,       <r6>)
define(<t1>,       <r7>)
define(<j>,        <r8>)
define(<ch>,       <v0>)
define(<maj>,      <v1>)
define(<bsa>,      <v2>)
  define(<vt6>,      <v2>)
  define(<vt6_vs>,   <vs34>)
define(<bse>,      <v3>)
  define(<vt5>,      <v3>)
  define(<vt5_vs>,   <vs35>)
define(<vt0>,      <v4>)
  define(<vt0_vs>,   <vs36>)
define(<vt1>,      <v5>)
  define(<vt1_vs>,   <vs37>)
define(<vt2>,      <v6>)
  define(<vt2_vs>,   <vs38>)
define(<vt3>,      <v7>)
  define(<vt3_vs>,   <vs39>)
define(<vt4>,      <v8>)
  define(<vt4_vs>,   <vs40>)
define(<vrb>,      <v21>)
define(<vsp8>,     <v22>)
  define(<aux>,      <v22>)
  define(<kpw0>,     <v22>)   dnl has only one kpw value
define(<vsp16>,    <v23>)
  define(<kpw1>,     <v23>)   dnl has only one kpw value
define(<shiftarg>, <v24>)
  define(<kpw2>,     <v24>)   dnl has only one kpw value
define(<kpw3>,     <v25>)     dnl has only one kpw value
define(<kplusw0>,  <v26>)     dnl contains 4 kpw inside
define(<kplusw1>,  <v27>)     dnl contains 4 kpw inside
define(<kplusw2>,  <v28>)     dnl contains 4 kpw inside
define(<kplusw3>,  <v29>)     dnl contains 4 kpw inside

dnl UNPACK(SRC,DST1,DST2,DST3)
dnl Unpacks 4 words from SRC to SRC,DST1,DST2,DST3
define(<UNPACK>, <
  vsldoi $2, $1, $1, 12
  vsldoi $3, $1, $1, 8
  vsldoi $4, $1, $1, 4
>)

dnl LOAD_W_PLUS_K()
dnl Loads all 16 words needed to W and sum them with K Treats unalignment.
define(<LOAD_W_PLUS_K>, <
  ifelse(INPUT_MEM_16BYTE_ALIGNED, 1, <
    # aligned load of input
    # w0 = w[j-16] to w[j-13]
    lvx       w0,   0,     INPUT
    addi      t0,   INPUT, 16
    # w1 = w[j-12] to w[j-9]
    lvx       w1,   0,     t0
    addi      t0,   INPUT, 32
    # w2 = w[j-8] to w[j-5]
    lvx       w2,   0,     t0
    addi      t0,   INPUT, 48
    # w3 = w[j-4] to w[j-1]
    lvx       w3,   0,     t0
  >,<
    # set vrb according to alignment
    lvsr      vrb,  0,     INPUT

    # unaligned load
    lvx       w0,   0,     INPUT
    addi      t0,   INPUT, 16
    lvx       w1,   0,     t0

    # w0 = w[j-16] to w[j-13]
    vperm     w0,   w1,    w0,   vrb
    addi      t0,   INPUT, 32
    lvx       w2,   0,     t0

    # w1 = w[j-12] to w[j-9]
    vperm     w1,   w2,    w1,   vrb
    addi      t0,   INPUT, 48
    lvx       w3,   0,     t0

    # w2 = w[j-8] to w[j-5]
    vperm     w2,   w3,    w2,   vrb
    addi      t0,   INPUT, 64
    lvx       vt0,  0,     t0

    # w3 = w[j-4] to w[j-1]
    vperm     w3,   vt0,   w3,   vrb
  >)

  ifelse(K_MEM_16BYTE_ALIGNED, 1, <
    # aligned load
    # vt0 = K[j-16] to K[j-13]
    lvx       vt0,  0,     K
    addi      t0,   K,     16
    # vt1 = K[j-12] to K[j-9]
    lvx       vt1,  0,     t0
    addi      t0,   K,     32
    # vt2 = K[j-8] to K[j-5]
    lvx       vt2,  0,     t0
    addi      t0,   K,     48
    # vt3 = K[j-4] to K[j-1]
    lvx       vt3,  0,     t0
  >,<
    # set vrb according to alignment
    lvsr      vrb,  0,     K

    # unaligned load
    lvx       vt0,  0,     K
    addi      t0,   K,     16
    lvx       vt1,  0,     t0

    # vt0 = K[j-16] to K[j-13]
    vperm     vt0,  vt1,   vt0,  vrb
    addi      t0,   K,     32
    lvx       vt2,  0,     t0

    # vt1 = K[j-12] to K[j-9]
    vperm     vt1,  vt2,   vt1,  vrb
    addi      t0,   K,     48
    lvx       vt3,  0,     t0

    # vt2 = K[j-8] to K[j-5]
    vperm     vt2,  vt3,   vt2,  vrb
    addi      t0,   K,     64
    lvx       vt4,  0,     t0

    # vt3 = K[j-4] to K[j-1]
    vperm     vt3,  vt4,   vt3,  vrb
  >)

  # Swap bytes
  vspltisw  vsp16,8
  vspltish  vsp8, 8
  vspltisw  shiftarg,1
  vsl       vsp16,vsp16, shiftarg
  vrlh      w0,   w0,   vsp8
  vrlh      w1,   w1,   vsp8
  vrlh      w2,   w2,   vsp8
  vrlh      w3,   w3,   vsp8
  vrlw      w0,   w0,   vsp16
  vrlw      w1,   w1,   vsp16
  vrlw      w2,   w2,   vsp16
  vrlw      w3,   w3,   vsp16

  # Add _w to K
  vadduwm   kplusw0,vt0,w0
  vadduwm   kplusw1,vt1,w1
  vadduwm   kplusw2,vt2,w2
  vadduwm   kplusw3,vt3,w3
>)

dnl LOAD_H_VEC(A,E)
dnl Loads 8 words from STATE to vectors A,E (packed).
define(<LOAD_H_VEC>, <
  ifelse(STATE_MEM_16BYTE_ALIGNED, 1, <
    # load aligned: a = {a,b,c,d}
    lvx    $1,  0,    STATE
    addi   t0,  STATE,16
    # load aligned: e = {e,f,g,h}
    lvx    $2,  0,    t0
  >,<
    # load unaligned
    lvx    $1,  0,    STATE
    addi   t0,  STATE,16
    lvsr   t1,  0,    t0
    lvx    $2,  0,    t0
    vperm  $1,  $2,   $1,  t1       # a = {a,b,c,d}
    addi   t0,  t0,   16
    lvx    aux, 0,    t0
    vperm  $2,  aux,  $2,  t1       # e = {e,f,g,h}
  >)
>)

dnl CALC_4W()
dnl Calculate 16 words W0,W1,W2,W3 and K to KPW0,KPW1,KPW2,KPW3 and advance j
define(<CALC_4W>, <
  ifelse(K_MEM_16BYTE_ALIGNED, 1, <
    # Load aligned of K[j]
    lvx        vt0,  j,     K
  >,<
    # Load unaligned of K[j]
    lvx        vt0,  j,     K
    addi       t0,   j,     16
    lvsr       vrb,  0,     K
    lvx        vt1,  t0,    K
    vperm      vt0,  vt1,   vt0,   vrb
  >)

  # Advance j
  addi       j,    j,     16

  # b = w[j-15], w[j-14], w[j-13], w[j-12]
  vsldoi     vt1,  w1,    w0,  12

  # c = w[j-7], w[j-6], w[j-5], w[j-4]
  vsldoi     vt2,  w3,    w2,  12

  # d = w[j-2], w[j-1], w[j-4], w[j-3]
  vsldoi     vt3,  w3,    w3,  8

  # b = s0(w[j-15]) , s0(w[j-14]) , s0(w[j-13]) , s0(w[j-12])
  vshasigmaw vt1,  vt1,   0,   0

  # d = s1(w[j-2]) , s1(w[j-1]) , s1(w[j-4]) , s1(w[j-3])
  vshasigmaw vt3,  vt3,   0,   0xf

  # c = s0(w[j-15]) + w[j-7],
  #     s0(w[j-14]) + w[j-6],
  #     s0(w[j-13]) + w[j-5],
  #     s0(w[j-12]) + w[j-4]
  vadduwm    vt2,  vt1,   vt2

  # c = s0(w[j-15]) + w[j-7] + w[j-16],
  #     s0(w[j-14]) + w[j-6] + w[j-15],
  #     s0(w[j-13]) + w[j-5] + w[j-14],
  #     s0(w[j-12]) + w[j-4] + w[j-13]
  vadduwm    vt2,  vt2,   w0

  # e = s0(w[j-15]) + w[j-7] + w[j-16] + s1(w[j-2]), // w[j]
  #     s0(w[j-14]) + w[j-6] + w[j-15] + s1(w[j-1]), // w[j+1]
  #     s0(w[j-13]) + w[j-5] + w[j-14] + s1(w[j-4]), // UNDEFINED
  #     s0(w[j-12]) + w[j-4] + w[j-13] + s1(w[j-3])  // UNDEFINED
  vadduwm    vt4,  vt2,   vt3

  # At this point, e[0] and e[1] are the correct values to be stored at w[j]
  # and w[j+1].
  # e[2] and e[3] are not considered.
  # b = s1(w[j]) , s1(s(w[j+1]) , UNDEFINED , UNDEFINED
  vshasigmaw vt1,  vt4,   0,   0xf

  # v5 = s1(w[j-2]) , s1(w[j-1]) , s1(w[j]) , s1(w[j+1])
  xxmrgld    vt3_vs,vt1_vs,vt3_vs

  # c = s0(w[j-15]) + w[j-7] + w[j-16] + s1(w[j-2]), // w[j]
  #     s0(w[j-14]) + w[j-6] + w[j-15] + s1(w[j-1]), // w[j+1]
  #     s0(w[j-13]) + w[j-5] + w[j-14] + s1(w[j]),   // w[j+2]
  #     s0(w[j-12]) + w[j-4] + w[j-13] + s1(w[j+1])  // w[j+4]
  vadduwm    vt2,  vt2,   vt3

  # Updating w0 to w3 to hold the new previous 16 values from w.
  vmr        w0,   w1
  vmr        w1,   w2
  vmr        w2,   w3
  vmr        w3,   vt2

  # Store K + w to v9 (4 values at once)
  vadduwm    kpw0, vt2,   vt0

  vsldoi     kpw1, kpw0,  kpw0,12
  vsldoi     kpw2, kpw0,  kpw0,8
  vsldoi     kpw3, kpw0,  kpw0,4
>)

dnl ROUND(A,B,C,D,E,F,G,H,KPW)
dnl H += S1(E) + Choice(E,F,G) + KPW
dnl D += H
dnl H += S0(A) + Majority(A,B,C)
dnl
dnl Where
dnl S1(E) = vshasigmaw(E,1,0xF)
dnl S0(A) = vshasigmaw(A,1,0x0)
dnl Choice (E, F, G) = vsel(E, F, G)
dnl Majority (A,B,C) = vsel(B, C, vxor(A, B))
define(<ROUND>, <
  vsel       ch,  $7,  $6, $5           # ch  = Ch(e,f,g)
  vxor       maj, $1,  $2               # intermediate Maj
  vsel       maj, $2,  $3, maj          # maj = Maj(a,b,c)
  vshasigmaw bsa, $1,  1,  0            # bsa = BigSigma0(a)
  vshasigmaw bse, $5,  1,  0xF          # bse = BigSigma1(e)
  vadduwm    vt1, $8,  bse              # vt1 = h + bse
  vadduwm    vt2, ch,  $9               # vt2 = ch + kpw
  vadduwm    vt3, vt1, vt2              # vt3 = h + bse + ch + kpw
  vadduwm    vt4, bsa, maj              # vt4 = bsa + maj
  vadduwm    $4,  $4,  vt3              # d   = d + vt3
  vadduwm    $8,  vt3, vt4              # h   = vt3 + vt4
>)

dnl UPDATE_SHA_STATE()
dnl Update hash by modulo summing old and new value and write on memory
define(<UPDATE_SHA_STATE>, <
  li        t0,    16
  li        t1,    32

  ifelse(STATE_MEM_16BYTE_ALIGNED, 1, <
    # vt0 = STATE[0]..STATE[3]
    lvx       vt0,   0,     STATE
    # vt5 = STATE[4]..STATE[8]
    lvx       vt5,   t0,    STATE
  >,<
    lvsr      vrb,   0,     STATE
    lvx       vt0,   0,     STATE         # vt0 = STATE[0]..STATE[3]
    lvx       vt5,   t0,    STATE         # vt5 = STATE[4]..STATE[8]
    vperm     vt0,   vt5,   vt0,   vrb
    lvx       vt6,   t1,    STATE         # vt5 = STATE[4]..STATE[8]
    vperm     vt5,   vt6,   vt5,   vrb
  >)

  vmrglw    vt1,   b,     a             # vt1 = {a, b, ?, ?}
  vmrglw    vt2,   d,     c             # vt2 = {c, d, ?, ?}
  vmrglw    vt3,   f,     e             # vt3 = {e, f, ?, ?}
  vmrglw    vt4,   h,     g             # vt4 = {g, h, ?, ?}
  xxmrgld   vt1_vs,vt2_vs,vt1_vs        # vt1 = {a, b, c, d}
  xxmrgld   vt3_vs,vt4_vs,vt3_vs        # vt3 = {e, f, g, h}

  # vt0 = {a+STATE[0], b+STATE[1], c+STATE[2], d+STATE[3]}
  vadduwm   vt0,   vt0,   vt1

  # vt5 = {e+STATE[4], f+STATE[5], g+STATE[6], h+STATE[7]
  vadduwm   vt5,   vt5,   vt3

  ifelse(STATE_MEM_16BYTE_ALIGNED, 1, <
    stvx      vt0,   0,     STATE
    li        t0,    16
    stvx      vt5,   t0,    STATE
  >,<
    mfvrwz    aux,   vt0                  # aux = a+STATE[0]
    stw       aux,   8(STATE)             # update h[3]

    # vt6 = {b+STATE[1], c+STATE[2], d+STATE[3], a+STATE[0]}
    vsldoi    vt6,   vt0,   vt0,   12
    mfvrwz    aux,   vt6                  # aux = b+STATE[1]
    stw       aux,   12(STATE)            # update h[2]

    # vt6 = {c+STATE[2], d+STATE[3], a+STATE[0], b+STATE[1]}
    vsldoi    vt6,   vt6,   vt6,   12
    mfvrwz    aux,   vt6                  # aux = c+STATE[2]
    stw       aux,   0(STATE)             # update h[1]

    # vt6 = {d+STATE[3], a+STATE[0], b+STATE[1], c+STATE[2]}
    vsldoi    vt6,   vt6,   vt6,   12
    mfvrwz    aux,   vt6                  # aux = d+STATE[3]
    stw       aux,   4(STATE)             # update h[0]
    mfvrwz    aux,   vt5                  # aux = e+STATE[4]
    stw       aux,   24(STATE)            # update h[7]

    # vt6 = {f+STATE[5], g+STATE[6], d+STATE[3], h+STATE[7]}
    vsldoi    vt6,   vt5,   vt5,   12
    mfvrwz    aux,   vt6                  # aux = f+STATE[5]
    stw       aux,   28(STATE)            # update h[6]

    # vt6 = {g+STATE[6], h+STATE[7], e+STATE[4], f+STATE[5]}
    vsldoi    vt6,   vt6,   vt6,   12
    mfvrwz    aux,   vt6                  # aux = g+STATE[6]
    stw       aux,   16(STATE)            # update h[5]

    # vt6 = {h+STATE[7], e+STATE[4], f+STATE[5], g+STATE[6]}
    vsldoi    vt6,   vt6,   vt6,   12
    mfvrwz    aux,   vt6                  # aux = h+STATE[7]
    stw       aux,   20(STATE)
  >)
>)

divert(1) dnl enable output

.text
ALIGN(16)
dnl void sha256_compress(uint32_t *STATE, const uint8_t *input, const uint32_t *k)
PROLOGUE(sha256_compress, 0, 10)  dnl v20 up to v29 are used

  # Load hash STATE to registers
  LOAD_H_VEC(a,e)

  # Unpack a-h data from the packed vector to a vector register each
  UNPACK(a, b, c, d)
  UNPACK(e, f, g, h)

  # Load 16 elements from w out of the loop
  LOAD_W_PLUS_K()

  UNPACK(kplusw0, kpw1, kpw2, kpw3)
  # iteration: #1
  ROUND(a, b, c, d, e, f, g, h, kplusw0)
  # iteration: #2
  ROUND(h, a, b, c, d, e, f, g, kpw1)
  # iteration: #3
  ROUND(g, h, a, b, c, d, e, f, kpw2)
  # iteration: #4
  ROUND(f, g, h, a, b, c, d, e, kpw3)

  UNPACK(kplusw1, kpw1, kpw2, kpw3)
  # iteration: #5
  ROUND(e, f, g, h, a, b, c, d, kplusw1)
  # iteration: #6
  ROUND(d, e, f, g, h, a, b, c, kpw1)
  # iteration: #7
  ROUND(c, d, e, f, g, h, a, b, kpw2)
  # iteration: #8
  ROUND(b, c, d, e, f, g, h, a, kpw3)

  UNPACK(kplusw2, kpw1, kpw2, kpw3)
  # iteration: #9
  ROUND(a, b, c, d, e, f, g, h, kplusw2)
  # iteration: #10
  ROUND(h, a, b, c, d, e, f, g, kpw1)
  # iteration: #11
  ROUND(g, h, a, b, c, d, e, f, kpw2)
  # iteration: #12
  ROUND(f, g, h, a, b, c, d, e, kpw3)

  UNPACK(kplusw3, kpw1, kpw2, kpw3)
  # iteration: #13
  ROUND(e, f, g, h, a, b, c, d, kplusw3)
  # iteration: #14
  ROUND(d, e, f, g, h, a, b, c, kpw1)
  # iteration: #15
  ROUND(c, d, e, f, g, h, a, b, kpw2)
  # iteration: #16
  ROUND(b, c, d, e, f, g, h, a, kpw3)

  # j will be multiple of 4 for loading words.
  # Whenever read, advance the pointer (e.g: when j is used in CALC_4W)
  li        j,   16*4

  # Rolling the 16 to 64 rounds
  li        t0, (64-16)/8
  mtctr     t0

ALIGN(16)
.Loop1:
  CALC_4W()
  # iteration: #17 #25 #33 #41 #49 #57
  ROUND(a, b, c, d, e, f, g, h, kpw0)
  # iteration: #18 #26 #34 #42 #50 #58
  ROUND(h, a, b, c, d, e, f, g, kpw1)
  # iteration: #19 #27 #35 #43 #51 #59
  ROUND(g, h, a, b, c, d, e, f, kpw2)
  # iteration: #20 #28 #36 #44 #52 #60
  ROUND(f, g, h, a, b, c, d, e, kpw3)

  CALC_4W()
  # iteration: #21 #29 #37 #45 #53 #61
  ROUND(e, f, g, h, a, b, c, d, kpw0)
  # iteration: #22 #30 #38 #46 #54 #62
  ROUND(d, e, f, g, h, a, b, c, kpw1)
  # iteration: #23 #31 #39 #47 #55 #63
  ROUND(c, d, e, f, g, h, a, b, kpw2)
  # iteration: #24 #32 #40 #48 #56 #64
  ROUND(b, c, d, e, f, g, h, a, kpw3)
  bdnz .Loop1

  # Update hash STATE
  UPDATE_SHA_STATE()


EPILOGUE(sha256_compress, 0, 10)  dnl v20 up to v29 are used

