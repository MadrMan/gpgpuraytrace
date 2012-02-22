#include <Common.h>
#include "ComputeDirect3D.h"
#include "DeviceDirect3D.h"
#include "ShaderVariableDirect3D.h"
#include "../Common/Logger.h"

#include "D3D11Shader.h"
#include <fstream>
#include <D3Dcompiler.h>

ComputeDirect3D::ComputeDirect3D(DeviceDirect3D* device) : device(device)
{
	shader = nullptr;
	newShader = nullptr;
}

ComputeDirect3D::~ComputeDirect3D()
{
	if(shader) shader->Release();
	if(newShader) newShader->Release();
}

IShaderVariable* ComputeDirect3D::getVariable(const std::string& name)
{
	for(auto it = buffers.begin(); it != buffers.end(); ++it)
	{
		ConstantBufferD3D* buffer = *it;
		for(auto var = buffer->variables.begin(); var != buffer->variables.end(); ++var)
		{
			if(name == (*var)->getName())
			{
				return *var;
			}
		}
	}
	return nullptr;
}

void ComputeDirect3D::addBuffer(ID3D11ShaderReflection* reflection, D3D11_SHADER_DESC reflectionDesc, unsigned int index)
{
	HRESULT result;
	ID3D11ShaderReflectionConstantBuffer* reflectionBuffer = nullptr;
	D3D11_SHADER_BUFFER_DESC reflectionBufferDesc;
	reflectionBuffer = reflection->GetConstantBufferByIndex(index);
	result = reflectionBuffer->GetDesc(&reflectionBufferDesc);
	if(result != S_OK)
	{
		LOGERROR(result, "reflectionBuffer->GetDesc");
		return ;
	}
	
	ConstantBufferD3D* newBuffer = new ConstantBufferD3D();
	
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory( &bufferDesc, sizeof(bufferDesc) );
	bufferDesc.ByteWidth = reflectionBufferDesc.Size;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; 
	bufferDesc.CPUAccessFlags = 0;

	result = device->getD3DDevice()->CreateBuffer(&bufferDesc, 0, &newBuffer->gpuBuffer);
	if(result != S_OK) 
	{
		LOGERROR(result, "ID3D11Device::CreateBuffer");
		return ;
	}
	bufferArray[index] = newBuffer->gpuBuffer;
		
	ID3D11ShaderReflectionVariable* shaderReflectionVar = nullptr;
	ID3D11ShaderReflectionType* shaderReflectionVarType = nullptr;
	D3D11_SHADER_VARIABLE_DESC shaderVarDesc;
	D3D11_SHADER_TYPE_DESC shaderTypeDesc;
	
	
	newBuffer->index = index;
	newBuffer->size = reflectionBufferDesc.Size;
	newBuffer->data = new char[newBuffer->size];
	newBuffer->name = reflectionBufferDesc.Name;
	newBuffer->dirty = true;

	ZeroMemory(newBuffer->data, newBuffer->size);
	
	for(unsigned int i =0; i < reflectionBufferDesc.Variables; i++)
	{
		shaderReflectionVar = reflectionBuffer->GetVariableByIndex(i);
		result = shaderReflectionVar->GetDesc(&shaderVarDesc);
		if(result != S_OK)
		{
			LOGERROR(result, "ID3D11ShaderReflectionVariable::GetDesc");
			return ;
		}

		shaderReflectionVarType = shaderReflectionVar->GetType();
		shaderReflectionVarType->GetDesc(&shaderTypeDesc);
		ShaderVariableDirect3D* shaderVariable = new ShaderVariableDirect3D(shaderVarDesc.Name, shaderVarDesc.StartOffset,  shaderVarDesc.Size, newBuffer);
		newBuffer->variables.push_back(shaderVariable);
		
		//doNT FORget to clear variablemanager
		//to register var	
		Variable v;
		v.name = shaderVarDesc.Name;
		v.sizeInBytes = shaderVarDesc.Size;
		v.type = shaderTypeDesc.Name;			//can be NULL
		v.pointer = newBuffer->data + shaderVarDesc.StartOffset;
		v.callback = new ICallback<ComputeDirect3D, const Variable&>(this, &ComputeDirect3D::onVariableChangedCallback);
		v.tag = shaderVariable;
		VariableManager::get()->registerVariable(v);
	}
	buffers.push_back(newBuffer);
}

