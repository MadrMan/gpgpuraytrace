#include <Common.h>
#include "ComputeDirect3D.h"
#include "DeviceDirect3D.h"
#include "ShaderVariableDirect3D.h"
#include "TextureDirect3D.h"
#include "../Common/Logger.h"

#include "D3D11Shader.h"
#include <fstream>
#include <D3Dcompiler.h>

ShaderIncludeHandler::ShaderIncludeHandler(const std::string& directory) : directory(directory)
{

}

HRESULT WINAPI ShaderIncludeHandler::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
{
	UNREFERENCED_PARAMETER(IncludeType);
	UNREFERENCED_PARAMETER(pParentData);

	std::string fullPath = directory + "/" + pFileName;

	BY_HANDLE_FILE_INFORMATION shaderFileInfo = {0};
	DWORD shaderFileBytesRead = 0;

	HANDLE hShaderFile = nullptr;
	while((hShaderFile = CreateFile(fullPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr)) == INVALID_HANDLE_VALUE)
	{
		DWORD err = GetLastError();
		switch(err)
		{
		case ERROR_SHARING_VIOLATION:
			//Ignore and sleep
			break;
		case ERROR_PATH_NOT_FOUND:
			Logger() << "Shader file not found: " << fullPath;
			return err;
		default:
			LOGERROR(err, "CreateFile");
			return err;
		}
		Sleep(10);
	}

	GetFileInformationByHandle(hShaderFile, &shaderFileInfo);
	DWORD shaderFileBytes = shaderFileInfo.nFileSizeLow;

	if(!shaderFileBytes)
	{
		Logger() << "Error getting shader file info: " << fullPath;

		CloseHandle(hShaderFile);
		return E_ACCESSDENIED;
	}

	char* fileData = new char[shaderFileBytes + 1];
	fileData[shaderFileInfo.nFileSizeLow] = 0;

	ReadFile(hShaderFile, fileData, shaderFileInfo.nFileSizeLow, &shaderFileBytesRead, nullptr);
	CloseHandle(hShaderFile);

	*ppData = fileData;
	*pBytes = shaderFileBytesRead;

	return S_OK;
}

HRESULT WINAPI ShaderIncludeHandler::Close(LPCVOID pData)
{
	delete[] pData;

	return S_OK;
}

ComputeShader3D::ComputeShader3D(ID3D11ComputeShader* shader) : shader(shader) 
{
	
}

ComputeShader3D::~ComputeShader3D()
{	
	if(shader) shader->Release();
	
	//clear buffers
	for(auto it = constantBuffers.begin(); it != constantBuffers.end(); ++it)
	{
		ConstantBufferD3D* buffer = *it;
		delete buffer;
	}

	//clear buffers
	for(size_t i = 0; i < constantBuffers.size(); i++)
	{
		gpubuffers[i]->Release();
	}
}

IShaderVariable* ComputeShader3D::getVariable(const std::string& name)
{
	for(auto it = constantBuffers.begin(); it != constantBuffers.end(); ++it)
	{
		ConstantBufferD3D* constBuff = *it;
		for(auto var = constBuff->variables.begin(); var != constBuff->variables.end(); ++var)
		{
			if(name == (*var)->getName())
			{
				return *var;
			}
		}
	}
	return nullptr;
}	
		
ComputeDirect3D::ComputeDirect3D(DeviceDirect3D* device) : device(device)
{
	shader = nullptr;
	newShader = nullptr;
}

ComputeDirect3D::~ComputeDirect3D()
{
	delete shader;
	delete newShader;
}

IShaderVariable* ComputeDirect3D::getVariable(const std::string& name)
{
	if(!shader) return nullptr;
	return shader->getVariable(name);
}

