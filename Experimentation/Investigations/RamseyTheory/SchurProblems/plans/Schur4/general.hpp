// Oliver Kullmann, 19.7.2012 (Swansea)
/* Copyright 2012 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*!
  \file Experimentation/Investigations/RamseyTheory/SchurProblems/plans/Schur4/general.hpp
  \brief On schur(4) and variations


  \todo Best method for determining wschur(4) = 67
  <ul>
   <li> OKsolver-2002:

   </li>
   <li> minisat-2.2.0:

   </li>
   <li> SplittingViaOKsolver (using minisat-2.2.0):
   \verbatim
> SplittingViaOKsolver -D40 WSchur_4_66.cnf
> cat SplitViaOKsolver_D40WSchur_4_66cnf_2012-07-19-192843/Result 
s UNKNOWN
c sat_status                            2
c initial_maximal_clause_length         4
c initial_number_of_variables           264
c initial_number_of_clauses             4686
c initial_number_of_literal_occurrences 13728
c number_of_2-clauses_after_reduction   396
c running_time(sec)                     271.6
c number_of_nodes                       201407
c number_of_quasi_single_nodes          0
c number_of_2-reductions                696
c number_of_pure_literals               0
c number_of_autarkies                   0
c max_tree_depth                        24
c proportion_searched                   0.000000e+00
c proportion_single                     0.000000e+00
c total_proportion                      0
c number_of_table_enlargements          0
c number_of_1-autarkies                 45197627
c splitting_cases                       100704
> cat Md5sum
080dbcd2783899d98b45e00974ed18cf
> cat Statistics
> summary(E$n)
   Min. 1st Qu.  Median    Mean 3rd Qu.    Max. 
  40.00   40.00   42.00   42.31   44.00   49.00 
> table(E$n)
   40    41    42    43    44    45    46    47    48    49 
28272 11064 13416 18096 16776  6264  5520  1248    24    24 
> summary(E$d)
   Min. 1st Qu.  Median    Mean 3rd Qu.    Max. 
  10.00   17.00   18.00   18.13   20.00   24.00 
> table(E$d)
   10    11    12    13    14    15    16    17    18    19    20    21    22 
   11    60   339   982  2637  5922 10812 16169 19305 18720 14018  7943  3026 
   23    24 
  680    80 


> ProcessSplitViaOKsolver SplitViaOKsolver_D40WSchur_4_66cnf_2012-07-19-192843
> cat Result 
SATISFIABLE
Found 24 satisfiable sub-instances.
  PID TTY      STAT   TIME COMMAND
27401 pts/2    R+   308:05 /bin/bash /home/csoliver/OKplatform/bin/ProcessSplitViaOKsolver SplitViaOKsolver_D40WSchur_4_66cnf_2012-07-19-192843
> cat SatisfyingAssignments 
48875 7425
48876 7429
49205 9447
49206 9451
49535 11469
49536 11473
52229 32601
52230 32605
52559 34623
52560 34627
52889 36645
52890 36649
55583 57777
55584 57781
55913 59799
55914 59803
56243 61821
56244 61825
58937 82953
58938 82957
59267 84975
59268 84979
59597 86997
59598 87001
> E=read_processsplit_minisat()
100704: 3.813h, sum-cfs=8.768916e+08, mean-t=0.136s, mean-cfs=8708, sat: 0 1 
$t:
   Min. 1st Qu.  Median    Mean 3rd Qu.    Max. 
 0.0080  0.0640  0.1000  0.1363  0.1520  8.3490 
sd= 0.1911814 
     95%      96%      97%      98%      99%     100% 
0.320020 0.352022 0.404025 0.496031 0.736046 8.348520 
sum= 13727.21 
$cfs:
   Min. 1st Qu.  Median    Mean 3rd Qu.    Max. 
    123    4165    6437    8708    9912  509000 
sd= 12001.76 
      95%       96%       97%       98%       99%      100% 
 20132.00  22292.00  25611.46  31218.14  45977.25 508953.00 
sum= 876891551 
$t ~ $cfs:
               Estimate  Std. Error  t value  Pr(>|t|)    
(Intercept) -1.8749e-03  6.4397e-05  -29.115 < 2.2e-16 ***
E$cfs        1.5870e-05  4.3430e-09 3654.097 < 2.2e-16 ***
R-squared: 0.9925 



> SplittingViaOKsolver -D40 WSchur_4_67.cnf
> cat SplitViaOKsolver_D40WSchur_4_67cnf_2012-07-19-121844/Result 
s UNKNOWN
c sat_status                            2
c initial_maximal_clause_length         4
c initial_number_of_variables           268
c initial_number_of_clauses             4825
c initial_number_of_literal_occurrences 14140
c number_of_2-clauses_after_reduction   402
c running_time(sec)                     276.9
c number_of_nodes                       201407
c number_of_quasi_single_nodes          0
c number_of_2-reductions                696
c number_of_pure_literals               0
c number_of_autarkies                   0
c max_tree_depth                        24
c proportion_searched                   0.000000e+00
c proportion_single                     0.000000e+00
c total_proportion                      0
c number_of_table_enlargements          0
c number_of_1-autarkies                 46003255
c splitting_cases                       100704
> cat Statistics
> summary(E$n)
   Min. 1st Qu.  Median    Mean 3rd Qu.    Max. 
  40.00   40.00   42.00   42.31   44.00   49.00 
> table(E$n)
   40    41    42    43    44    45    46    47    48    49 
28272 11064 13416 18096 16776  6264  5520  1248    24    24 
> summary(E$d)
   Min. 1st Qu.  Median    Mean 3rd Qu.    Max. 
  10.00   17.00   18.00   18.13   20.00   24.00 
> table(E$d)
   10    11    12    13    14    15    16    17    18    19    20    21    22 
   11    60   339   982  2637  5922 10812 16169 19305 18720 14018  7943  3026 
   23    24 
  680    80 

> ProcessSplitViaOKsolver SplitViaOKsolver_D40WSchur_4_67cnf_2012-07-19-121844
> cat Result 
UNSATISFIABLE
  PID TTY      STAT   TIME COMMAND
13122 pts/2    R+   339:12 /bin/bash /home/csoliver/OKplatform/bin/ProcessSplitViaOKsolver SplitViaOKsolver_D40WSchur_4_67cnf_2012-07-19-121844
> E=read_processsplit_minisat()
100704: 4.279h, sum-cfs=9.087163e+08, mean-t=0.153s, mean-cfs=9024, sat: 0 
$t:
   Min. 1st Qu.  Median    Mean 3rd Qu.    Max. 
  0.008   0.072   0.112   0.153   0.176   9.177 
sd= 0.2058379 
     95%      96%      97%      98%      99%     100% 
0.372023 0.411545 0.468029 0.560035 0.780048 9.176570 
sum= 15405.8 
$cfs:
   Min. 1st Qu.  Median    Mean 3rd Qu.    Max. 
    136    4333    6710    9024   10380  540200 
sd= 12170.76 
      95%       96%       97%       98%       99%      100% 
 21118.70  23298.76  26364.00  31717.58  46117.16 540202.00 
sum= 908716288 
$t ~ $cfs:
              Estimate Std. Error  t value  Pr(>|t|)    
(Intercept) 3.0345e-03 1.5026e-04   20.195 < 2.2e-16 ***
E$cfs       1.6617e-05 9.9176e-09 1675.510 < 2.2e-16 ***
R-squared: 0.9654 
   \endverbatim
   </li>
   <li> All the satisfying assignments should be determined (if possible).
    <ol>
     <li> 24 = 4!, and so one might assume that we have one "essential"
     satisfiable node together with its 24 symmetric variations. </li>
     <li> And then it seems that the underlying hypergraph doesn't have
     non-trivial automorphisms. </li>
    </ol>
   </li>
  </ul>

/*
