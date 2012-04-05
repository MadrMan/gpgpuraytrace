#include <Common.h>
#include "ComputeDirect3D.h"
#include "DeviceDirect3D.h"
#include "ShaderVariableDirect3D.h"
#include "TextureDirect3D.h"

#include "../Common/Logger.h"
#include "../Common/CRC32.h"
#include "../Common/VFS.h"

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

	std::string filePath = directory + "/" + pFileName;
	std::string fullPath;
	if(!VFS::get()->openFile(filePath, &fullPath))
	{
		LOGFUNCERROR(filePath << " not found in VFS");
		return -1;
	}

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

	std::vector<ShaderVariableDirect3D*>& vars = shader->getVariables();
	for(auto it = vars.begin(); it != vars.end(); ++it)
	{
		IShaderVariable* buff = *it;
		if(buff->getName() == name)
		{
			return buff;
		}
	}

	return nullptr;
}

IShaderArray* ComputeDirect3D::getArray(const std::string& name)
{
	if(!shader) return nullptr;

	std::vector<UAVBufferD3D*>& vars = shader->getArrays();
	for(auto it = vars.begin(); it != vars.end(); ++it)
	{
		IShaderArray* buff = *it;
		if(buff->getName() == name)
		{
			return buff;
		}
	}

	return nullptr;
}

IShaderBuffer* ComputeDirect3D::getBuffer(const std::string& name)
{
	if(!shader) return nullptr;

	std::vector<ConstantBufferD3D*>& vars = shader->getConstantBuffers();
	for(auto it = vars.begin(); it != vars.end(); ++it)
	{
		IShaderBuffer* buff = *it;
		if(buff->getName() == name)
		{
			return buff;
		}
	}

	return nullptr;
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
		ConstantBufferD3D* newBuffer = new ConstantBufferD3D(device, reflectionBufferDesc.Name, reflectionBufferDesc.Size);
		if(!newBuffer->create(true))
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
			newBuffer->addVariable(shaderVariable);
			createdShader->getVariables().push_back(shaderVariable);
	
			//check if this variable needs to be send to the variablemanager
			if(newBuffer->getName()[0] == VariableManager::PREFIX)
			{
				Variable v;
				v.name = shaderVarDesc.Name;
				v.sizeInBytes = shaderVarDesc.Size;
				v.type = shaderTypeDesc.Name; //Can be null
				v.pointer = (char*)newBuffer->getData() + shaderVarDesc.StartOffset;
				v.callback = new ICallback<ComputeDirect3D, const Variable&>(this, &ComputeDirect3D::onVariableChangedCallback);
				v.tag = shaderVariable;
				VariableManager::get()->registerVariable(v);
			}
		}

		createdShader->getConstantBuffers().push_back(newBuffer);
	} 

	return true;
}

