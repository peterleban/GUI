# Script for setting geometrical coefficients. Please note that first field in the array "16"
# defines number of fields in the array. Command is composed from the following items
#
# caput -a IOC_NAME:BPM_NAME:calibration:k:X_Y_PLANE:FREQ_HARMONIC 16 K0 K1 K2 K3 K4 K5 K6 K7 K8 K9 K10 K11 K12 K13 K14 K15 


# KX geometrical coefficients for for frequency harmonic 1
caput -a LSPH2204:bpm3:calibration:k:x:f1 16 0 0 0 0 10000000 0 0 0 0 0 0 0 0 0 0 0
# KX geometrical coefficients for for frequency harmonic 2
caput -a LSPH2204:bpm3:calibration:k:x:f2 16 0 0 0 0 10000000 0 0 0 0 0 0 0 0 0 0 0


# KY geometrical coefficients for for frequency harmonic 1
caput -a LSPH2204:bpm3:calibration:k:y:f1 16 0 10000000 0 0 0 0 0 0 0 0 0 0 0 0 0 0
# KY geometrical coefficients for for frequency harmonic 2
caput -a LSPH2204:bpm3:calibration:k:y:f2 16 0 10000000 0 0 0 0 0 0 0 0 0 0 0 0 0 0 
