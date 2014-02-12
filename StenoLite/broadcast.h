#include "stdafx.h"

#ifndef MY_BROADCAST_H
#define MY_BROADCAST_H

#include <string>

void CloseServer();
DWORD WINAPI RunServer(LPVOID lparam);
bool ServerRunning();
void AddNewEvent(int numdeletes, const std::string& text);

#endif