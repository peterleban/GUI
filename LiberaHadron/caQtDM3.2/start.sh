#!/bin/bash

# Allow large waveform readout
export EPICS_CA_MAX_ARRAY_BYTES=5000000

# Tell EPICS clients where to look for the IOCs
export EPICS_CA_ADDR_LIST="10.0.3.103 10.0.4.191 134.94.210.154 134.94.210.155 134.94.210.156 134.94.210.157 134.94.210.158 134.94.210.159"

# Call caQtDM
/home/libera/epics/caqtdm-3.9.4/caQtDM_Binaries/caQtDM -macro "DEVICE1=H2204,DEVICE2=bplus,DEVICE3=libera03,DEVICE4=libera04,DEVICE5=libera05,DEVICE6=libera06,DEVICE7=libera07,DEVICE8=libera08,DEVICE9=libera09,DEVICE10=libera10" selection.ui
