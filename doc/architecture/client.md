# Process flow

	fd = readServerInfo() : exit if false
	fd2 = createClientPipe() (global variable)

	connectServer() = send pid to server
	wait for write

	while (!Ctrl+C)
		write Q
		read&print A
		
# Functions

int readServerInfo(char* info_file_path)
	return a file descriptor on the base server's named pipe or 0 if error.
	
	char* info_file_path : path to the server.info file (#define)


int createClientPipe()
	create a named pipe for communicate with the server and return its file descriptor.
	Name of the pipe is PID.


int connectServer(int base_server_pipe_fd, int timeout)
	Write PID to base_server_pipe_fd.
	
	Return 1 if server write 1 in process named pipe.
	Return 0 if no write before timeout.
