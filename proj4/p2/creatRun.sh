# 910604aA!

# TCP send
echo "./b.out -send -stat 500 -rhost 127.0.0.1 -rport 4180 -lport 4181 -proto tcp\
 -pktsize 16 -pktnum 10 -pktrate 16\
 -sbufsize 2048 -rbufsize 4096" > rcTcpSend.sh

# TCP recv
echo "./b.out -recv -stat 500 -rhost 127.0.0.1 -rport 4180 -lport 4181 -proto tcp\
 -pktsize 16 -pktnum 10 -pktrate 16\
 -sbufsize 2048 -rbufsize 4096" > rcTcpRecv.sh

# UDP send
echo "./b.out -send -stat 500 -rhost 127.0.0.1 -rport 4180 -lport 4181 -proto udp\
 -pktsize 16 -pktnum 10 -pktrate 16\
 -sbufsize 2048 -rbufsize 4096" > rcUdpSend.sh

# UDP recv
echo "./b.out -recv -stat 500 -rhost 127.0.0.1 -rport 4180 -lport 4181 -proto udp\
 -pktsize 16 -pktnum 10 -pktrate 16\
 -sbufsize 2048 -rbufsize 4096" > rcUdpRecv.sh

# TCP response
echo "./b.out -response -stat 500 -rhost 127.0.0.1 -rport 4180 -lport 4181 -proto tcp\
 -pktsize 16 -pktnum 10\
 -sbufsize 2048 -rbufsize 4096" > rcssTcp.sh

# UDP response
echo "./b.out -response -stat 500 -rhost 127.0.0.1 -rport 4180 -lport 4181 -proto udp\
 -pktsize 16 -pktnum 10\
 -sbufsize 2048 -rbufsize 4096" > rcssUdp.sh


echo "./a.out -lport 4180" > rserver.sh
chmod +x r*.sh

# lport is used for UDP only
# pktnum for response mode = max packages on traffic
