#include <Common.h>
#include "VariableManager.h"


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
	variables.clear();
}