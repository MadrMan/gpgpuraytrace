#pragma once

typedef unsigned int checksum_t;
class CRC32
{
public:
	static checksum_t hash(void* data, size_t length, checksum_t hash = 0);
};