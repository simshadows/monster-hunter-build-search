#!/usr/bin/env sh

(time ./mhrbs search search_benchmarks/gs_mt_as_pp.json) 2>&1 | tee mhrbs.log
