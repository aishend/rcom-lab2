#!/bin/bash
# Configuracao do tuxY4, fica tudo registado no exp<N>_<tux>.txt
# Uso (dentro da pasta rede): sudo bash tux4.sh <experiencia>

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
reg ifconfig if_e1 $REDE0.254/24

# exp 3 - o tuxY4 passa a ser router entre as duas redes
if [ $EXP -ge 3 ]; then
    reg ifconfig if_e2 up
    reg ifconfig if_e2 $REDE1.253/24
    reg sysctl net.ipv4.ip_forward=1
    reg sysctl net.ipv4.icmp_echo_ignore_broadcasts=0
    reg iptables -P FORWARD ACCEPT
    reg iptables -F FORWARD
fi

# exp 4 - rede do lab pelo RC
if [ $EXP -ge 4 ]; then
    reg route add -net 172.16.1.0/24 gw $REDE1.254
fi

bash estado.sh $EXP
