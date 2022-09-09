#!/usr/bin/env bash
set -euo pipefail

cd _build_tests
ctest --output-on-failure
