#ifndef STRUCTURES_H_INCLUDED
#define STRUCTURES_H_INCLUDED

#include <sys/types.h>

#include "server_config.h"

typedef enum COMMAND
{
	CONNECTION,
	MESSAGE,
	DECONNECTION,
	START,
	VOTE,
	ASK,
	ASK_TO
} COMMAND;

typedef enum GAME_RESULT
{
	PLAYERS,
	SPY,
	PAR
} GAME_RESULT;

typedef struct Client
{
	char pseudo[PSEUDO_MAX_SIZE];
	int is_spy;
	int pipe_fd;
	pid_t PID;
	int vote;
} Client;

typedef struct Message
{
	Client from;
	Client to;
	COMMAND command;
	char message[STRING_MAX_SIZE];
	int data;
} Message;

#endif
