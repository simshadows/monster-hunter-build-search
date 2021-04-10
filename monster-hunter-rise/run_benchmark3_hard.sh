#!/usr/bin/env sh

(time ./mhwrs search search_benchmarks/gs_mt_as_pp_hb3.json) 2>&1 | tee mhrbs.log
