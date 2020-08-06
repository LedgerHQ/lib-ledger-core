/*
This code is written by kerukuro for cppcrypto library (http://cppcrypto.sourceforge.net/)
and released into public domain.
*/

#ifndef CPPCRYPTO_SHA512_H
#define CPPCRYPTO_SHA512_H

#include "alignedarray.h"
#include <array>
#include <functional>

namespace cppcrypto
{

	class sha512
	{
	public:
		sha512(size_t hashsize = 512);
		~sha512();

		void init();
		void update(const unsigned char* data, size_t len);
		void final(unsigned char* hash);

		size_t hashsize() const { return hs; }
		size_t blocksize() const { return 1024; }
		sha512* clone() const { return new sha512; }
		void clear();

	protected:
		void transform(void* m, uint64_t num_blks);

		std::function<void(void*, uint64_t)> transfunc;
		aligned_pod_array<uint64_t, 8, 32> H;
		std::array<unsigned char, 128> m;
		size_t pos;
		uint64_t total;
		size_t hs;
	};

}

#endif
