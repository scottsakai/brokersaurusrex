#!/bin/bash
g++ -Og -ggdb  -o brokersaurusrex \
  main.cc \
  rexitem.cc \
  worker.cc \
  pool.cc \
  -lbroker \
  -lcaf_core \
  -lre2 -lpthread
