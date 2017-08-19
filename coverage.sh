#!/bin/bash

lcov --rc lcov_branch_coverage=1 --capture --directory . --output-file build/coverage.info
genhtml build/coverage.info --branch-coverage --output-directory build/coverage
