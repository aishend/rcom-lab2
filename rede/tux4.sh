#!/bin/bash
# Configuracao do tuxY4
# Uso: sudo bash tux4.sh <experiencia>   (ex.: sudo bash tux4.sh 3)

EXP=$1
REDE0=172.16.10
REDE1=172.16.11

# para o NetworkManager nao apagar a nossa configuracao (quadro do prof)
nmcli device set if_e1 managed no
nmcli device set if_e2 managed no

# exp 1
ifconfig if_e1 up
ifconfig if_e1 $REDE0.254/24

# exp 3 - o tuxY4 passa a ser router entre as duas redes
if [ $EXP -ge 3 ]; then
    ifconfig if_e2 up
    ifconfig if_e2 $REDE1.253/24
    sysctl net.ipv4.ip_forward=1
    sysctl net.ipv4.icmp_echo_ignore_broadcasts=0
    iptables -P FORWARD ACCEPT
    iptables -F FORWARD
fi

# exp 4 - rede do lab pelo RC
if [ $EXP -ge 4 ]; then
    route add -net 172.16.1.0/24 gw $REDE1.254
fi

ifconfig if_e1
ifconfig if_e2
route -n
