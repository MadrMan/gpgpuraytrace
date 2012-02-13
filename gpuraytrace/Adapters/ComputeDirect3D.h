#pragma once

#include "../Factories/ICompute.h"

class DeviceDirect3D;
class ComputeDirect3D : public ICompute
{
public:
	ComputeDirect3D(DeviceDirect3D* device);
	virtual ~ComputeDirect3D();

	virtual bool create(const std::string& fileName, const std::string& main) override;

	virtual void run() override;

private:
	DeviceDirect3D* device;
	ID3D11ComputeShader* shader;
	ID3D11ComputeShader* newShader;
};