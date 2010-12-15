// Matthew Gwynne, 6.8.2009 (Swansea)
/* Copyright 2009, 2010 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*!
  \file Experimentation/Investigations/Cryptography/AdvancedEncryptionStandard/plans/FieldMulInvestigations.hpp
  \brief On investigations into the AES field multiplications


  \todo Connections
  <ul>
   <li> See Investigations/BooleanFunctions/plans/Permutations.hpp for
   general investigations on permutations of {0,1}^n. </li>
  </ul>


  \todo Minimum size small scale AES operations
  <ul>
   <li> See ssmult_fullcnf_fcs() in
   OKlib/ComputerAlgebra/Cryptology/Lisp/Cryptanalysis/Rijndael/testobjects/FieldOperationsAnalysis.mac . </li>
   <li> In the small scale AES, the size of the field and the size of the 
   block may vary and therefore, rather than simplying having the standard 8 
   bit "byte" field in AES, the "word" (a generalisation of byte) field
   may now be of arbitrary size. </li>
   <li> In reality, in the literature (see [Small Scale Variants of the AES; 
   Cid, Murphy and Robshaw]), only one field of size 4 is considered and
   in every field, only multiplications by 01, 02, and 03 are considered 
   (in each respective field). </li>
   <li> Therefore, other than those given for the standard AES, we must
   also consider the minimisation of these 01,02 and 03 multiplications
   over the 4 bit field. </li>
   <li> So we are considering the following minisation problems
   (given as maxima code to generate their minimum CNF representations) :
   <ul>
    <li> Multiplication by 01 in any field is the trivial identity, so we have
    the trivial equivalence translation as the minimum representation. </li>
    </li>
    <li> Multiplication by 02 in GF(2^4) with x^4+x+1 as the modulo polynomial:
    \verbatim
min_2_cnfs : all_minequiv_bvs_cs(ssmult_fullcnf_fcs(2,2,4,ss_polynomial_2_4)[2]);
    \endverbatim
    </li>
    <li> Multiplication by 03 in GF(2^4) with x^4+x+1 as the modulo polynomial:
    \verbatim
min_3_cnfs : all_minequiv_bvs_cs(ssmult_fullcnf_fcs(3,2,4,ss_polynomial_2_4)[2]);
    \endverbatim
    </li>
    </ul>
   </li>
  </ul>


  \todo Number of prime implicates for field multiplications
  <ul>
   <li> Currently, within the SAT translation, the most powerful representation
   of the Field operations used within the AES SAT translation (such as 
   multiplication by 02, 03 etc within Rijndael's byte field, see 
   ComputerAlgebra/Cryptology/Lisp/Cryptanalysis/Rijndael/plans/FieldOperationsAnalysis.hpp)
   is the canonical translation using new variables (see "dualts_fcl" in 
   ComputerAlgebra/Satisfiability/Lisp/ClauseSets/Constructions.mac).
   </li>
   <li> However, the most powerful representation of any boolean function, not 
   considering size of the translation is always the set of prime implicates. 
   </li>
   <li> In general, the set of prime implicates for a given boolean function is
   prohibitively large, such as is the case with the Sbox with > 100000
   clauses. </li>
   <li> This is not always the case though, as the size and structure of the 
   set of prime implicates for each of the field multiplication functions must
   be checked. </li>
   <li> Small set of prime implicates would provide much better representations
   than "dualts_fcl" produces, as then there is no problem that the SAT solver
   might somehow get "lost" branching or basing it's heuristics on the new 
   variables, and if the set of prime implicates is small enough, it might
   even be smaller than the "dualts_fcl" translation. </li>
   <li> The full CNF representation of the boolean function for multiplication
   by a constant factor n (where n is the integer representation for
   a polynomial in base 2) in Rijndael's byte-field, where n is an integer 
   (see 
   ComputerAlgebra/Cryptology/Lisp/Cryptanalysis/Rijndael/FieldOperationsAnalysis.mac), 
   can be generated by:
   \verbatim
output_rijnmult_fullcnf_stdname(n);
   \endverbatim
   and the prime implicates can then be generated using:
   \verbatim
QuineMcCluskey-n16-O3-DNDEBUG AES_byte_field_mul_full_n.cnf > AES_byte_field_mul_full_n.pi
   \endverbatim
   and getting a histogram of clause lengths
   \verbatim
> # Calculate the number of each length clause
> for n in `seq 1 16`; do echo -n $n ": " && C=`echo -n '^' && perl -e "print \"[^ ]+ +\" x $n" && echo '0$'` && cat AES_Sbox_pi.cnf | grep -v "^p" | grep -E "$C" | wc -l; done
   \endverbatim
   </li>
   <li> Multiplication by 1: </li>
   <li> Multiplication by 2:
   <ul>
    <li> There are 58 prime implicates for multiplication by 2. </li>
    <li> The minimal size of a prime clause is 2. </li>
    <li> The maximal size of a prime clause is 4. </li>
    <li> Multiplication by 2 is a very simple operation, which is essentially
    a shift operation, hence the small number of prime implicates. </li>
    <li> In this case, representing multiplication by 02 by all it's prime
    implicates seems the only real solution, given such a small number. </li>
   </ul>
   </li>
   <li> Multiplication by 3:
   <ul>
    <li> There are 5048 prime implicates for multiplication by 3. </li>
    <li> The minimal size of a prime clause is 3. </li>
    <li> The maximal size of a prime clause is 9. </li>
   </ul>
   </li>
   <li> Multiplication by 9:
   <ul>
    <li> There are 7840 prime implicates for multiplication by 9. </li>
    <li> The minimal size of a prime clause is 3. </li>
    <li> The maximal size of a prime clause is 9. </li>
   </ul>
   </li>
   <li> Multiplication by 11:
   <ul>
    <li> There are 15008 prime implicates for multiplication by 11. </li>
    <li> The minimal size of a prime clause is 4. </li>
    <li> The maximal size of a prime clause is 9. </li>
   </ul>
   </li>
   <li> Multiplication by 13:
   <ul>
    <li> There are 15312 prime implicates for multiplication by 13. </li>
    <li> The minimal size of a prime clause is 4. </li>
    <li> The maximal size of a prime clause is 9. </li>
   </ul>
   </li>
   <li> Multiplication by 14:
   <ul>
    <li> There are 14300 prime implicates for multiplication by 14. </li>
    <li> The minimal size of a prime clause is 3. </li>
    <li> The maximal size of a prime clause is 9. </li>
   </ul>
   </li>
   <li> We need to handle all multiplications; and we need the size (and
   number) of minimum CNF representations. </li>
   <li> It would be nice here to be able to read the clause-sets into the
   Maxima system, where we have statistics_fcl etc. (see "Input and output" in 
   ComputerAlgebra/Satisfiability/Lisp/ClauseSets/plans/general.hpp). </li>
  </ul>

  
  \todo Minimisation of the field operations
  <ul>
   <li> See "Minimisation" in 
   OKlib/Satisfiability/FiniteFunctions/plans/general.hpp . </li>
   <li> R QCA Packages
   <ul>
    <li> We can use the QCA package, given in 
    Buildsystem/ExternalSources/SpecialBuilds/plans/R.hpp to compute
    the minimum sized CNF or DNF clause-set representation. </li>
    <li> This should be possible using the following code:
     \verbatim
######## In Maxima #######
generate_full_byteop_tt(byteop) :=  
  map(
     lambda([ce],
       append(
         int2polyadic_padd(ce[1],2,8),
         int2polyadic_padd(ce[2],2,8),
         if byteop(ce[1]) = ce[2] then [1] else [0]))
     ,cartesian_product(setmn(0,255),setmn(0,255)))$

rijn_lookup_mul : lambda([b],
  buildq([b],lambda([a], aes_field_mul_data[b,a])))$

mulConstant : 2;
with_stdout(sconcat("RijndaelMul",mulConstant,".tt"), block(
  apply(print, endcons("O",create_list(i,i,1,16))),
  for tt_line in generate_full_byteop_tt(rijn_lookup_mul(mulConstant)) do
    apply(print,tt_line)
  ))$


######## In R ###########

oklib_load_all()
library(QCA)

mulConstant = 2
mul_tt = read.table(paste("RijndaelMul",mulConstant,".tt",sep=""),header=TRUE)
eqmcc(mul_tt, outcome="O", expl.0=TRUE)
    \endverbatim
    where mulConstant can be set in each case to one of 2,3 for the 
    multiplication in the encryption direction, and 9,11,13 or 14 for
    the multiplications used when the decryption of MixColumn is
    included in the translation. </li>  
    <li> Even with multiplication by 02, the R/QCA system still runs out of
    memory (see "Minimisation" in 
    OKlib/Satisfiability/FiniteFunctions/plans/general.hpp for details). </li>
    <li> Another possibility is to minimise the field multiplications by 02
    using the minimum transversal functions present in the Maxima subsystem. 
    Assuming the prime implicates for multiplication by 02 have been generated
    like so:
    \verbatim
output_rijnmult_fullcnf_stdname(2);
    \endverbatim
    in Maxima, and then from the shell
    \verbatim
QuineMcCluskey-n16-O3-DNDEBUG AES_byte_field_mul_full_2.cnf > AES_Mul2_PI.cnf
    \endverbatim
    the following, in Maxima, should produce a set of all minimum 
    representations
    \verbatim 
oklib_plain_include("stringproc")$

read_fcs_f(n) := block([fh, line, ll, cs : [], l,b,c],
  fh : openr(n),
  while stringp(line : readline(fh)) do (
    ll : tokens(line),
    if length(ll) >= 1 then
      if not(first(ll) = "c" or first(ll) = "p") then
        cs : cons(setify(rest(map(parse_string,ll),-1)), cs)
  ),
  cs : setify(cs),
  return(cs_to_fcs(cs))
)$

Mul2PI : read_fcs("AES_Mul2_PI.cnf")$
MTHG2 : minimum_transversals_bvs_hg(ghg2hg(subsumption_ghg(Mul2[2], rijnmult_fullcnf_fcs(2)[2])))$
    \endverbatim
    </li>
    <li> For multiplication by 02, the above Maxima function returns 102 
    minimum CNF representations of size 20 in 2190.1490 seconds. </li>
    <li> An example of such a minimum representation is:
    \verbatim
{{-16,-15,-8},{-16,-13,-6},{-16,6,13},{-16,8,15},{-15,1,8},{-14,7},{-13,1,6},
{-12,-5,-1},{-12,5,16},{-11,4},{-10,3},{-9,2},{-8,1,15},{-7,14},{-6,1,13},
{-5,12,16},{-4,11},{-3,10},{-2,9},{-1,5,12}}
    \endverbatim
    </li>
    <li> Most (90) of the minimum representations contain 8 clauses of size 2, 
    and 12 clauses of size 3. There are then a further twelve clause-sets 
    where there are only 8 clause of size 3, but then 4 clauses of size 4. 
    </li>
    <li> MG is currently running experiments with the other field 
    multiplications. </li>
   </ul>
   </li>
   <li> Espresso-ab (see Logic "synthesis" in
   Buildsystem/ExternalSources/SpecialBuilds/plans/BooleanFunctions.hpp)
   <ul>
    <li> Espresso-ab takes as input a truth table in PLA format. </li>
    <li> We can generate a truth table in PLA format for the Sbox in the
    following way:
    \verbatim
generate_full_aes_mul_tt(elem) :=  
  map(
     lambda([ce],
       append(
         int2polyadic_padd(ce[1],2,8),
         int2polyadic_padd(ce[2],2,8),
         if rijn_natmul(elem,ce[1]) = ce[2] then [1] else [0]))
     ,cartesian_product(setmn(0,255),setmn(0,255)))$

elem : 3;
with_stdout(sconcat("Mul",elem,".pla"), block(
  print(".i 16"),
  print(".o 1"),
  for tt_line in generate_full_aes_mul_tt(elem) do
    print(apply(sconcat,rest(tt_line,-1)),1-last(tt_line))
  ))$
    \endverbatim
    where the PLA file will be called "Sbox.pla", and will be represented
    as a DNF representing the negation of the Sbox (as by default Espresso
    minimises DNF formulas.
    </li>
    <li> Using espresso-ab with the "exact" option for 02, espresso
    yields a minimum size clause-set of size 10, however
    for 03, espresso-ab runs out of memory on an 8GB machine. </li>
   </ul>
   </li>
  </ul>

  \todo Finding small solutions
  <ul>
                                                    <li> For all but the non-trivial (01 and 02) multiplications, finding the minimum representation, whether it be using espresso, or using branch and bound methods, very little progresses is made by the solvers. This is presumably due to the large number of prime implicates, and sheer combinatorial explosion. </li>
                                                    <li> Therefore, we must also consider only small minimal solutions, which might not be the minimum but we hope are close. </li>
   <li> Finding small CNF representations via weighted MaxSAT
   <ul>
    <li> To find the minimum representation for the full clause-set
                                                    of a given field multiplication, generated like so
    \verbatim
output_rijnmult_fullcnf_stdname(2);
    \endverbatim
    we can consider the translation of this to the subsumption hypergraph
    for the clause-set and it's prime implicates (see XXX) and then
    translate this subsumption hypergraph to a weighted MaxSAT problem
    where the minimum weight solutions to this problem correspond exactly
    to the minimum sized clause-set representations of the original clause-set.
    </li>
    <li> To do this, we construct the subsumption hypergraph like so
    \verbatim
QuineMcCluskeySubsumptionHypergraph-n16-O3-DNDEBUG AES_byte_field_mul_full_2.cnf AES_byte_field_mul_2_pi.cnf > AES_byte_field_mul_2_shg.cnf
    \endverbatim
    where the prime implicates for the "AES_byte_field_mul_full_2.cnf" are 
    placed in "AES_byte_field_mul_2_pi.cnf" and the subsumption hypergraph 
    relating the prime implicates to the original total clauses is placed in
    "AES_byte_field_mul_2_shg.cnf". 
    </li>
    <li> Then the subsumption hypergraph must be converted to a weight MaxSAT
    problem, which can be accomplished using a simple AWK script, like so
    \verbatim
cat AES_byte_field_mul_2_shg.cnf | 
awk 'BEGIN    { VARS=0; CLAUSES=0 }
     /^p/     { VARS=$3; CLAUSES=$4; print "p wcnf " $3 " " ($3+$4) }
     /^[^cp]/ { print (VARS + 1) " " $0 }
     /^c/     { print }
     END      { for (i = 1; i <= VARS; i++) print "1 " "-" i " 0" }' > AES_byte_field_mul_2_shg.wcnf 
    \end
    (The above translation needs to be further specified).
    </li>
    <li> Given such a weighted maxsat problem, one can then use ubcsat to
    search for small solutions, like so
    \verbatim
ubcsat-okl  -alg gsat -w -runs 100 -cutoff 1000000 -i AES_byte_field_mul_2_shg.wcnf
    \endverbatim
    </li>
    <li> Given a solution of weight 20 with seed 1402276559 we can then 
    generate the clause set of that weight like so
    \verbatim
function shg_assignment2clause_set() {
  PI_FILE=$1 
  # Grab lines out
  LINES=`cat - | xargs echo | sed -e 's/v//g' | sed -e 's/ *\(-[0-9]\+\|v\)//g'`;
  echo ${LINES}
  NUM_CLAUSES=`echo "${LINES}" | wc -w`
  # Grab out specifically those lines, ignoring p and c lines and combining 
  # clauses onto one line with ExtendedToStrictDimacs-O3-DNDEBUG
  AWK_LINES=`echo ${LINES} | sed -e 's/ \+/||(NR - 1) == /g'`; 
  cat ${PI_FILE} | grep -v "^c" | 
  awk "(NR-1) == ${AWK_LINES} { print  }
       /^p/ { print \"p cnf \" \$3 \" \" ${NUM_CLAUSES} }"
}
ubcsat-okl  -alg gsat -w -runs 100 -cutoff 1000000 -wtarget 20 -solve 1 -seed 1402276559 -i AES_byte_field_mul_2_shg.wcnf >  AES_byte_field_mul_2_m20.result; 
cat AES_byte_field_mul_2_m20.result |
awk "BEGIN {READ=0}
/^Variables/ {READ=0}
READ == 1 { print }
/^# Solution found for -wtarget/ {READ=1}" | shg_assignment2clause_set AES_byte_field_mul_2_pi.cnf
    \endverbatim
    Note here, the above should be reimplemented as a proper shell script
    or C++ program
    </li>
   </ul>
   </li>
  </ul>


*/
