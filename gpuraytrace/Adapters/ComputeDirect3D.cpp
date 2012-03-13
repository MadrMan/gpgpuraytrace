#include <Common.h>
#include "ComputeDirect3D.h"
#include "DeviceDirect3D.h"
#include "ShaderVariableDirect3D.h"
#include "TextureDirect3D.h"

#include "../Common/Logger.h"
#include "../Common/CRC32.h"

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
		case ERROR_FILE_NOT_FOUND:
		case ERROR_PATH_NOT_FOUND:
			Logger() << "Shader file not found: " << fullPath;
			return -1;
		default:
			LOGERROR(err, "CreateFile");
			return -1;
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
		delete *it;
	}

	//clear uavs
	for(auto it = uavBuffers.begin(); it != uavBuffers.end(); ++it)
	{
		delete *it;
	}
}

IShaderVariable* ComputeShader3D::getVariable(const std::string& name)
{
	for(auto it = buffers.begin(); it != buffers.end(); ++it)
	{
		BufferD3D* buff = *it;
		for(auto var = buff->variables.begin(); var != buff->variables.end(); ++var)
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
	
	if(reflectionBufferDesc.Type == D3D_CT_CBUFFER)
	{
		ConstantBufferD3D* newBuffer = new ConstantBufferD3D(device, reflectionBufferDesc);
		if(!newBuffer->create())
			return false;

		ID3D11ShaderReflectionVariable* shaderReflectionVar = nullptr;
		ID3D11ShaderReflectionType* shaderReflectionVarType = nullptr;
		D3D11_SHADER_VARIABLE_DESC shaderVarDesc;
		D3D11_SHADER_TYPE_DESC shaderTypeDesc;
		for(UINT i = 0; i < reflectionBufferDesc.Variables; i++)
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
				v.type = shaderTypeDesc.Name; //Can be null
				v.pointer = newBuffer->data + shaderVarDesc.StartOffset;
				v.callback = new ICallback<ComputeDirect3D, const Variable&>(this, &ComputeDirect3D::onVariableChangedCallback);
				v.tag = shaderVariable;
				VariableManager::get()->registerVariable(v);
			}
		}

		createdShader->getConstantBuffers().push_back(newBuffer);
		createdShader->getBuffers().push_back(newBuffer);
	} 
	else if(reflectionBufferDesc.Type == D3D_CT_RESOURCE_BIND_INFO) 
	{
		D3D11_SHADER_VARIABLE_DESC variableDesc;
		ID3D11ShaderReflectionVariable* variable = reflectionBuffer->GetVariableByIndex(0);
		variable->GetDesc(&variableDesc);

		UAVBufferD3D* newBuffer = new UAVBufferD3D(device, reflectionBufferDesc, variableDesc.Size);
		newBuffer->create();

		ShaderVariableDirect3D* shaderVariable = new ShaderVariableDirect3D(reflectionBufferDesc.Name, 0, reflectionBufferDesc.Size, newBuffer);
		newBuffer->variables.push_back(shaderVariable);

		createdShader->getUAVBuffers().push_back(newBuffer);
		createdShader->getBuffers().push_back(newBuffer);
	}

	return true;
}

