echo "./b.out -send -stat 500 -rhost 127.0.0.1 -rport 4180 -lport 4181 -proto udp \\\
 -pktsize 16 -pktnum 10 -pktrate 16 \\\
 -sbufsize 2048 -rbufsize 4096" > rclient.sh
echo "./a.out -lport 4180" > rserver.sh
chmod +x r*.sh