bool ComputeDirect3D::addBuffer(ID3D11ShaderReflection* reflection, unsigned int index, ComputeShader3D* createdShader)
{
	HRESULT result;
	ID3D11ShaderReflectionConstantBuffer* reflectionBuffer = nullptr;
	D3D11_SHADER_BUFFER_DESC reflectionBufferDesc;
	reflectionBuffer = reflection->GetConstantBufferByIndex(index);
	result = reflectionBuffer->GetDesc(&reflectionBufferDesc);
	if(result != S_OK)
	{
		LOGERROR(result, "reflectionBuffer->GetDesc");
		return false;
	}
	
	ConstantBufferD3D* newBuffer = new ConstantBufferD3D(createdShader->getGpuBuffers(),index);
		
	ID3D11ShaderReflectionVariable* shaderReflectionVar = nullptr;
	ID3D11ShaderReflectionType* shaderReflectionVarType = nullptr;
	D3D11_SHADER_VARIABLE_DESC shaderVarDesc;
	D3D11_SHADER_TYPE_DESC shaderTypeDesc;
	
	newBuffer->index = index;
	newBuffer->size = reflectionBufferDesc.Size;
	newBuffer->data = new char[newBuffer->size];
	newBuffer->name = reflectionBufferDesc.Name;
	newBuffer->dirty = true;
	newBuffer->create(device);
	
	ZeroMemory(newBuffer->data, newBuffer->size);
	
	for(unsigned int i =0; i < reflectionBufferDesc.Variables; i++)
	{
		shaderReflectionVar = reflectionBuffer->GetVariableByIndex(i);
		result = shaderReflectionVar->GetDesc(&shaderVarDesc);
		if(result != S_OK)
		{
			LOGERROR(result, "ID3D11ShaderReflectionVariable::GetDesc");
			return false;
		}

		shaderReflectionVarType = shaderReflectionVar->GetType();
		shaderReflectionVarType->GetDesc(&shaderTypeDesc);
		ShaderVariableDirect3D* shaderVariable = new ShaderVariableDirect3D(shaderVarDesc.Name, shaderVarDesc.StartOffset,  shaderVarDesc.Size, newBuffer);
		newBuffer->variables.push_back(shaderVariable);
	
		//check if this variable needs to be send to the variablemanager
		if(newBuffer->name[0] == VariableManager::PREFIX)
		{
			Variable v;
			v.name = shaderVarDesc.Name;
			v.sizeInBytes = shaderVarDesc.Size;
			v.type = shaderTypeDesc.Name;			//can be NULL
			v.pointer = newBuffer->data + shaderVarDesc.StartOffset;
			v.callback = new ICallback<ComputeDirect3D, const Variable&>(this, &ComputeDirect3D::onVariableChangedCallback);
			v.tag = shaderVariable;
			VariableManager::get()->registerVariable(v);
		}
	}
	createdShader->getConstantBuffers().push_back(newBuffer);

	return true;
}

void ComputeDirect3D::onVariableChangedCallback(const Variable& var)
{
	ShaderVariableDirect3D* shaderVariable = static_cast<ShaderVariableDirect3D*>(var.tag);
	shaderVariable->getBuffer()->dirty = true;
}

bool ComputeDirect3D::reflect(ID3D10Blob* shaderBlob, ComputeShader3D* createdShader)
{
	ID3D11ShaderReflection* reflection = nullptr; 

	HRESULT result = D3DReflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**) &reflection); 
	if(FAILED(result))
	{
		LOGERROR(result, "D3DReflect could not be created");
		return false;
	}

	D3D11_SHADER_DESC reflectionDesc;
	reflection->GetDesc(&reflectionDesc);
	if(FAILED(result))
	{
		LOGERROR(result, "D3DReflectDesc could not be created");
		return false;
	}

	//loop to find all constant shader buffers
	unsigned int index;
	for(index = 0; index < reflectionDesc.ConstantBuffers; index++)
	{
		if(!addBuffer(reflection, index, createdShader))
		{
			return false;
		}
	}

	reflection->Release();

	return true;
}

bool ComputeDirect3D::create(const std::string& directory, const std::string& fileName, const std::string& main)
{
	//clear variables in variablemanager, this also tells the client (if there is one) to clear their variables
	VariableManager::get()->clear();

	//Create shader opener
	ShaderIncludeHandler handler(directory);

	//Open main shader file
	UINT fileSize = 0;
	LPCVOID fileData = 0;
	HRESULT result = handler.Open(D3D_INCLUDE_LOCAL, fileName.c_str(), nullptr, &fileData, &fileSize);
	
	UINT shaderFlags;
#if defined(_DEBUG)
	shaderFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_OPTIMIZATION_LEVEL0;
#else
	shaderFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	std::string csProfile = "cs_5_0";
	if(device->getFeatureLevel() == D3D_FEATURE_LEVEL_10_0)
		csProfile = "cs_4_0";
	if(device->getFeatureLevel() == D3D_FEATURE_LEVEL_10_1)
		csProfile = "cs_4_1";

	Logger() << "Creating CS from " << fileName << " at " << csProfile;

	//Compile main shader file and includes
	ID3D10Blob* shaderBlob;
	ID3D10Blob* errorBlob;
	result = D3DCompile(fileData, fileSize, fileName.c_str(), nullptr, &handler, main.c_str(), csProfile.c_str(), shaderFlags, 0, &shaderBlob, &errorBlob);
	handler.Close(fileData);

	if(result != S_OK)
	{
		if(result != E_FAIL)
		{
			LOGERROR(result, "D3DCompile");
		}

		if(errorBlob)
		{
			Logger() << "The following errors occured while trying to compile:\n" << (const char*)errorBlob->GetBufferPointer();
			errorBlob->Release();
		} else {
			Logger() << "No error message";
		}

		return false;
	}

	if(errorBlob)
	{
		Logger() << "The following warning occured while compiling:\n" << (const char*)errorBlob->GetBufferPointer();
		errorBlob->Release();
	}

	ID3D11ComputeShader* shaderCS;
	result = device->getD3DDevice()->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &shaderCS);
	if(result != S_OK)
	{
		LOGERROR(result, "CreateComputeShader");
		return false;
	}

	ComputeShader3D* createdShader = new ComputeShader3D(shaderCS);
	if(!reflect(shaderBlob, createdShader))
	{
		return false;
	}

	//if(!shader)
	//{
	//	shader = createdShader;
	//} else {
		copyShaderVarsToNewShader(createdShader);
		newShader = createdShader;
	//}

	Logger() << "Compiled new shader";
	
	return true;
}

