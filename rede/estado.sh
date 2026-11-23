#!/bin/bash
# Regista o estado da rede deste tux (ips, macs, rotas e arp)
# Uso: bash estado.sh <experiencia>

# corremos sempre a partir da pasta do script, para encontrar o reg.sh
cd "$(dirname "$0")"

if [ -z "$1" ]; then
    echo "Usage: $0 <experiment number>"
    exit 1
fi

EXP=$1

bash reg.sh $EXP ifconfig if_e1
bash reg.sh $EXP ifconfig if_e2
bash reg.sh $EXP route -n
bash reg.sh $EXP arp -a
