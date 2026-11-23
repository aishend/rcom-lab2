#!/bin/bash
# Guarda o estado da rede deste tux num txt para o relatorio
# Uso: bash guardar.sh <experiencia>   (ex.: bash guardar.sh 1)

EXP=$1
FICHEIRO=exp${EXP}_$(hostname).txt

# >> acrescenta ao fim, assim podemos correr varias vezes na mesma experiencia
{
    echo "===== $(hostname) - exp $EXP - $(date) ====="
    echo
    echo "--- ifconfig if_e1 ---"
    ifconfig if_e1
    echo "--- ifconfig if_e2 ---"
    ifconfig if_e2
    echo "--- route -n ---"
    route -n
    echo
    echo "--- arp -a ---"
    arp -a
    echo
} >> $FICHEIRO

echo "Saved to $FICHEIRO"