void ComputeDirect3D::onVariableChangedCallback(const Variable& var)
{
	ShaderVariableDirect3D* shaderVariable = static_cast<ShaderVariableDirect3D*>(var.tag);
	shaderVariable->getBuffer()->dirty = true;
}

void ComputeDirect3D::reflect(ID3D10Blob* shaderBlob)
{
	ID3D11ShaderReflection* reflection = nullptr; 

	HRESULT result = D3DReflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**) &reflection); 
	if(FAILED(result))
	{
		LOGERROR(result, "D3DReflect could not be created");
		return ;
	}

	D3D11_SHADER_DESC reflectionDesc;
	reflection->GetDesc(&reflectionDesc);
	if(FAILED(result))
	{
		LOGERROR(result, "D3DReflectDesc could not be created");
		return ;
	}

	//loop needed to get index of constant buffer

	unsigned int index;
	for(index = 0; index < reflectionDesc.ConstantBuffers; index++)
	{
		addBuffer(reflection, reflectionDesc, index);
	}

	reflection->Release();
}

bool ComputeDirect3D::create(const std::string& fileName, const std::string& main)
{
	BY_HANDLE_FILE_INFORMATION shaderFileInfo = {0};
	DWORD shaderFileBytesRead = 0;

	HANDLE hShaderFile = nullptr;
	while((hShaderFile = CreateFile(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr)) == INVALID_HANDLE_VALUE)
	{
		DWORD err = GetLastError();
		if(err != ERROR_SHARING_VIOLATION)
		{
			LOGERROR(err, "CreateFile");
			return false;
		}
		Sleep(10);
	}

	GetFileInformationByHandle(hShaderFile, &shaderFileInfo);
	DWORD shaderFileBytes = shaderFileInfo.nFileSizeLow;

	if(!shaderFileBytes)
	{
		Logger() << "Error getting shader file info: " << fileName;

		CloseHandle(hShaderFile);
		return false;
	}

	char* fileData = new char[shaderFileBytes + 1];
	fileData[shaderFileInfo.nFileSizeLow] = 0;

	ReadFile(hShaderFile, fileData, shaderFileInfo.nFileSizeLow, &shaderFileBytesRead, nullptr);
	CloseHandle(hShaderFile);

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

	ID3D10Blob* shaderBlob;
	ID3D10Blob* errorBlob;
	HRESULT result = D3DCompile(fileData, shaderFileInfo.nFileSizeLow + 1, fileName.c_str(), nullptr, ((ID3DInclude*)(UINT_PTR)1), main.c_str(), csProfile.c_str(), 0, shaderFlags, &shaderBlob, &errorBlob);
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

	reflect(shaderBlob);

	ID3D11ComputeShader* createdShader = nullptr;
	result = device->getD3DDevice()->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &createdShader);
	if(result != S_OK)
	{
		LOGERROR(result, "CreateComputeShader");
		return false;
	}

	if(!shader)
	{
		shader = createdShader;
	} else {
		newShader = createdShader;
	}

	return true;
}

void ComputeDirect3D::run()
{
	if(newShader)
	{
		Logger() << "Replacing ComputeShader with updated file";

		if(shader) 
		{
			shader->Release();
			shader = nullptr;
		}
		shader = newShader;
		newShader = nullptr;
	}

	for(auto it = buffers.begin(); it != buffers.end(); ++it)
	{
		ConstantBufferD3D* buffer = *it;
		if(buffer->dirty)
		{
			buffer->dirty = true;
			device->getImmediate()->UpdateSubresource(buffer->gpuBuffer, 0, 0, buffer->data, buffer->size, 0);
		}
	}


	device->getImmediate()->CSSetConstantBuffers(0, buffers.size(), bufferArray);


	ID3D11DeviceContext* dc = device->getImmediate();
	dc->CSSetShader(shader, 0, 0);
	dc->Dispatch(device->getWindow()->getWindowSettings().width, device->getWindow()->getWindowSettings().height, 1);
	
	//dc->CSSetShader(shader, nullptr, 0);
	//dc->CSSetUnorderedAccessViews(
	//dc->
}