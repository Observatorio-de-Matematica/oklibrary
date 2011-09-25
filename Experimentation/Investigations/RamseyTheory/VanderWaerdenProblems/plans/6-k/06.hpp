// Oliver Kullmann, 16.2.2009 (Swansea)
/* Copyright 2009, 2010, 2011 Oliver Kullmann
This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as published by
the Free Software Foundation and included in this library; either version 3 of the
License, or any later version. */

/*!
  \file Experimentation/Investigations/RamseyTheory/VanderWaerdenProblems/plans/6-k/06.hpp
  \brief On investigations into vanderwaerden_2(6) = 1132

  Instance created by output_vanderwaerden2nd_stdname(6,6,1132) at
  Maxima-level, or by "VanderWaerden-O3-DNDEBUG 6 6 1132" at C++ level
  (instances differ by the order of negated literals in clauses).


  \todo Basic statistics for n=1132
  <ul>
   <li> n=1132, c=255154, l=1530924, all clauses of length 6. </li>
  </ul>


  \todo Local search solvers
  <ul>
   <li> n = 1000
    <ul>
     <li> ubcsat-irots seems best. </li>
     <li> But finding a solution seems hopeless; less than 1400 falsified
     clauses seems to require billions of steps --- the difference between
     100,000 and 10,000,000 steps is very small. </li>
     <li> We need to look into the Heule-et-al article. </li>
     <li> Finding the best ubcsat-algorithm for new ubcsat-version:
     \verbatim
> E=run_ubcsat("VanDerWaerden_2-6-6_1131.cnf",runs=100,cutoff=10000)
> eval_ubcsat_dataframe(E)
1. irots:
1969 1970 1972 1973 1975 1979 1980 1981 1983 1985 1988 1993 1995 1996 1998 1999
   2    1    1    1    1    1    1    3    1    1    2    1    1    2    3    1
2000 2002 2003 2004 2005 2006 2007 2008 2010 2011 2014 2015 2016 2017 2018 2019
   2    2    1    3    1    1    1    1    1    2    2    3    2    1    3    7
2020 2021 2022 2023 2024 2025 2026 2027 2028 2029 2030 2032 2034 2035 2037 2038
   4    1    2    3    1    3    2    1    1    2    1    2    4    1    1    1
2039 2041 2044 2045 2046 2048 2051 2052 2053 2058 2061
   1    4    1    1    1    1    1    1    1    1    1
fps: 20227
2. samd:
1986 2001 2006 2007 2013 2018 2022 2023 2024 2026 2029 2030 2031 2032 2034 2035
   1    1    1    2    1    1    1    1    1    2    2    1    1    1    1    2
2036 2037 2038 2039 2041 2042 2043 2044 2045 2046 2047 2048 2050 2051 2054 2055
   2    1    5    2    1    3    1    1    2    3    3    1    1    2    2    2
2056 2057 2058 2059 2060 2061 2062 2063 2066 2067 2069 2071 2073 2074 2075 2076
   1    1    1    1    2    3    2    1    3    1    2    2    1    1    1    2
2077 2078 2079 2082 2084 2085 2086 2087 2088 2089 2091 2092 2093 2096 2103 2106
   1    2    1    2    1    2    1    1    1    2    1    1    1    1    1    1
2108 2110 2114
   1    1    1
fps: 28910
3. ag2wsat:
2001 2005 2011 2012 2013 2014 2017 2018 2019 2020 2021 2022 2023 2024 2026 2028
   1    1    1    2    1    3    2    3    2    2    1    1    1    1    4    1
2029 2030 2031 2032 2033 2035 2036 2037 2038 2039 2040 2041 2042 2043 2044 2045
   2    3    3    1    3    5    4    2    3    1    2    4    1    3    2    2
2046 2048 2049 2050 2051 2052 2053 2054 2055 2056 2060 2061 2062 2067 2069 2072
   2    2    2    4    2    3    1    5    2    2    1    1    1    1    2    1
fps: 13541
4. gsatt:
2001 2008 2009 2011 2013 2014 2015 2017 2018 2019 2020 2021 2022 2024 2026 2027
   1    1    1    1    1    1    1    1    2    1    1    2    1    1    1    1
2028 2030 2031 2032 2033 2035 2037 2039 2042 2043 2044 2045 2046 2047 2048 2049
   2    1    2    1    2    1    3    1    1    1    1    2    1    1    3    1
2052 2053 2054 2055 2056 2058 2059 2061 2062 2065 2066 2067 2068 2071 2072 2074
   3    4    5    1    4    1    4    2    2    2    2    2    1    2    2    2
2075 2076 2077 2079 2082 2084 2085 2087 2088 2089 2090 2091 2097 2105 2150
   1    3    1    1    1    2    2    1    1    1    1    1    1    1    1
fps: 28588
5. rots:
2003 2011 2015 2016 2017 2024 2025 2026 2029 2031 2032 2033 2034 2035 2036 2037
   1    1    1    4    1    1    2    1    1    2    3    1    1    1    1    1
2038 2039 2040 2041 2042 2043 2044 2045 2046 2047 2048 2050 2052 2053 2055 2057
   1    1    3    1    4    1    5    1    2    2    1    3    4    1    4    2
2058 2059 2060 2061 2062 2063 2066 2067 2068 2069 2070 2071 2072 2074 2075 2076
   1    2    1    1    1    3    1    1    1    1    1    1    1    1    4    1
2078 2079 2080 2085 2086 2087 2093 2096 2097 2099 2102 2105 2108 2111 2116 2123
   1    2    1    1    1    1    2    1    1    1    1    1    1    1    1    2
fps: 28169
6. ddfw:
2023 2024 2028 2029 2031 2032 2033 2034 2035 2036 2037 2038 2039 2040 2041 2042
   1    1    2    2    1    1    2    1    1    1    2    2    1    3    4    2
2043 2044 2045 2046 2047 2048 2049 2050 2051 2052 2053 2054 2055 2056 2057 2058
   1    2    1    1    2    2    2    2    2    5    6    1    4    2    2    3
2059 2060 2061 2062 2063 2064 2065 2066 2067 2068 2069 2071 2072 2073 2074 2075
   2    3    2    2    5    2    2    3    3    1    2    1    1    1    1    1
2076 2077
   2    1
fps: 1724
7. g2wsat:
2030 2033 2037 2042 2045 2049 2050 2051 2052 2053 2054 2055 2056 2057 2058 2059
   1    1    1    2    3    1    1    2    2    2    1    1    1    1    1    2
2060 2062 2063 2064 2065 2066 2067 2068 2070 2071 2072 2073 2074 2075 2076 2077
   3    3    5    2    4    2    3    5    2    2    2    4    3    2    2    2
2078 2079 2080 2081 2082 2083 2084 2085 2086 2087 2088 2089 2090 2092 2093 2096
   1    6    1    2    2    1    1    1    1    3    3    1    2    1    1    2
2099 2100
   1    1
fps: 15701
8. sapsnr:
2044 2069 2081 2084 2090 2091 2092 2094 2095 2096 2097 2099 2100 2101 2102 2103
   1    1    1    1    1    1    2    1    2    3    2    2    2    1    3    3
2104 2105 2106 2107 2108 2109 2110 2111 2112 2113 2114 2115 2116 2117 2118 2119
   2    1    6    3    3    2    2    3    4    2    3    2    4    3    7    5
2120 2121 2122 2123 2125 2126 2127 2129 2130 2131 2133 2139
   3    5    1    1    1    1    1    1    4    1    1    1
fps: 10934
9. rsaps:
2061 2062 2069 2074 2076 2077 2078 2079 2084 2085 2086 2087 2089 2090 2091 2092
   1    1    1    1    1    2    1    1    1    2    1    1    1    4    1    1
2093 2097 2098 2099 2100 2101 2102 2103 2104 2105 2106 2107 2108 2109 2110 2111
   2    6    3    2    5    3    1    2    3    3    2    4    3    3    1    4
2112 2113 2115 2116 2117 2118 2119 2120 2122 2124 2128 2129
   3    5    2    3    2    3    2    1    2    6    1    2
fps: 11016
10. hwsat:
2066 2083 2115 2117 2119 2121 2122 2126 2127 2132 2135 2136 2139 2141 2142 2146
   1    1    1    1    1    1    1    3    1    2    1    2    1    2    1    1
2148 2149 2150 2151 2153 2154 2158 2159 2160 2162 2163 2164 2165 2166 2167 2168
   1    1    1    2    3    2    1    2    1    3    3    2    2    2    1    2
2169 2171 2172 2173 2174 2176 2177 2179 2183 2184 2186 2187 2189 2190 2191 2193
   2    2    1    3    1    2    1    1    3    2    1    2    4    3    1    1
2194 2195 2196 2200 2201 2203 2207 2209 2218 2221 2225 2227 2230 2235 2236 2249
   1    1    2    1    1    2    1    2    1    1    1    1    2    1    1    1
fps: 32852
11. saps:
2076 2077 2081 2082 2085 2086 2089 2090 2092 2094 2095 2096 2097 2098 2100 2101
   1    1    1    1    3    1    2    1    2    1    1    2    1    1    1    4
2102 2103 2104 2105 2106 2107 2108 2109 2110 2111 2112 2113 2114 2115 2116 2117
   2    1    4    5    4    4    4    3    3    4    2    3    2    3    4    3
2118 2119 2120 2122 2124 2125 2126 2128 2129 2130 2131
   3    3    1    2    2    4    4    2    1    2    1
fps: 10988
12. gwsat:
2085 2090 2097 2098 2102 2103 2105 2106 2107 2110 2112 2113 2114 2116 2117 2118
   1    1    1    1    1    1    1    2    1    2    1    1    1    2    3    1
2119 2120 2121 2122 2123 2124 2125 2126 2128 2129 2130 2131 2133 2134 2135 2138
   5    2    1    1    2    2    6    2    3    2    1    2    3    1    5    1
2140 2141 2142 2143 2144 2145 2146 2147 2148 2150 2151 2152 2155 2156 2159 2161
   1    1    1    1    1    1    1    1    1    2    1    1    3    3    1    1
2163 2164 2165 2167 2170 2172 2174 2175 2178 2182 2185 2192 2195
   4    1    3    1    1    2    1    1    1    1    1    1    1
fps: 16158

> E=run_ubcsat("VanDerWaerden_2-6-6_1131.cnf",runs=100,cutoff=1000000)
> eval_ubcsat_dataframe(E)
1. irots:
1397 1932 1942 1944 1951 1957 1958 1959 1960 1961 1962 1963 1964 1965 1966 1967
   1    1    1    1    1    1    3    1    1    2    1    2    2    3    2    1
1968 1969 1970 1971 1972 1973 1974 1975 1976 1977 1979 1980 1981 1982 1983 1984
   1    5    2    1    5    6    2    1    1    3    3    1    5    7    3    5
1985 1986 1987 1988 1989 1990 1993 1997 2000
   6    6    1    4    2    1    2    2    1
fps: 25393
2. rots:
1919 1929 1933 1935 1936 1939 1942 1943 1944 1946 1949 1950 1951 1952 1953 1954
   1    1    1    1    1    1    1    2    1    1    1    2    1    1    1    2
1955 1957 1958 1959 1960 1961 1962 1963 1965 1966 1967 1968 1970 1971 1972 1973
   1    2    1    1    2    3    2    1    2    2    2    3    3    1    1    2
1974 1975 1976 1977 1978 1979 1980 1981 1982 1983 1984 1986 1987 1988 1989 1990
   1    3    1    3    4    1    1    2    2    1    2    2    1    4    1    1
1991 1992 1993 1994 1995 1997 1998 2002 2004 2005 2019 2028
   2    2    2    4    3    3    1    1    1    1    1    1
fps: 36751
3. ag2wsat:
1920 1934 1939 1940 1941 1942 1943 1945 1946 1947 1950 1951 1952 1954 1955 1956
   1    1    1    1    2    2    1    1    1    1    1    4    4    1    3    4
1957 1958 1959 1960 1961 1962 1963 1964 1965 1966 1967 1968 1969 1970 1971 1972
   3    5    2    5    3    6    5    4    1    1    4    4    7    2    2    3
1973 1974 1976 1977 1978 1982 1984
   3    3    2    2    1    2    1
fps: 19958
4. gwsat:
1928 1929 1930 1932 1933 1934 1935 1936 1937 1938 1939 1940 1942 1943 1945 1946
   1    1    1    1    1    1    1    3    1    1    3    1    1    3    1    4
1947 1948 1949 1950 1951 1952 1953 1954 1955 1956 1957 1958 1959 1960 1961 1962
   3    2    1    2    3    3    2    2    4    2    1    4    3    2    5    1
1963 1964 1965 1966 1968 1969 1970 1971 1974 1975 1976 1977 1979 1981 1984 1986
   3    5    2    2    3    1    2    3    2    1    3    2    1    1    1    1
1987 1988
   1    1
fps: 22336
5. gsatt:
1944 1945 1947 1952 1955 1957 1958 1962 1963 1966 1970 1972 1974 1975 1976 1977
   1    1    2    1    1    1    1    2    1    1    1    2    1    1    2    1
1980 1981 1982 1984 1986 1987 1989 1990 1992 1993 1994 1995 1996 1997 1998 2002
   1    2    4    4    2    1    3    1    2    1    2    2    5    3    3    6
2003 2004 2005 2006 2007 2008 2010 2011 2012 2013 2015 2016 2018 2019 2020 2021
   1    3    1    2    3    1    1    1    4    1    1    4    1    2    1    1
2022 2023 2024 2025 2026 2029 2033 2036 2040
   2    1    1    1    1    1    1    1    1
fps: 54361
6. samd:
1944 1952 1955 1960 1961 1970 1971 1972 1973 1974 1975 1978 1979 1981 1982 1983
   1    1    1    2    1    1    1    3    1    1    2    4    2    1    2    1
1984 1985 1987 1988 1989 1990 1991 1992 1993 1995 1996 1997 1998 2000 2001 2002
   1    2    1    1    4    1    4    5    1    1    3    1    4    3    3    2
2003 2005 2006 2008 2009 2010 2011 2012 2013 2014 2017 2019 2020 2021 2023 2024
   3    1    2    2    1    3    2    2    1    1    2    1    3    1    2    1
2027 2028 2031 2034 2036 2041 2050 2054
   2    1    1    1    2    1    1    1
fps: 54355
7. g2wsat:
1992 1994 1995 1999 2002 2003 2004 2007 2008 2009 2010 2011 2012 2013 2014 2015
   2    2    2    1    2    1    2    2    1    2    2    2    3    3    3    6
2016 2017 2018 2019 2020 2021 2022 2023 2024 2025 2026 2027 2029 2030 2031 2032
   3    5    9    2    6    3    6    9    4    3    3    4    2    1    2    1
2034
   1
fps: 20351
8. ddfw:
1996 1997 2000 2002 2004 2005 2006 2007 2008 2009 2010 2011 2012 2013 2014 2015
   1    1    1    1    1    1    2    4    1    3    2    7    3    3    6    5
2016 2017 2018 2019 2020 2021 2022 2023 2024 2025 2027 2029 2030
   7    4    3    5   10    6    3    9    5    2    2    1    1
fps: 2440
     \endverbatim
     We see that all algorithms show only small improvements, except of the one
     outlier for irots, reaching min=1397. </li>
     <li> So the new ubcsat-algorithms don't improve anything here, and local
     search performs poorly. While the palindromic problems seem (modulo
     segmentation-fault problems) to perform much better, and the palindromic
     gap is also zero here! </li>
    </ul>
   </li>
  </ul>


  \todo Upper bounds
  <ul>
   <li> n = 1132
    <ol>
     <li> OKsolver-2002
     \verbatim
> OKsolver_2002-O3-DNDEBUG -M -D30 VanderWaerden_2_6_1132.cnf
     \endverbatim
     looks hopeless (no node of depth 30 solved in one hour). </li>
    </ol>
   </li>
  </ul>


  \todo Determining the best ubcsat-algorithm for palindromic problems
  <ul>
   <li> Cutoff=10^6:
   \verbatim
> E=run_ubcsat("VanDerWaerden_pd_2-6-6_567.cnf",runs=100,cutoff=1000000)
WARNING[run_ubcsat]: There have been 2 errors with the following algorithms:
 saps, sapsnr .

> plot(E$alg,E$min)
> eval_ubcsat_dataframe(E)
rsaps :
  0   1 139 148 155 157 159 160 161 163 164 165 166 168 169
 45  31   1   1   1   2   3   2   2   5   2   1   2   1   1
paws :
  0   1 150 153 155 157 158 159 160 161 162 163 164 165 166 167 168 169 170 171
  4   7   1   1   1   1   1   1   2   1   3   6   2  16  12  10  17   9   3   1
172
  1
ddfw :
  0   1 111 116 117 118 119 120 121 122 123 124 128 134 141 142 143 144 145 146
  3  12   1   1  12   2   4   5   3   3   3   3   1   1   1   1   3   2   1   2
147 148 149 150 151 152
  8  11   5   7   3   2
ag2wsat :
  0   1 109 111 116 117 118 119 120 121 122 123 124 125 138 142 143 144 145 146
  2   7   3   4   2   6   5   5   4   4   2   5   4   1   1   1   1   4   6   6
147 148 149 150 151 152 153
  6   9   4   2   2   3   1
rots :
  0   1 117 119 120 121 122 123 124 136 137 138 139 140 141 142 143 144 145 146
  1   3   1   1   1   1   1   2   1   1   2   3   4   9   6  13  18  17   5  10
g2wsat :
  0   1 114 119 120 122 123 124 125 126 127 128 129 130 131 132 146 148 151 153
  1   2   1   2   2   4   1   1   2   2   2   2   1   1   1   2   1   1   3   4
154 155 156 157 158 159 160 161
  5  14   9  11  12   8   4   1
rnov :
  0   1 128 130 132 133 134 138 140 141 149 158 171 176 177 178 179 180 181 182
  1   2   1   2   1   1   2   2   1   1   1   1   1   1   3   2   4   2   4   2
183 184 185 186 187 188 189
  8  18   3   9  18   5   4
rnovp :
  0   1 131 133 138 139 140 141 143 154 173 176 177 178 179 180 181 182 183 184
  1   1   1   1   1   2   2   1   1   1   2   3   6   5   6   1   4   8  11   9
185 186 187 188 189 191
 11   4   9   7   1   1
irots :
  1 110 111 113 114 115 118 119 120 121 122 123 124 125 126 128 130 133 145 146
  4   1   3   1   1   1   1   1   7   4   3   5   4   2   1   2   1   1   4   5
dano :
  1 123 124 125 126 131 134 135 137 138 139 141 146 153 162 174 175 177 178 179
  5   1   1   2   1   1   1   3   1   1   2   1   2   1   1   1   1   1   2   7
180 181 182 183 184 185 186 187 188 189 190
  6   7   6   8   8  10   6   6   4   1   2
anovpp :
  1 120 128 132 133 134 137 143 166 168 170 171 172 173 174 175 176 177 178 179
  3   1   1   3   2   2   1   1   1   1   1   2   2   1   3   8   3   7  11   9
anovp :
  1 117 120 121 122 127 129 132 133 134 137 146 151 157 159 168 169 171 172 173
  2   2   1   1   1   1   1   1   3   3   2   1   1   1   1   1   1   3   1   1
174 175 176 177 178 179 180 181 182 183 184 185 186 187
  1   3   2   7   5   8   8  11   9   4   7   4   1   1
   \endverbatim
   rsaps the clear winner (ignoring for now the old problems with segfaulting
   saps, sapsnr). </li>
  </ul>


  \todo vdw_2^pd(6,6) = (567,1132)
  <ul>
   <li> Certificates:
    <ol>
     <li> n=566:
     \verbatim
1,2,3,5,8,9,10,12,14,15,
16,17,19,23,26,27,29,31,32,33,
37,42,45,50,51,52,53,54,57,58,
61,62,63,64,65,70,73,78,82,83,
84,86,88,89,92,96,98,99,100,101,
103,105,106,107,110,112,113,115,116,118,
121,122,123,125,127,128,129,130,132,136,
139,140,142,144,145,146,150,155,158,163,
164,165,166,167,170,171,174,175,176,177,
178,183,186,191,195,196,197,199,201,202,
205,209,211,212,213,214,216,218,219,220,
223,225,226,228,229,231,234,235,236,238,
240,241,242,243,245,249,252,253,255,257,
258,259,263,268,271,276,277,278,279,280,
283
     \endverbatim
     or
     \verbatim
4,6,7,11,13,18,20,21,22,24,
25,28,30,34,35,36,38,39,40,41,
43,44,46,47,48,49,55,56,59,60,
66,67,68,69,71,72,74,75,76,77,
79,80,81,85,87,90,91,93,94,95,
97,102,104,108,109,111,114,117,119,120,
124,126,131,133,134,135,137,138,141,143,
147,148,149,151,152,153,154,156,157,159,
160,161,162,168,169,172,173,179,180,181,
182,184,185,187,188,189,190,192,193,194,
198,200,203,204,206,207,208,210,215,217,
221,222,224,227,230,232,233,237,239,244,
246,247,248,250,251,254,256,260,261,262,
264,265,266,267,269,270,272,273,274,275,
281,282
     \endverbatim
     </li>
     <li> n=1131:
     \verbatim
2,7,8,10,11,12,13,14,16,21,
25,26,32,35,39,41,42,43,47,49,
50,52,54,55,58,59,62,64,67,69,
70,71,75,77,78,79,81,82,84,85,
86,87,88,91,92,93,95,96,97,98,
100,106,109,110,111,112,114,116,117,118,
119,122,128,130,131,132,133,135,136,137,
140,141,142,143,144,146,147,149,150,151,
153,157,158,159,161,164,166,169,170,173,
174,176,178,179,181,185,186,187,189,193,
196,202,203,207,212,214,215,216,217,218,
220,221,226,228,233,234,236,237,238,239,
240,242,247,251,252,258,261,265,267,268,
269,273,275,276,278,280,281,284,285,288,
290,293,295,296,297,301,303,304,305,307,
308,310,311,312,313,314,317,318,319,321,
322,323,324,326,332,335,336,337,338,340,
342,343,344,345,348,354,356,357,358,359,
361,362,363,366,367,368,369,370,372,373,
375,376,377,379,383,384,385,387,390,392,
395,396,399,400,402,404,405,407,411,412,
413,415,419,422,428,429,433,438,440,441,
442,443,444,446,447,452,453,454,459,460,
462,463,464,465,466,468,473,477,478,484,
487,491,493,494,495,499,501,502,504,506,
507,510,511,514,516,519,521,522,523,527,
529,530,531,533,534,536,537,538,539,540,
543,544,545,547,548,549,550,552,558,561,
562,563,564
     \endverbatim
     </li>
    </ol>
   </li>
   <li> Established by "CRunPdVdWk1k2 6 6 minisat-2.2.0". </li>
   <li> Evaluation:
   \verbatim
> E=read_crunpdvdw_minisat()
$cfs:
    Min.  1st Qu.   Median     Mean  3rd Qu.     Max.
       0    13620   390600  1006000  1089000 51400000
sd= 2467837
     95%      96%      97%      98%      99%     100%
 3771146  4689686  5342741  7541277  9834239 51398334
sum= 851449200
   \endverbatim
   Not showing times, since apparently the processor switched frequencies
   (csltok); should be rerun. </li>
   <li> The even problems look harder (as usual, before the second phase where
   only one parity is considered). </li>
   <li> Interesting the large palindromic span, and that the palindromic gap
   is zero here. </li>
   <li> So here using palindromic problems to get solutions for the ordinary
   problems is very successful. </li>
   <li> "RunPdVdWk1k2 6 6 gsat-tabu 100 10000000" yields
   \verbatim
Break point 1: 566
Break point 2: 567
> E=read_ubcsat("VanDerWaerden_pd_2-6-6_567.cnf_OUT",nrows=100)
125 138 139 140 141 142 143 144 145 146 147 148 149
  1   3   1  15   9  10  12  19  17   7   3   2   1
100
> E=read_ubcsat("VanDerWaerden_pd_2-6-6_568.cnf_OUT",nrows=100)
107 112 123 129 130 134 135 137 138 139 140 141 142 143 144 145 146 147 148 149
  2   1   3   1   2   1   1   1   5   2   4   6  16  12  15   5   7   6   5   3
151 155
  1   1
100
   \endverbatim
   n=246 seems to be the first harder problem, all other problems for n<=566
   are easy. n=567 is actually satisfiable, while n=568 is unsatisfiable.
   </li>
   <li> "RunPdVdWk1k2 6 6 rsaps 100 10000000" yields --- unfortunately we get
   segmentation faults (csltok). We need to see whether there are news from the
   Ubcsat-group:
   \verbatim
Break point 1: 567
Break point 2: 788
# both breakpoints due to segmentation faults (also on the runs without
# initial solutions, and also with ubcsat version beta-10)
   \endverbatim
   </li>
   <li> Using a 32-bit machine (cstriton):
   \verbatim
RunPdVdWk1k2 6 6 rsaps 100 10000000
Break point 1: 567
Break point 2: 1132
   \endverbatim
   All satisfying assignments found in the first run with the old solution,
   max osteps = 351989. The two unsatisfiable instances:
   \verbatim
> E=read_ubcsat("VanDerWaerden_pd_2-6-6_568.cnf_OUT",nrows=100)
  1 108 109 110 111 112 113 114 116 117 118 119
 75   1   2   3   2   3   2   3   3   4   1   1
100
> E=read_ubcsat("VanDerWaerden_pd_2-6-6_1133.cnf_OUT",nrows=100)
   1  492 1021 1023 1025 1027 1028 1030 1031 1032 1033 1034 1035 1036 1037 1038
  55    1    2    1    2    1    2    2    7    2    7    3    3    3    4    3
1041 1044
   1    1
100
   \endverbatim
   </li>
  </ul>

*/

