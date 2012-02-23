#pragma once

#include "../Factories/ICompute.h"
#include "D3D11Shader.h"
#include "ShaderVariableDirect3D.h"
#include "VariableManager.h"

class ComputeShader3D
{
private:	
	ID3D11ComputeShader* shader;
	std::vector<ConstantBufferD3D*> constantBuffers;
	ID3D11Buffer* gpubuffers[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
public:
	ComputeShader3D(ID3D11ComputeShader* shader);
	~ComputeShader3D();
	IShaderVariable* getVariable(const std::string& name);
	
	ID3D11ComputeShader* getShader() { return shader; }
	ID3D11Buffer** getGpuBuffers() { return gpubuffers; }
	std::vector<ConstantBufferD3D*>& getConstantBuffers() { return constantBuffers; }		
	
};

class DeviceDirect3D;
class ComputeDirect3D : public ICompute
{
public:
	ComputeDirect3D(DeviceDirect3D* device);
	virtual ~ComputeDirect3D();

	virtual bool create(const std::string& fileName, const std::string& main) override;

	virtual void run() override;

    virtual IShaderVariable* getVariable(const std::string& name) override;

	virtual bool swap() override;

private:
	void createBuffer(ID3D10Blob* shaderBlob);
	void reflect(ID3D10Blob* shaderBlob, ComputeShader3D* createdShader);
	void addBuffer(ID3D11ShaderReflection* reflection, D3D11_SHADER_DESC reflectionDesc, unsigned int index, ComputeShader3D* createdShader);

	void onVariableChangedCallback(const Variable& var);

	//void loopShaderBuffer(ID3D11ShaderReflectionConstantBuffer* reflectionBuffer, D3D11_SHADER_BUFFER_DESC reflectionBufferDesc, int bufferPosition); 

	DeviceDirect3D* device;

	ComputeShader3D* shader;
	ComputeShader3D* newShader;
};