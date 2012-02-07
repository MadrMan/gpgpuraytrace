#include "Common.h"
#include "ComputeDirect3D.h"

#include "DeviceDirect3D.h"

#include <fstream>
#include <D3Dcompiler.h>

ComputeDirect3D::ComputeDirect3D(DeviceDirect3D* device) : device(device)
{
	shader = nullptr;
}

ComputeDirect3D::~ComputeDirect3D()
{
}

bool ComputeDirect3D::create(const std::string& fileName, const std::string& main)
{
	//const std::string fileName("shader/csmain.hlsl");
	std::ifstream file(fileName);
	file.seekg(0, std::ios_base::end);
	unsigned int pos = (unsigned int)file.tellg();
	char* fileData;
	fileData = new char[pos + 1];
	file.seekg(0, std::ios_base::beg);
	file.read(fileData, pos);
	file.close();
	fileData[pos] = 0;

	UINT shaderFlags;
#if defined(_DEBUG)
	shaderFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_OPTIMIZATION_LEVEL0;
#else
	shaderFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
	
	ID3D10Blob* shaderBlob;
	ID3D10Blob* errorBlob;
	HRESULT result = D3DCompile(fileData, pos + 1, fileName.c_str(), nullptr, ((ID3DInclude*)(UINT_PTR)1), main.c_str(), "cs_5_0", 0, shaderFlags, &shaderBlob, &errorBlob);
	if(result != S_OK)
	{
		errorBlob->Release();
		return false;
	}
	
	result = device->getD3DDevice()->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &shader);
	if(result != S_OK)
	{
		return false;
	}

	return true;
}