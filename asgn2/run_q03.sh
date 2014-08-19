#!/bin/bash
ipcs -q | tail -n+4 | tr -s ' ' |cut -d ' ' -f 3 | parallel "ipcrm -Q {}"
