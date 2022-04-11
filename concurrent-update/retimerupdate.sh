#!/bin/sh
#Example: psuupdate.sh 215 0 /tmp/01_ECD16010092_Pri_APP_v1_6_0_0.bin 0 0
retimerfwupgrade $@ 0 0
retVal=$?
if [ $retVal -ne 0 ]; then
    echo "Failed to update Primary"
    exit $retVal
fi
retimerfwupgrade $@ 1 0
retVal=$?
if [ $retVal -ne 0 ]; then
    echo "Failed to update secondary"
    exit $retVal
fi
retimerfwupgrade $@ 2 0
retVal=$?
if [ $retVal -ne 0 ]; then
    echo "Failed to update Community"
    exit $retVal
fi

