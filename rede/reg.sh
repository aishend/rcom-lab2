#!/bin/bash
# Corre um comando e regista-o no txt da experiencia (comando + resultado)
# Uso: bash reg.sh <experiencia> <comando>
#   ex.: bash reg.sh 1 ping -c 5 172.16.10.254
#        bash reg.sh 1 sudo arp -d 172.16.10.254

EXP=$1
shift
FICHEIRO=exp${EXP}_$(hostname).txt

echo "[$(date +%H:%M:%S)] $(hostname)\$ $*" | tee -a $FICHEIRO
"$@" 2>&1 | tee -a $FICHEIRO
echo | tee -a $FICHEIRO
