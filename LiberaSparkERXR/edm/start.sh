#!/bin/sh

export EPICS_HOST_ARCH=linux-x86
export EPICS_TOPDIR=/home/libera/epics

# For large data transfers - e.g. DD data set this
export EPICS_CA_MAX_ARRAY_BYTES=5000000

# Export EPICS install bin folder to PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$EPICS_TOPDIR/install/lib:$EPICS_TOPDIR/install/lib/$EPICS_HOST_ARCH
export PATH=$PATH:$EPICS_TOPDIR/install/bin/$EPICS_HOST_ARCH

export EDM_TOPDIR=$EPICS_TOPDIR/sparkr/edm
export EDMDATAFILES=$EDM_TOPDIR/panels
export EDMOBJECTS=$EDM_TOPDIR/prefs
export EDMFILES=$EDM_TOPDIR/prefs
export EDMPVOBJECTS=$EDM_TOPDIR/prefs
export EDMHELPFILES=$EPICS_TOPDIR/base-3.14.10/extensions/src/edm/helpFiles

echo "EPICS host   : $EPICS_HOST_ARCH"
echo "EPICS folder : $EPICS_TOPDIR"
echo "EDM panels folder : $EDMDATAFILES"
echo "EDM prefs files  : $EDMFILES"

cd $EDMDATAFILES
edm -m "HOSTNAME=spark1" -x main.edl

exit $?

