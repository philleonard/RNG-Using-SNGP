Random Number Generation Using Genetic Programming
==================================================

Final Year Project Appendix. Including code, documents (dissertation, reports, presentations etc..), LaTeX files and test data.

A comparison of GP and SNGP for evolving RNGs, and a comparison of RNGs produced through evolutionary means against other common RNGs. 
##Abstract

This is the final report for the project; `Random Number Generation Using Genetic Programming' by Philip Leonard, supervised by Dr David Jackson (primary supervisor) and Professor Paul Dunne (secondary supervisor). This document covers all aspects of the project from the introduction and background, to the design implementation and analysis of the project in order to give the reader an understanding of how I implemented the project and what things I found when evaluating the project.

This report demonstrates that using methods described by Koza in [1], it is possible to genetically breed Random Number Generators that produce sequences of pseudo random bits with near maximal entropy. This report more importantly shows that it is possible to produce better results using the Single Node Genetic Programming methodology described by Jackson in [2]. As well as this, this report also demonstrates that the Random Number Generators Produced genetically by these methods also produce sequences of random bits with higher entropy than other widely used RNGs. It not only outperforms the C programming Linear Congruent Generator algorithm implemented in the rand() function, but can also perform better than a "True" Random Number Generator that creates random bit sequences from observed atmospheric noise.

This GitHub repository contains the implemetations for all methods of producing random numbers as described above. It also contains the latex files for all of the accompanying documents for the project.

[1] John R. Koza, Evolving a Computer Program to Generate Random Numbers Using the Genetic Programming Paradigm. Stanford University, 1991.

[2] David Jackson, Single Node Genetic Programming on Problems with Side Effects University of Liverpool, 2012.
