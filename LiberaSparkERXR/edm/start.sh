#!/bin/sh

[ $# -lt 1 ] && {
    echo "Usage: $0 IOCname $1 IPaddress"
    exit 255
}

DEVICE=$1
IP=$2

export EPICS_CA_ADDR_LIST=$IP

export EPICS_HOST_ARCH=linux-x86
export EPICS_BASE=base-3.14.12.2
export EPICS_TOPDIR=/home/libera/epics/

# For large data transfers - e.g. DD data set this
export EPICS_CA_MAX_ARRAY_BYTES=5000000

# Export EPICS install bin folder to PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$EPICS_TOPDIR/$EPICS_BASE/lib:$EPICS_TOPDIR/$EPICS_BASE/lib/$EPICS_HOST_ARCH
export PATH=$PATH:$EPICS_TOPDIR/$EPICS_BASE/bin/$EPICS_HOST_ARCH

export EDM_TOPDIR="$(readlink -f "$(dirname "$0")")"
export EDMDATAFILES=$EDM_TOPDIR/panels
export EDMOBJECTS=$EDM_TOPDIR/prefs
export EDMFILES=$EDM_TOPDIR/prefs
export EDMPVOBJECTS=$EDM_TOPDIR/prefs
export EDMHELPFILES=$EPICS_TOPDIR/$EPICS_BASE/extensions/src/edm/helpFiles

echo "EPICS host   : $EPICS_HOST_ARCH"
echo "EPICS folder : $EPICS_TOPDIR/$EPICS_BASE"
echo "EDM panels folder : $EDMDATAFILES"
echo "EDM prefs files  : $EDMFILES"

cd $EDMDATAFILES
edm -m "HOSTNAME=$DEVICE" -x main.edl

exit $?

