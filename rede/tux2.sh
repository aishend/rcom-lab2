#!/bin/bash
# Configuracao do tuxY2 (so entra a partir da exp 2)
# Uso: sudo bash tux2.sh <experiencia>   (ex.: sudo bash tux2.sh 3)

EXP=$1
REDE0=172.16.10
REDE1=172.16.11

# para o NetworkManager nao apagar a nossa configuracao (quadro do prof)
nmcli device set if_e1 managed no
nmcli device set if_e2 managed no

# exp 2
ifconfig if_e1 up
ifconfig if_e1 $REDE1.1/24

# exp 3 - a outra rede passa pelo tuxY4
if [ $EXP -ge 3 ]; then
    route add -net $REDE0.0/24 gw $REDE1.253
fi

# exp 4 - rede do lab pelo RC
if [ $EXP -ge 4 ]; then
    route add -net 172.16.1.0/24 gw $REDE1.254
fi

ifconfig if_e1
route -n