void ComputeDirect3D::onVariableChangedCallback(const Variable& var)
{
	ShaderVariableDirect3D* shaderVariable = static_cast<ShaderVariableDirect3D*>(var.tag);
	static_cast<ConstantBufferD3D*>(shaderVariable->getParent())->change();
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

	D3D11_SHADER_INPUT_BIND_DESC bindDesc;
	for(UINT x = 0; x < reflectionDesc.BoundResources; ++x)
	{
		reflection->GetResourceBindingDesc(x, &bindDesc);
		std::vector<UAVBufferD3D*>& buffers = createdShader->getArrays();

		UAVBufferD3D* newBuffer = nullptr;
		switch(bindDesc.Type)
		{
		case D3D_SIT_UAV_RWSTRUCTURED:
			newBuffer = new UAVBufferD3D(device, bindDesc.Name, bindDesc.NumSamples);
			//newBuffer->addVariable().push_back(new ShaderVariableDirect3D(newBuffer->name, 0, newBuffer->size, newBuffer));
			/*if(!newBuffer->create(true))
			{
				LOGFUNCERROR("UAVBufferD3D " << bindDesc.Name << "could not be created");
			}*/
			break;
		case D3D_SIT_UAV_RWTYPED:
			newBuffer = new UAVBufferD3D(device, bindDesc.Name, device->getSwapUAV());
			break;
		}

		if(newBuffer)
		{
			//buffers[bindDesc.BindPoint] = newBuffer;
			buffers.push_back(newBuffer);
		}
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
	LPCVOID fileData = nullptr;
	HRESULT result = handler.Open(D3D_INCLUDE_LOCAL, fileName.c_str(), nullptr, &fileData, &fileSize);

	//Add macros
	const std::string sizeXStr = convert(threadSize.x);
	const std::string sizeYStr = convert(threadSize.y);
	const std::string sizeZStr = convert(threadSize.z);
	D3D_SHADER_MACRO macros[] =
	{
		{"GROUP_SIZE_X", sizeXStr.c_str()},
		{"GROUP_SIZE_Y", sizeYStr.c_str()},
		{"GROUP_SIZE_Z", sizeZStr.c_str()},
		{nullptr, nullptr}
	};

	//Compile main shader file and includes
	ID3DBlob* preprocBlob;
	result = D3DPreprocess(fileData, fileSize, fileName.c_str(), macros, &handler, &preprocBlob, errorBlob);
	if(FAILED(result)) return result;

	handler.Close(fileData);

	//Calculate checksum for precompiled thing based on file contents, macros, and compiler flags
	checksum_t preChecksum = CRC32::hash(preprocBlob->GetBufferPointer(), preprocBlob->GetBufferSize());
	preChecksum = CRC32::hash(&shaderFlags, sizeof(shaderFlags), preChecksum);

	const std::string cachedDirectory = "Media/cache/" + directory;
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
	preprocBlob->Release();
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

bool ComputeDirect3D::create(const std::string& directory, const std::string& fileName, const std::string& main, const ThreadSize& ts)
{
	threadSize = ts;

	//clear variables in variablemanager, this also tells the client (if there is one) to clear their variables
	VariableManager::get()->clear();

	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT result = getCompiledBlob(directory, fileName, main, &shaderBlob, &errorBlob);
	if(result != S_OK)
	{
		if(result != E_FAIL)
		{
			LOGERROR(result, "getCompiledBlob");
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
		//if (shader) copyShaderVarsToNewShader(createdShader);
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
		if(newShaderBuffer->getName()[0] != VariableManager::PREFIX) continue;
		
		//for each variable in a watched buffer
		for(auto it2 = newShaderBuffer->getVariables().begin(); it2 != newShaderBuffer->getVariables().end(); ++it2)
		{
			ShaderVariableDirect3D* shaderVarNew = (ShaderVariableDirect3D*)*it2;
			
			//for each old buffer
			for(auto it3 = shader->getConstantBuffers().begin(); it3 != shader->getConstantBuffers().end(); ++it3)
			{
				ConstantBufferD3D* oldShaderBuffer = *it3;	
				if(oldShaderBuffer->getName()[0] != VariableManager::PREFIX ||
					oldShaderBuffer->getName().compare(newShaderBuffer->getName()) != 0) continue;

				//for each variable in the old buffer (that is watched)
				for(auto it4 = oldShaderBuffer->getVariables().begin(); it4 != oldShaderBuffer->getVariables().end(); ++it4)
				{
					ShaderVariableDirect3D* shaderVarOld = (ShaderVariableDirect3D*)*it4;
					if(shaderVarNew->getName().compare(shaderVarOld->getName()) != 0 ||
						shaderVarNew->getSizeInBytes() != shaderVarOld->getSizeInBytes()) continue;
				
					shaderVarNew->write(static_cast<ConstantBufferD3D*>(shaderVarOld->getParent())->getData());
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

void ComputeDirect3D::run(unsigned int dispatchX, unsigned int dispatchY, unsigned int dispatchZ)
{
	if(!shader) return;

	for(auto it = shader->getConstantBuffers().begin(); it != shader->getConstantBuffers().end(); ++it)
	{
		ConstantBufferD3D* buffer = *it;
		buffer->update(device->getImmediate());
	}

	ID3D11Buffer* gpuBuffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
	ID3D11UnorderedAccessView* uavViews[4];

	size_t maxCB = shader->getConstantBuffers().size();
	size_t maxUAV = shader->getArrays().size();
	for(size_t x = 0; x < maxCB; ++x)
		gpuBuffers[x] = shader->getConstantBuffers()[x]->getGpuBuffer();
	for(size_t x = 0; x < maxUAV; ++x)
		uavViews[x] = shader->getArrays()[x]->getView();

	device->getImmediate()->CSSetConstantBuffers(0, maxCB, gpuBuffers);
	device->getImmediate()->CSSetUnorderedAccessViews(0, maxUAV, uavViews, 0);

	ID3D11DeviceContext* dc = device->getImmediate();
	dc->CSSetShader(shader->getShader(), 0, 0);
	dc->Dispatch(dispatchX, dispatchY, dispatchZ);
}

void ComputeDirect3D::setTexture(int stage, ITexture* texture)
{
	ID3D11ShaderResourceView* view;
	if(texture)
	{
		view = static_cast<TextureDirect3D*>(texture)->getView();
		if(view) device->getImmediate()->CSSetShaderResources(stage, 1, &view);
	} else {
		view = nullptr;
		device->getImmediate()->CSSetShaderResources(stage, 1, &view);
	}
}