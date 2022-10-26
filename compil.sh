#!/bin/bash

sudo gcc client.c -o client
sudo gcc server.c -o server
sudo gcc modules/tchat/client.c -o modules/tchat/client
sudo gcc modules/tchat/server.c -o modules/tchat/server
