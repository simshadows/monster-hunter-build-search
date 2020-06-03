#!/usr/bin/env sh

(time ./mhwibs search search_benchmarks/gs_mt_as.json) 2>&1 | tee mhwibs.log
