#!/bin/bash

echo "libproteus.so tests..."
LD_LIBRARY_PATH=. ./proteus_tests
echo
echo "libproteus.a tests..."
./proteus_tests_static
