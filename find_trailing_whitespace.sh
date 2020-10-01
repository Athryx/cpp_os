#!/bin/sh

cd $(dirname $0)
grep -n '^\s*$' $(tree -fi | grep -E '.*\.[ch]$|.*\.asm$' | grep -v '/old/')
