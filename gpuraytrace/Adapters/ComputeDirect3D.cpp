#include <Common.h>
#include "ComputeDirect3D.h"
#include "VariableManager.h"
#include "DeviceDirect3D.h"
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
	


	//check the shader file for mutable variables
	ID3D11ShaderReflection* reflection = nullptr; 
	result = D3DReflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**) &reflection); 
	if(result != S_OK)
	{
		LOGERROR(result, "D3DReflect");
		return false;
	}
	
	D3D11_SHADER_DESC reflectionDesc;
	reflection->GetDesc(&reflectionDesc);
	if(result != S_OK)
	{
		LOGERROR(result, "D3DReflectDesc");
		return false;
	}
	
	//loop needed to get index of constant buffer
	bool foundConstantBuffer = false;
	ID3D11ShaderReflectionConstantBuffer* reflectionBuffer = nullptr;
	D3D11_SHADER_BUFFER_DESC reflectionBufferDesc;
	unsigned int constantBufferPlace;
	for(constantBufferPlace = 0; constantBufferPlace < reflectionDesc.ConstantBuffers; constantBufferPlace++)
	{
		reflectionBuffer = reflection->GetConstantBufferByIndex(constantBufferPlace);
		result = reflectionBuffer->GetDesc(&reflectionBufferDesc);
		if(result != S_OK)
		{
			LOGERROR(result, "reflectionBuffer->GetDesc");
			return false;
		}
		
		const std::string targetName("ConstantBuffer");
		if(targetName == reflectionBufferDesc.Name)
		{
			foundConstantBuffer = true;
			break;
		}
	}

	if(foundConstantBuffer)
	{
		ID3D11Buffer* buff;
		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory( &bufferDesc, sizeof(bufferDesc) );
		bufferDesc.ByteWidth = reflectionBufferDesc.Size;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; 
		bufferDesc.CPUAccessFlags = 0;
	
		result = device->getD3DDevice()->CreateBuffer(&bufferDesc, 0 ,&buff);
		if(result != S_OK)
		{
			LOGERROR(result, "ID3D11Device::CreateBuffer");
			return false;
		}

		device->getImmediate()->CSSetConstantBuffers(constantBufferPlace, reflectionDesc.ConstantBuffers, &buff);

		shaderConstBufferSize = reflectionBufferDesc.Size;
		shaderConstBuffer = new char[shaderConstBufferSize];	//TODO cleanup

		ID3D11ShaderReflectionVariable* shaderReflectionVar = nullptr;
		ID3D11ShaderReflectionType* shaderReflectionVarType = nullptr;
		D3D11_SHADER_VARIABLE_DESC shaderVarDesc;
		D3D11_SHADER_TYPE_DESC shaderTypeDesc;
		VariableManager::get()->clear();			//clear buffer
	
		for(unsigned int i =0; i < reflectionBufferDesc.Variables; i++)
		{
			shaderReflectionVar = reflectionBuffer->GetVariableByIndex(i);
			result = shaderReflectionVar->GetDesc(&shaderVarDesc);		//TODO errorcheck
			if(result != S_OK)
			{
				LOGERROR(result, "ID3D11ShaderReflectionVariable::GetDesc");
				return false;
			}

			shaderReflectionVarType = shaderReflectionVar->GetType();
			shaderReflectionVarType->GetDesc(&shaderTypeDesc);
	
			Variable v;
			v.name = shaderVarDesc.Name;
			v.sizeInBytes = shaderVarDesc.Size;
			v.type = shaderTypeDesc.Name;			//can be NULL
			v.pointer = shaderConstBuffer + shaderTypeDesc.Offset;
			VariableManager::get()->registerVariable(v);
		}

		device->getImmediate()->UpdateSubresource(buff,0,0,shaderConstBuffer,shaderConstBufferSize,0);
	}
	reflection->Release();
	
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

	ID3D11DeviceContext* dc = device->getImmediate();
	dc->CSSetShader(shader, 0, 0);
	dc->Dispatch(device->getWindow()->getWindowSettings().width / 10, device->getWindow()->getWindowSettings().height / 10, 1);
	
	//dc->CSSetShader(shader, nullptr, 0);
	//dc->CSSetUnorderedAccessViews(
	//dc->
}