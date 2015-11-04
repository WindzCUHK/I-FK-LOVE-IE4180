int mySend(bool isUDP, int socket, struct sockaddr *addr, char *package, int packageSize) {
	if (isUDP) {
		return sendto(socket, package, packageSize, 0, addr, sizeof(*addr));
	} else {
		return send(socket, package, packageSize, 0);
	}
}