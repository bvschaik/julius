#!/bin/bash

cd build && find . -name '*.gcda' -exec rm '{}' \; && make && make test && cd .. && ./coverage.sh 
