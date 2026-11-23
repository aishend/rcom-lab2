#!/bin/bash
# Configuracao do tuxY2 (so entra a partir da exp 2), fica tudo registado no exp<N>_<tux>.txt
# Uso (dentro da pasta rede): sudo bash tux2.sh <experiencia>

EXP=$1
REDE0=172.16.10
REDE1=172.16.11

reg() {
    bash reg.sh $EXP "$@"
}

# para o NetworkManager nao apagar a nossa configuracao (quadro do prof)
reg nmcli device set if_e1 managed no
reg nmcli device set if_e2 managed no

# exp 2
reg ifconfig if_e1 up
reg ifconfig if_e1 $REDE1.1/24

# exp 3 - a outra rede passa pelo tuxY4
if [ $EXP -ge 3 ]; then
    reg route add -net $REDE0.0/24 gw $REDE1.253
fi

# exp 4 - rede do lab pelo RC
if [ $EXP -ge 4 ]; then
    reg route add -net 172.16.1.0/24 gw $REDE1.254
fi

bash estado.sh $EXP
