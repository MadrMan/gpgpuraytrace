#pragma once

class Noise
{
public:
	Noise();
	virtual ~Noise();

	int setSeed(int seed);
	void generate();

	static const int TEXTURE_SIZE = 256;
	unsigned char* permutations2D;
	char* permutations1D;

private:
	void generatePermutations2D();
	void generatePermuationsGradient1D();
	int getPermutation2D(int i);
};
