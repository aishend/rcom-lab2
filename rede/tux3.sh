#!/bin/bash
# Configuracao do tuxY3, fica tudo registado no exp<N>_<tux>.txt
# Uso (dentro da pasta rede): sudo bash tux3.sh <experiencia>

# corremos sempre a partir da pasta do script, para encontrar o reg.sh
cd "$(dirname "$0")"

if [ -z "$1" ]; then
    echo "Usage: $0 <experiment number>"
    exit 1
fi

EXP=$1
REDE0=172.16.10
REDE1=172.16.11

reg() {
    bash reg.sh $EXP "$@"
}

# para o NetworkManager nao apagar a nossa configuracao (quadro do prof)
reg nmcli device set if_e1 managed no
reg nmcli device set if_e2 managed no

# exp 1
reg ifconfig if_e1 up
reg ifconfig if_e1 $REDE0.1/24

# exp 3 - a outra rede passa pelo tuxY4
if [ $EXP -ge 3 ]; then
    reg route add -net $REDE1.0/24 gw $REDE0.254
fi

# exp 4 - rede do lab tambem pelo tuxY4
if [ $EXP -ge 4 ]; then
    reg route add -net 172.16.1.0/24 gw $REDE0.254
fi

bash estado.sh $EXP