void ComputeDirect3D::copyShaderVarsToNewShader(ComputeShader3D* createdShader)
{
	if(!shader || !createdShader)
	{
		LOGFUNCERROR("Invalid parameter");
		return;
	}

	//for each new buffer
	for(auto it1 = createdShader->getConstantBuffers().begin(); it1 != createdShader->getConstantBuffers().end(); ++it1)
	{
		ConstantBufferD3D* newShaderBuffer = *it1;	
		if(newShaderBuffer->name[0] != VariableManager::PREFIX) continue;
		
		//for each variable in a watched buffer
		for(auto it2 = newShaderBuffer->variables.begin(); it2 != newShaderBuffer->variables.end(); ++it2)
		{
			ShaderVariableDirect3D* shaderVarNew = (ShaderVariableDirect3D*)*it2;
			
			//for each old buffer
			for(auto it3 = shader->getConstantBuffers().begin(); it3 != shader->getConstantBuffers().end(); ++it3)
			{
				ConstantBufferD3D* oldShaderBuffer = *it3;	
				if(oldShaderBuffer->name[0] != VariableManager::PREFIX ||
					oldShaderBuffer->name.compare(newShaderBuffer->name) != 0) continue;

				//for each variable in the old buffer (that is watched)
				for(auto it4 = oldShaderBuffer->variables.begin(); it4 != oldShaderBuffer->variables.end(); ++it4)
				{
					ShaderVariableDirect3D* shaderVarOld = (ShaderVariableDirect3D*)*it4;
					if(shaderVarNew->getName().compare(shaderVarOld->getName()) != 0 ||
						shaderVarNew->getSizeInBytes() != shaderVarOld->getSizeInBytes()) continue;
				
					shaderVarNew->write(shaderVarOld->getBuffer()->data);
				}
			}
		}
	}
}


bool ComputeDirect3D::swap()
{
	if(newShader)
	{
		Logger() << "Replacing ComputeShader with updated file";

		if(shader) 
		{
			delete shader;
		}

		shader = newShader;
		newShader = nullptr;


		//tell the variable manager to resend the variables
		VariableManager::get()->sendAllVariables();
		return true;
	}

	return false;
}

void ComputeDirect3D::run()
{
	if(!shader) return;

	for(auto it = shader->getConstantBuffers().begin(); it != shader->getConstantBuffers().end(); ++it)
	{
		ConstantBufferD3D* buffer = *it;
		if(buffer->dirty)
		{
			buffer->dirty = false;
			device->getImmediate()->UpdateSubresource(buffer->gpuBuffer, 0, 0, buffer->data, buffer->size, 0);
		}
	}

	device->getImmediate()->CSSetConstantBuffers(0, shader->getConstantBuffers().size(), shader->getGpuBuffers());

	ID3D11DeviceContext* dc = device->getImmediate();
	dc->CSSetShader(shader->getShader(), 0, 0);
	dc->Dispatch(device->getWindow()->getWindowSettings().width / 16, device->getWindow()->getWindowSettings().height / 16, 1);
	
	//dc->CSSetShader(shader, nullptr, 0);
	//dc->CSSetUnorderedAccessViews(
	//dc->
}

void ComputeDirect3D::setTexture(int stage, ITexture* texture)
{
	if(texture)
	{
		ID3D11ShaderResourceView* view = static_cast<TextureDirect3D*>(texture)->getView();
		device->getImmediate()->CSSetShaderResources(stage, 1, &view);
	} else {
		device->getImmediate()->CSSetShaderResources(stage, 1, nullptr);
	}
}