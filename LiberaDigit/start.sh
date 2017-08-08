#!/bin/bash

[ $# -lt 1 ] && {
    echo "Usage: $0 IOCname $1 IPaddress"
    exit 255
}

DEVICE=$1
IP=$2
#IP=10.0.3.106

# Allow large waveform readout
export EPICS_CA_MAX_ARRAY_BYTES=5000000

# Tell EPICS clients where to look for the IOCs
export EPICS_CA_ADDR_LIST=${IP}

# Call caQtDM, specify IOC name and main window to open
/home/libera/epics/caqtdm-3.9.4/caQtDM_Binaries/caQtDM -macro "DEVICE=${DEVICE}" main.ui
