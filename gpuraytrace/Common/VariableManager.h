#pragma once

#include <string>
#include <vector>

struct Variable
{
	void* pointer;
	std::string name;
	std::string type;
	int sizeInBytes;
};

class VariableManager
{
public:
	static VariableManager* get();
	void registerVariable(const Variable& var);
	void clear();

private:
	VariableManager() : variables() { };
	static VariableManager* variableManager;
	std::vector<Variable> variables;
};