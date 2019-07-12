#!/bin/bash
g++ -O3 -o brokersaurusrex \
  main.cc \
  rexitem.cc \
  worker.cc \
  pool.cc \
  -lre2 -lpthread
