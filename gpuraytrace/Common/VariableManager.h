#pragma once

#include <string>
#include <vector>

struct Variable
{
	void* pointer;
	std::string name;
	std::string type;
	int sizeInBytes;
	ICallbackBase<const Variable&>* callback;
	void* tag;
};

class VariableManager
{
public:
	static VariableManager* get();
	void registerVariable(const Variable& var);
	void clear();
	void start();
	
private:
	VariableManager() : variables() { };
	static VariableManager* variableManager;
	std::vector<Variable> variables;

	static unsigned int WINAPI netLoopStatic(void* args);
	void netLoop();
	void sendVariable(Variable* var);
	void sendClearAllVariables();
	bool readBytes(char* outBuffer, int length);
	static const int bufferSize = 40192; 
	int bufferAmount;
	int bufferPosition;
	unsigned int client;
	char buffer[bufferSize];
};