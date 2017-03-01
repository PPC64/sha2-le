#!/bin/bash
bin_dir="./bin"
a="abc"
b="EN?<AL@FSPDG?UBMENDNEWESOSHWMAL<CXAL>BQSO=DLV@=B>EJNDX=ED?IL@RLRNMOBUEMOUY?OWQMBAOAY<TMIYUALCTEPUVELQ>RHPHRQWHEBG>JQ?LHTPRLKDUUR?KVJXPIKT?>W@?@CIBDJ>Q?RX>JTPJFGNEAPYPCV@<MIU@QH<FQADFRPS<U@>MQAOM?HNFRTBLGQBWX<LYWXQIMHLFLKIKALSUXCMSM?VTDDHUFKSTJNJIUJ@COVUFBYTLOJD=HXNVNUEDS??RDYUVEQAW@?PDL?QL<HJQ<?L?W=WIDCKMLVU<RWR<U<RN<OG>EV=YHHMERNJ=PWFFR>M<DGYARUGFY=PDBWO=YQFYMOENCQR=VP==JAHYL>VOILXMOUYPNTHORPSBT>QNDVOLH>CEY?SN<QDLUXJYJ==YFOMFOWKTJ=VPY=HLB<WJJEXVFTEV?UXBY?JTUT<LTXHNNHJNHOGXHDCIYKIH<XPQU?GTU@BDSXRVGAQL=JXEL"
c="EN?<AL@FSPDG?UBMENDNEWESOSHWMAL<CXAL>BQSO=DLV@=B>EJNDX=ED?IL@RLRNMOBUEMOUY?OWQMBAOAY<TMIYUALCTEPUVELQ>RHPHRQWHEBG>JQ?LHTPRLKDUUR?KVJXPIKT?>W@?@CIBDJ>Q?RX>JTPJFGNEAPYPCV@<MIU@QH<FQADFRPS<U@>MQAOM?HNFRTBLGQBWX<LYWXQIMHLFLKIKALSUXCMSM?VTDDHUFKSTJNJIUJ@COVUFBYTLOJD=HXNVNUEDS??RDYUVEQAW@?PDL?QL<HJQ<?L?W=WIDCKMLVU<RWR<U<RN<OG>EV=YHHMERNJ=PWFFR>M<DGYARUGFY=PDBWO=YQFYMOENCQR=VP==JAHYL>VOILXMOUYPNTHORPSBT>QNDVOLH>CEY?SN<QDLUXJYJ==YFOMFOWKTJ=VPY=HLB<WJJEXVFTEV?UXBY?JTUT<LTXHNNHJNHOGXHDCIYKIH<XPQU?GTU@BDSXRVGAQL="
d="EN?<AL@FSPDG?UBMENDNEWESOSHWMAL<CXAL>BQSO=DLV@=B>EJNDX=ED?IL@RLRNMOBUEMOUY?OWQMBAOAY<TMIYUALCTEPUVELQ>RHPHRQWHEBG>JQ?LHTPRLKDUUR?KVJXPIKT?>W@?@CIBDJ>Q?RX>JTPJFGNEAPYPCV@<MIU@QH<FQADFRPS<U@>MQAOM?HNFRTBLGQBWX<LYWXQIMHLFLKIKALSUXCMSM?VTDDHUFKSTJNJIUJ@COVUFBYTLOJD=HXNVNUEDS??RDYUVEQAW@?PDL?QL<HJQ<?L?W=WIDCKMLVU<RWR<U<RN<OG>EV=YHHMERNJ=PWFFR>M<DGYARUGFY=PDBWO=YQFYMOENCQR=VP==JAHYL>VOILXMOUYPNTHORPSBT>QNDVOLH>CEY?SN<QDLUXJYJ==YFOMFOWKTJ=VPY=HLB<WJJEXVFTEV?UXBY?JTUT<LTXHNNHJNHOGXHDCIYKIH<XPQU?GTU@BDSXRVGAQL=JXELFJWNXCJSD"

# will exit with 1 if any error occur on comparison
RET_CODE=0

function cmp() {
    diff -u $1 _ref > _out
    if [[ $? -eq 0 ]]; then
      echo -en "$2 is Ok\t"
    else
      echo -en "$2 is NOK\n"
      tail -n2 _out
      RET_CODE=1
    fi
    rm -f _out
}

for sha_bits in 256 512; do
  echo "Running tests for SHA-${sha_bits}:";

  asm_bin=sha${sha_bits}_${CC}
  libcrypto_bin=sha${sha_bits}_libcrypto_${CC}
  ctr=1
  for file in $a $b $c $d; do
    echo -n $file > _tmp
    ${bin_dir}/${asm_bin} _tmp > _asm
    ${bin_dir}/${libcrypto_bin} _tmp > _libcrypto
    sha${sha_bits}sum _tmp | cut -d\  -f1 > _ref
    echo -en "Test #"${ctr}":\t"
    cmp _asm asm
    cmp _libcrypto libcrypto
    #TODO: fix this ugly hack
    ctr=$(echo $ctr + 1 | bc)
    echo ""
    rm -f _ref _c _asm _tmp _libcrypto
  done
  echo ""
done

exit $RET_CODE
