#!/bin/sh

cd $(dirname $0)
grep -v '^\s*$' $(tree -fi | grep -E '.*\.[ch]$|.*\.asm$' | grep -v '/old/') | wc -l
