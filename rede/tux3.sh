#!/bin/bash
# Configuracao do tuxY3
# Uso: sudo bash tux3.sh <experiencia>   (ex.: sudo bash tux3.sh 3)

EXP=$1
REDE0=172.16.10
REDE1=172.16.11

# para o NetworkManager nao apagar a nossa configuracao (quadro do prof)
nmcli device set if_e1 managed no
nmcli device set if_e2 managed no

# exp 1
ifconfig if_e1 up
ifconfig if_e1 $REDE0.1/24

# exp 3 - a outra rede passa pelo tuxY4
if [ $EXP -ge 3 ]; then
    route add -net $REDE1.0/24 gw $REDE0.254
fi

# exp 4 - rede do lab tambem pelo tuxY4
if [ $EXP -ge 4 ]; then
    route add -net 172.16.1.0/24 gw $REDE0.254
fi

ifconfig if_e1
route -n
