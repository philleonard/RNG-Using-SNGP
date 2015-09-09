RNG Using GP
==================================================

Final Year Project Appendix. Including code, documents (dissertation, reports, presentations etc..), LaTeX files and test data.

A comparison of GP and SNGP for evolving RNGs, and a comparison of RNGs produced through evolutionary means against other common RNGs. 

[Dissertation](/Project%20Documents/dissertation.pdf?raw=true)

[ACM GECCO 2015 Paper](http://dl.acm.org/citation.cfm?id=2754820&dl=ACM&coll=DL&CFID=711530688&CFTOKEN=30217449)

##Abstract
Random Number Generators are an important aspect of many modern day software systems, cryptographic protocols and modelling techniques. To be more accurate, it is Pseudo Random Number Generators (PRNGs) that are more commonly used over their expensive, and less practical hardware based counterparts. Given that PRNGs rely on some deterministic algorithm (typically a Linear Congruential Generator) we can leverage Shannon's theory of information as our fitness function in order to generate these algorithms by evolutionary means. In this paper we compare traditional Genetic Programming (GP) against its graph based implementation, Single Node Genetic Programming (SNGP), for this task. We show that with SNGPs unique program structure and use of dynamic programming, it is possible to obtain smaller, higher entropy PRNGs, over six times faster and produced at a solution rate twice that achieved using Koza's standard GP model. We also show that the PRNGs obtained from evolutionary methods produce higher entropy outputs than other widely used PRNGs and Hardware RNGs (specifically recordings of atmospheric noise), as well as surpassing them in a variety of other statistical tests presented in the NIST RNG test suite.
