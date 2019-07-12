#!/bin/bash
g++ -Og -o brokersaurusrex \
  main.cc \
  rexitem.cc \
  worker.cc \
  pool.cc \
  -lre2 -lpthread
