#!/bin/sh

echo -e 'Version: $Id: gfast_env.sh 000 2023-01-11 18:00:00Z ulbergc $\n'

# With "LEAPSECONDS=", times will be handled consistently internally
# pending addressing iscl <-> dmlib issues.
export LEAPSECONDS=no-correction-for-gfast
# export LEAPSECONDS=/app/share/etc/leapseconds
#env | grep LEAPSECONDS

source /app/eewdata/run/bin/ew_linux.bash
#env | sort | grep EW
