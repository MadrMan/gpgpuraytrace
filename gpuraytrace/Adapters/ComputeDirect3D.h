#pragma once

#include "../Factories/ICompute.h"
#include "D3D11Shader.h"
#include "ShaderVariableDirect3D.h"
#include "VariableManager.h"

class ComputeShader3D
{
public:
	ComputeShader3D(ID3D11ComputeShader* shader);
	virtual ~ComputeShader3D();

	ID3D11ComputeShader* getShader() { return shader; }
	std::vector<ConstantBufferD3D*>& getConstantBuffers() { return constantBuffers; }		
	std::vector<UAVBufferD3D*>& getArrays() { return uavBuffers; }	
	std::vector<ShaderVariableDirect3D*>& getVariables() { return variables; }	

private:	
	ID3D11ComputeShader* shader;
	std::vector<ShaderVariableDirect3D*> variables;
	std::vector<UAVBufferD3D*> uavBuffers;
	std::vector<ConstantBufferD3D*> constantBuffers;
};

class ShaderIncludeHandler : public ID3DInclude
{
public:
	ShaderIncludeHandler(const std::string& directory);

	HRESULT WINAPI Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes);
	HRESULT WINAPI Close(LPCVOID pData);

private:
	std::string directory;
};

class DeviceDirect3D;
class ComputeDirect3D : public ICompute
{
public:
	ComputeDirect3D(DeviceDirect3D* device);
	virtual ~ComputeDirect3D();

	virtual bool create(const std::string& directory, const std::string& fileName, const std::string& main) override;
	virtual void run(unsigned int dispatchX, unsigned int dispatchY, unsigned int dispatchZ) override;
	virtual IShaderVariable* getVariable(const std::string& name) override;
	virtual IShaderArray* getArray(const std::string& name) override;
	virtual IShaderBuffer* getBuffer(const std::string& name) override;
	virtual bool swap() override;
	virtual void setTexture(int stage, ITexture* texture) override;

private:
	void createBuffer(ID3D10Blob* shaderBlob);
	bool reflect(ID3D10Blob* shaderBlob, ComputeShader3D* createdShader);
	bool addBuffer(ID3D11ShaderReflection* reflection, unsigned int index, ComputeShader3D* createdShader);
	HRESULT getCompiledBlob(const std::string& directory, const std::string& fileName, const std::string& main, ID3DBlob** shaderBlob, ID3DBlob** errorBlob);
	
	//!copy the var values from old shader to the new.
	void ComputeDirect3D::copyShaderVarsToNewShader(ComputeShader3D* createdShader);
	void onVariableChangedCallback(const Variable& var);

	//void loopShaderBuffer(ID3D11ShaderReflectionConstantBuffer* reflectionBuffer, D3D11_SHADER_BUFFER_DESC reflectionBufferDesc, int bufferPosition); 

	DeviceDirect3D* device;

	ComputeShader3D* shader;
	ComputeShader3D* newShader;
};