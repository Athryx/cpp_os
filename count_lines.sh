#!/bin/sh

cd $(dirname $0)
grep -v '^\s*$' $(tree -fi | grep -E '.*\.[ch]pp$|.*\.asm$' | grep -v '/old/') | wc -l
