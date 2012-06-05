#pragma once

//! Class which can generate noise
class Noise
{
public:
	//! Constructor
	Noise();

	//! Destructor
	virtual ~Noise();

	//! Generate the noise and fill the arrays
	void generate(bool random = true);

	static const int TEXTURE_SIZE = 128;
	unsigned char* permutations2D;
	float* permutations1D;
private:
	void generatePermutations2D();
	void generatePermuationsGradient1D();
	int getPermutation2D(int i);
	int* permutations;
};
