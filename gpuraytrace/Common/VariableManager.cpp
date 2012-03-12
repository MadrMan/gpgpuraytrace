#include <Common.h>
#include "VariableManager.h"
#include "Logger.h"

#include <WS2tcpip.h>
#include <process.h>

VariableManager* VariableManager::variableManager;

VariableManager* VariableManager::get()
{
	if(variableManager) return variableManager;
	
	variableManager =  new  VariableManager();	
	return variableManager;
}

void VariableManager::registerVariable(const Variable& var)
{
	variables.push_back(var);
}

void VariableManager::clear()
{
	if(client) 
		sendClearAllVariables();

	variables.clear();
}

void VariableManager::start()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2),&wsaData);
	_beginthreadex(0, 0, netLoopStatic, this, 0, 0);
}

unsigned int VariableManager::netLoopStatic(void* args)
{
	static_cast<VariableManager*>(args)->netLoop();
	return 0;
}

bool VariableManager::readBytes(char* outBuffer, int length)
{
	while(bufferAmount < length)
	{
		memmove(buffer, buffer + bufferPosition, bufferAmount);
		bufferPosition = 0;

		int readLength = recv(client, buffer + bufferAmount, bufferSize - bufferAmount, 0);
		if(!readLength) return false;
		if(readLength == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			LOGERROR(error, "recv");
			return false;
		}

		bufferAmount += readLength;
	}

	memcpy(outBuffer, buffer + bufferPosition, length);
	bufferPosition += length;
	bufferAmount -= length;

	return true;
}



/// <summary>
/// Run the variable manager which handles the networking
/// 
/// -Packet format-
/// TO server:
/// [var:s][data:x]
///
/// FROM server:
/// [add(1):1][var:s][type:s][length:2][data:x]
/// [remove(0):1][var:s]
/// [removeAll(2):1]
/// </summary>
void VariableManager::sendVariable(Variable* var)
{
	unsigned char length;

	//identifier
	unsigned char type = 1;
	send(client, (char*)&type, 1, 0);

	//name of var
	length = (unsigned char)var->name.size();
	send(client,(char*)&length, 1, 0);		
	send(client,var->name.data(), length, 0);

	//type of var
	length = (unsigned char)var->type.size();
	send(client,(char*)&length, 1, 0);	
	send(client,var->type.data(), length, 0);

	//content of var
	unsigned short lengthShort = (unsigned short) var->sizeInBytes;
	send(client, (char*)&lengthShort, 2, 0);		
	send(client, (char*)var->pointer, lengthShort, 0);
}


void VariableManager::sendAllVariables()
{
	if(client)
	{
		for(auto it = variables.begin(); it != variables.end(); ++it)
			sendVariable(&*it);
	}
}

void VariableManager::sendClearAllVariables()
{
	unsigned char type = 2;
	send(client, (char*)&type, 1, 0);
}

void VariableManager::netLoop()
{
	int result;
	SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port = htons(10666);

	result = bind(listener, (SOCKADDR*)&service, sizeof(service));
	if(FAILED(result))
	{
		int error = WSAGetLastError();
		LOGERROR(error, "bind");
		return;
	}

	listen(listener, 5);

	for(;;)
	{
		client = accept(listener, 0, 0);
		Logger() << "Client connected";

		sendAllVariables();

		bufferAmount = 0;
		bufferPosition = 0;
		for(;;)
		{
			unsigned char stringLength;
			if(!readBytes((char*)&stringLength, 1)) break;

			std::string variableName;
			variableName.resize(stringLength);
			if(!readBytes((char*)variableName.data(), stringLength)) break;

			Variable* var = nullptr;
			for(auto it = variables.begin(); it != variables.end(); ++it)
			{
				if(it->name == variableName)
				{
					var = &*it;
					break;
				}
			}

			if(var)
			{
				if(!readBytes((char*)var->pointer, var->sizeInBytes)) break;
				var->callback->run(*var);
			}
			else
			{
				Logger() << "Var not found: " << variableName;
				closesocket(client);
				break;
			}

			Logger() << "Client send variable: " << variableName;
		}
		Logger() << "Client disconnected";
	}
}
