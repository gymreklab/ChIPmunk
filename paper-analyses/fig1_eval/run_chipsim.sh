#!/bin/bash

source params.sh

factor=$1
BED=$(ls /storage/mlamkin/projects/encode_data_learn/${factor}/*.bed)
OUTPREFIX=${OUTDIR}/${factor}.chipsim

Rscript --vanilla ./run_chipsim.R ${NREADS} ${REFFA_CHR} ${BED} ${OUTPREFIX} ${READLEN}
