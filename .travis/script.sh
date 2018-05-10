#!/bin/bash

cd /build/
./configure && make && ./src/ui-tests/ui-tests
