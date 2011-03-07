This repository hosts code for my "Serendipitous Prodigalities" blog.

#1. A little theory and a small case study on optimizing code for run-time efficiency.#


We will analyse code developpement for a language identification program using ngrams. The focus will solely be on run-time efficiency, not language classification accuracy.

##1.1. Using domain-specific knowledge UTF-8, UTF-32 and custom storage for n-grams representations.##


###1.1.1. First implementations###

cf. ngrams\_counter\_[utf8|utf32|bitfields].hxx

###1.1.2. Profiler-driven tuning###

Using pref, ad cache/callgrind
cf. BASIC\_UTF8\_CMP, CUSTOM\_ARRAY\_ROTATE

##1.2. Using domain-specific knowledge : standard unordered_map and custom perfect hashing for n-gram matching.##
TODO, using gperf, llvm switch statement

##1.3. Parallelization granularity : processes/threads, intra/inter files##
TODO, using [GNU parallel](http://linuxsoftware.co.nz/wiki/Parallel "GNU parallel") and OpenMP (+ CPU affinity) or std::threads
