#pragma once
#ifndef SOCKET_H
#define SOCKET_H

int create_socket();
void newConnection_handler(int fd);
void close_socket();
int process_cmd(const char* cmd, char* answer);

#endif