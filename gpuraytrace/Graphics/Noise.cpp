#include <Common.h>
#include "Noise.h"

#include <ctime>

static float g3[][3] =  
{
    {1,1,0},
    {-1,1,0},
    {1,-1,0},
    {-1,-1,0},
    {1,0,1},
    {-1,0,1},
    {1,0,-1},
    {-1,0,-1}, 
    {0,1,1},
    {0,-1,1},
    {0,1,-1},
    {0,-1,-1},
    {1,1,0},
    {0,-1,1},
    {-1,1,0},
    {0,-1,-1}
};

Noise::Noise()
{
	permutations2D = nullptr;
	permutations1D = nullptr;
}

Noise::~Noise()
{
	delete[] permutations2D;
	delete[] permutations1D;
}

void Noise::generate()
{
	srand(time(0));
	permutations = new int[TEXTURE_SIZE];
	for(int x = 0; x < TEXTURE_SIZE; x++)
		permutations[x] = x;
	for(int x = 0; x < TEXTURE_SIZE; x++)
	{
		std::swap(permutations[x], permutations[rand() % TEXTURE_SIZE]);
	}

	generatePermutations2D();
	generatePermuationsGradient1D();
}

int Noise::getPermutation2D(int i)
{
    return permutations[i % TEXTURE_SIZE];
}

void Noise::generatePermutations2D()
{
	const int bpp = 4;
	permutations2D = new unsigned char[TEXTURE_SIZE * TEXTURE_SIZE * bpp];
    for (int x = 0; x < TEXTURE_SIZE; x++)
    {
        for (int y = 0; y < TEXTURE_SIZE; y++)
        {
			unsigned char* color = permutations2D + (x + (y * TEXTURE_SIZE)) * bpp;
            int A = getPermutation2D(x) + y;
			int B = getPermutation2D(x + 1) + y;
            color[0] = getPermutation2D(A);
            color[1] = getPermutation2D(A + 1);
            color[2] = getPermutation2D(B);
            color[3] = getPermutation2D(B + 1);
        }
    }
}

void Noise::generatePermuationsGradient1D()
{
	const int bpp = 4;
    permutations1D = new float[TEXTURE_SIZE * bpp];
    for (int x = 0; x < TEXTURE_SIZE; x++)
    {
		float* color = permutations1D + x * bpp;
		float* grad = g3[permutations[x] % 16];
        color[0] = grad[0];
		color[1] = grad[1];
		color[2] = grad[2];
		color[3] = 0.0f;
    }
}