void ComputeDirect3D::onVariableChangedCallback(const Variable& var)
{
	ShaderVariableDirect3D* shaderVariable = static_cast<ShaderVariableDirect3D*>(var.tag);
	shaderVariable->getBuffer()->change();
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

HRESULT ComputeDirect3D::getCompiledBlob(const std::string& directory, const std::string& fileName, const std::string& main, ID3DBlob** shaderBlob, ID3DBlob** errorBlob)
{
#if defined(_DEBUG)
	UINT shaderFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_OPTIMIZATION_LEVEL0;
#else
	UINT shaderFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	std::string csProfile = "cs_5_0";
	if(device->getFeatureLevel() == D3D_FEATURE_LEVEL_10_0)
		csProfile = "cs_4_0";
	if(device->getFeatureLevel() == D3D_FEATURE_LEVEL_10_1)
		csProfile = "cs_4_1";

	Logger() << "Creating CS from " << fileName << " at " << csProfile;

	//Create shader opener
	ShaderIncludeHandler handler(directory);

	//Open main shader file
	UINT fileSize = 0;
	LPCVOID fileData = 0;
	HRESULT result = handler.Open(D3D_INCLUDE_LOCAL, fileName.c_str(), nullptr, &fileData, &fileSize);

	//Compile main shader file and includes
	ID3DBlob* preprocBlob;
	result = D3DPreprocess(fileData, fileSize, fileName.c_str(), nullptr, &handler, &preprocBlob, errorBlob);
	if(FAILED(result)) return result;

	handler.Close(fileData);

	checksum_t preChecksum = CRC32::hash(preprocBlob->GetBufferPointer(), preprocBlob->GetBufferSize());

	const std::string cachedDirectory = directory + "/cache";
	const std::string cachedFile = cachedDirectory + "/" + fileName + ".bin";
	HANDLE hShaderCached = CreateFile(cachedFile.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
	if(hShaderCached == INVALID_HANDLE_VALUE) //No cached file found
	{
		Logger() << "No existing cached file found for " << fileName;
	} else {
		BY_HANDLE_FILE_INFORMATION cachedFileInfo;
		GetFileInformationByHandle(hShaderCached, &cachedFileInfo);

		checksum_t cachedChecksum;
		DWORD cacheFileBytesRead;

		ReadFile(hShaderCached, &cachedChecksum, sizeof(cachedChecksum), &cacheFileBytesRead, nullptr);

		//Check if the existing checksum equals the old checksum
		//If equal, load from cache instead of compiling ourselves
		if(preChecksum == cachedChecksum)
		{
			DWORD blobSize = cachedFileInfo.nFileSizeLow - sizeof(checksum_t);
			HRESULT res = D3DCreateBlob(blobSize, shaderBlob);
			
			ReadFile(hShaderCached, (*shaderBlob)->GetBufferPointer(), (*shaderBlob)->GetBufferSize(), &cacheFileBytesRead, nullptr);

			CloseHandle(hShaderCached);
			preprocBlob->Release();

			Logger() << "Loaded cached shader " << cachedFile;

			return S_OK;
		} else {
			CloseHandle(hShaderCached);
		}
	}

	//No usable cached version was found, so compile and save
	result = D3DCompile(preprocBlob->GetBufferPointer(), preprocBlob->GetBufferSize(), fileName.c_str(), nullptr, 0, main.c_str(), csProfile.c_str(), shaderFlags, 0, shaderBlob, errorBlob);
	if(FAILED(result)) return result;

	CreateDirectory(cachedDirectory.c_str(), nullptr);
	hShaderCached = CreateFile(cachedFile.c_str(), GENERIC_WRITE, 0, nullptr,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
	if(hShaderCached == INVALID_HANDLE_VALUE) 
	{
		Logger() << "Could not save cached file to " << cachedFile;
	} else {
		DWORD cacheFileBytesWritten;
		WriteFile(hShaderCached, &preChecksum, sizeof(preChecksum), &cacheFileBytesWritten, nullptr);
		WriteFile(hShaderCached, (*shaderBlob)->GetBufferPointer(), (*shaderBlob)->GetBufferSize(), &cacheFileBytesWritten, nullptr);
		CloseHandle(hShaderCached);
		Logger() << "Saved cached file to " << cachedFile;
	}

	return S_OK;
}

bool ComputeDirect3D::create(const std::string& directory, const std::string& fileName, const std::string& main)
{
	//clear variables in variablemanager, this also tells the client (if there is one) to clear their variables
	VariableManager::get()->clear();

	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT result = getCompiledBlob(directory, fileName, main, &shaderBlob, &errorBlob);
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

	//TODO if this is uncommented and the first shader to be loaded has an compile error, a new (errorless) shader is not displayed 
	//if(!shader)
	//{
	//	shader = createdShader;
	//} else {
		if (shader) copyShaderVarsToNewShader(createdShader);
		newShader = createdShader;
	//}

	Logger() << "Compiled new shader";

	shaderBlob->Release();
	
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
				
					shaderVarNew->write(static_cast<ConstantBufferD3D*>(shaderVarOld->getBuffer())->data);
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

	ID3D11Buffer* gpuBuffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
	ID3D11UnorderedAccessView* uavViews[4];

	size_t maxCB = shader->getConstantBuffers().size();
	size_t maxUAV = shader->getUAVBuffers().size();
	for(size_t x = 0; x < maxCB; ++x)
		gpuBuffers[x] = shader->getConstantBuffers()[x]->gpuBuffer;
	for(size_t x = 0; x < maxUAV; ++x)
		uavViews[x] = shader->getUAVBuffers()[x]->gpuView;

	device->getImmediate()->CSSetConstantBuffers(0, maxCB, gpuBuffers);
	device->getImmediate()->CSSetUnorderedAccessViews(1, maxUAV, uavViews, 0);

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