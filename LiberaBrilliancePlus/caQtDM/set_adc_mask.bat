#!/bin/bash

[ $# -lt 1 ] && {

    echo "Usage: $0 iocname"

    exit 255

}

PV=$1

len=${#PV};

caput -a "${PV::len-1}:tbt:adc_mask" $(cat adc_mask)
