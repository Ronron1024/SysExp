#ifndef STRUCTURES_H_INCLUDED
#define STRUCTURES_H_INCLUDED

#include <sys/types.h>

#include "server_config.h"

typedef enum COMMAND
{
	CONNECTION,
	MESSAGE,
	DECONNECTION,
	START
} COMMAND;

typedef struct Client
{
	char pseudo[STRING_MAX_SIZE];
	int is_spy;
	int pipe_fd;
	pid_t PID;
} Client;

typedef struct Message
{
	Client client;
	COMMAND command;
	char message[STRING_MAX_SIZE];
} Message;

#endif
