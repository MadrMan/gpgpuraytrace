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
		return E_FAIL;
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
			return E_FAIL;
		default:
			LOGERROR(err, "CreateFile");
			return E_FAIL;
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
	
	for(auto it = resources.begin(); it != resources.end(); ++it)
	{
		if(it->type == CSResourceType::Texture) continue;
		delete it->resource;
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

IResource* ComputeDirect3D::getResource(int type, const std::string& name)
{
	if(!shader) return nullptr;

	for(auto it = shader->getResources().begin(); it != shader->getResources().end(); ++it)
	{
		if(it->type == CSResourceType::Texture) continue;
		if((it->type & type) && static_cast<IShaderVariable*>(it->resource)->getName() == name)
			return it->resource;
	}

	return nullptr;
}

IShaderVariable* ComputeDirect3D::getVariable(const std::string& name)
{
	return static_cast<IShaderVariable*>(getResource(CSResourceType::Variable, name));
}

IShaderArray* ComputeDirect3D::getArray(const std::string& name)
{
	return static_cast<IShaderArray*>(getResource(CSResourceType::SBuffer | CSResourceType::UAV, name));
}

IShaderBuffer* ComputeDirect3D::getBuffer(const std::string& name)
{
	return static_cast<IShaderBuffer*>(getResource(CSResourceType::CBuffer, name));
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

			ComputeShaderResource csr;
			csr.resource = shaderVariable;
			csr.slot = i;
			csr.type = CSResourceType::Variable;
			createdShader->getResources().push_back(csr);
	
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

		ComputeShaderResource csr;
		csr.resource = newBuffer;
		csr.slot = index;
		csr.type = CSResourceType::CBuffer;
		createdShader->getResources().push_back(csr);
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

		ComputeShaderResource csr = {0};
		switch(bindDesc.Type)
		{
		case D3D_SIT_STRUCTURED:
			csr.resource = new StructuredBufferD3D(device, bindDesc.Name, bindDesc.NumSamples);
			csr.type = CSResourceType::SBuffer;
			break;
		case D3D_SIT_UAV_RWSTRUCTURED:
			csr.resource = new UAVBufferD3D(device, bindDesc.Name, bindDesc.NumSamples);
			csr.type = CSResourceType::UAV;
			break;
		case D3D_SIT_UAV_RWTYPED:
			csr.resource = new UAVBufferD3D(device, bindDesc.Name, device->getSwapUAV());
			csr.type = CSResourceType::UAV;
			break;
		}

		if(csr.resource)
		{
			csr.slot = bindDesc.BindPoint;
			createdShader->getResources().push_back(csr);
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

HRESULT ComputeDirect3D::getCompiledBlob(const std::string& directory, const std::string& fileName, const std::string& main, ID3DBlob** shaderBlob, ID3DBlob** errorBlob, const std::vector<MacroType>& addmacros)
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
	D3D_SHADER_MACRO macro;
	std::vector<D3D_SHADER_MACRO> macros;
	const std::string threadSizeXStr = convert(threadSize.x);
	const std::string threadSizeYStr = convert(threadSize.y);
	const std::string threadSizeZStr = convert(threadSize.z);

	macro.Name = "GROUP_SIZE_X";
	macro.Definition = threadSizeXStr.c_str();
	macros.push_back(macro);

	macro.Name = "GROUP_SIZE_Y";
	macro.Definition = threadSizeYStr.c_str();
	macros.push_back(macro);

	macro.Name = "GROUP_SIZE_Z";
	macro.Definition = threadSizeZStr.c_str();
	macros.push_back(macro);

	for(auto it = addmacros.begin(); it != addmacros.end(); ++it)
	{
		macro.Name = it->first.c_str();
		macro.Definition = it->second.c_str();
		macros.push_back(macro);
	}

	macro.Name = nullptr;
	macro.Definition = nullptr;
	macros.push_back(macro);

	//Compile main shader file and includes
	ID3DBlob* preprocBlob;
	result = D3DPreprocess(fileData, fileSize, fileName.c_str(), &macros.front(), &handler, &preprocBlob, errorBlob);
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

bool ComputeDirect3D::create(const std::string& directory, const std::string& fileName, const std::string& main, const ThreadSize& ts, const std::vector<MacroType>& macros)
{
	threadSize = ts;

	//clear variables in variablemanager, this also tells the client (if there is one) to clear their variables
	VariableManager::get()->clear();

	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT result = getCompiledBlob(directory, fileName, main, &shaderBlob, &errorBlob, macros);
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
	/*for(auto it1 = createdShader->getConstantBuffers().begin(); it1 != createdShader->getConstantBuffers().end(); ++it1)
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
	}*/
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
		//VariableManager::get()->sendAllVariables();
		return true;
	}

	return false;
}

void ComputeDirect3D::run(unsigned int dispatchX, unsigned int dispatchY, unsigned int dispatchZ)
{
	if(!shader) return;

	ID3D11DeviceContext* dc = device->getImmediate();

	ID3D11Buffer* gpuBuffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT] = {0};
	ID3D11UnorderedAccessView* uavViews[D3D11_PS_CS_UAV_REGISTER_COUNT] = {0};
	ID3D11ShaderResourceView* gpuTextures[16] = {0};
	int maxBuffers = 0, maxArrays = 0, maxTextures = 0;

	for(auto it = shader->getResources().begin(); it != shader->getResources().end(); ++it)
	{
		switch(it->type)
		{
		case CSResourceType::CBuffer:
			{
				ConstantBufferD3D* cb = static_cast<ConstantBufferD3D*>(it->resource);
				cb->update(dc);
				gpuBuffers[it->slot] = cb->getGpuBuffer();
				maxBuffers = std::max(maxBuffers, it->slot + 1);
			} break;
		case CSResourceType::UAV:
			{
				UAVBufferD3D* arr = static_cast<UAVBufferD3D*>(it->resource);
				uavViews[it->slot] = arr->getView();
				maxArrays = std::max(maxArrays, it->slot + 1);
			} break;
		case CSResourceType::SBuffer:
			{
				StructuredBufferD3D* sb = static_cast<StructuredBufferD3D*>(it->resource);
				gpuTextures[it->slot] = sb->getView();
				maxTextures = std::max(maxTextures, it->slot + 1);
			} break;
		case CSResourceType::Texture:
			{
				TextureDirect3D* tex = static_cast<TextureDirect3D*>(it->resource);
				gpuTextures[it->slot] = tex->getView();
				maxTextures = std::max(maxTextures, it->slot + 1);
			} break;
		default:
			break;
		}
	}

	dc->CSSetConstantBuffers(0, maxBuffers, gpuBuffers);
	dc->CSSetUnorderedAccessViews(0, maxArrays, uavViews, nullptr);
	dc->CSSetShaderResources(0, maxTextures, gpuTextures);

	dc->CSSetShader(shader->getShader(), 0, 0);
	dc->Dispatch(dispatchX, dispatchY, dispatchZ);
}

void ComputeDirect3D::setTexture(int stage, ITexture* texture)
{
	std::vector<ComputeShaderResource>& csra = shader->getResources();
	ComputeShaderResource* found = nullptr;
	for(auto it = csra.begin(); it != csra.end(); ++it)
	{
		if(it->type == CSResourceType::Texture && it->slot == stage)
		{
			if(!texture)
			{
				csra.erase(it);
				return;
			}

			found = &*it;
			break;
		}
	}

	if(!texture) return;

	if(!found)
	{
		ComputeShaderResource csr;
		csr.slot = stage;
		csr.type = CSResourceType::Texture;
		csr.resource = texture;
		csra.push_back(csr);
	} else {
		found->resource = texture;
	}
}