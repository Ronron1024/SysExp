# Process flow

	fd = CreateServerPipe()
	WriteServerInfo()

	fork() (Clients connection)

	ServerMenu() -> signalHandlers

	(Clients connection)

	while (1)
		client_pid = waitForConnection()

		fork() (Manage client)

	(Manage client)
		while (client connected)
			read Q
			search A
			write A
		break
	

# Functions

createServerPipe
writeServerInfo
serverMenu
waitForConnection
searchAnswer
openClientPipe