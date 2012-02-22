#pragma once

#include "../Factories/ICompute.h"
#include "D3D11Shader.h"
#include "ShaderVariableDirect3D.h"
#include "VariableManager.h"

class DeviceDirect3D;
class ComputeDirect3D : public ICompute
{
public:
	ComputeDirect3D(DeviceDirect3D* device);
	virtual ~ComputeDirect3D();

	virtual bool create(const std::string& fileName, const std::string& main) override;

	virtual void run() override;

    virtual IShaderVariable* getVariable(const std::string& name) override;

private:
	void createBuffer(ID3D10Blob* shaderBlob);
	void reflect(ID3D10Blob* shaderBlob);
	void addBuffer(ID3D11ShaderReflection* reflection, D3D11_SHADER_DESC reflectionDesc, unsigned int index);

	void onVariableChangedCallback(const Variable& var);

	//void loopShaderBuffer(ID3D11ShaderReflectionConstantBuffer* reflectionBuffer, D3D11_SHADER_BUFFER_DESC reflectionBufferDesc, int bufferPosition); 

	DeviceDirect3D* device;
	ID3D11ComputeShader* shader;
	ID3D11ComputeShader* newShader;

	std::vector<ConstantBufferD3D*> buffers;
	ID3D11Buffer* bufferArray[D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT];
};