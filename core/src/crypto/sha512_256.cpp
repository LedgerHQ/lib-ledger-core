/*
This code is written by kerukuro for cppcrypto library (http://cppcrypto.sourceforge.net/)
and released into public domain.
*/


#include "sha512.h"
//#include "portability.h"
#include <memory.h>
#include <functional>




namespace cppcrypto
{

	sha512::~sha512()
	{
		clear();
	}
	sha512::sha512(size_t hashsize)
		: hs(hashsize)
	{
               transfunc = [this](void* m, uint64_t num_blks) { transform(m, num_blks); };

	}

	static const uint64_t K[80] = {
		0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc,
		0x3956c25bf348b538, 0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118,
		0xd807aa98a3030242, 0x12835b0145706fbe, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,
		0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 0xc19bf174cf692694,
		0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,
		0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,
		0x983e5152ee66dfab, 0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4,
		0xc6e00bf33da88fc2, 0xd5a79147930aa725, 0x06ca6351e003826f, 0x142929670a0e6e70,
		0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,
		0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b,
		0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30,
		0xd192e819d6ef5218, 0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8,
		0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8,
		0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3,
		0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
		0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b,
		0xca273eceea26619c, 0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178,
		0x06f067aa72176fba, 0x0a637dc5a2c898a6, 0x113f9804bef90dae, 0x1b710b35131c471b,
		0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c,
		0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817
	};

	static inline uint64_t rotr(uint64_t x, int n)
	{
		return (x >> n) | (x << (64 - n));
	}

	static inline uint64_t shr(uint64_t x, int n)
	{
		return x >> n;
	}

	static inline uint64_t Ch(uint64_t x, uint64_t y, uint64_t z)
	{
		return (x & y) ^ (~x & z);
	}

	static inline uint64_t Maj(uint64_t x, uint64_t y, uint64_t z)
	{
		return (x & y) ^ (x & z) ^ (y & z);
	}

	static inline uint64_t sum0(uint64_t x)
	{
		return rotr(x, 28) ^ rotr(x, 34) ^ rotr(x, 39);
	}

	static inline uint64_t sum1(uint64_t x)
	{
		return rotr(x, 14) ^ rotr(x, 18) ^ rotr(x, 41);
	}

	static inline uint64_t sigma0(uint64_t x)
	{
		return rotr(x, 1) ^ rotr(x, 8) ^ shr(x, 7);
	}

	static inline uint64_t sigma1(uint64_t x)
	{
		return rotr(x, 19) ^ rotr(x, 61) ^ shr(x, 6);
	}

	void sha512::update(const unsigned char* data, size_t len)
	{
		if (pos && pos + len >= 128)
		{
			memcpy(&m[0] + pos, data, 128 - pos);
			transfunc(&m[0], 1);
			len -= 128 - pos;
			total += (128 - pos) * 8;
			data += 128 - pos;
			pos = 0;
		}
		if (len >= 128)
		{
			size_t blocks = len / 128;
			size_t bytes = blocks * 128;
			transfunc((void*)data, blocks);
			len -= bytes;
			total += (bytes)* 8;
			data += bytes;
		}
		memcpy(&m[0]+pos, data, len);
		pos += len;
		total += len * 8;
	}

	void sha512::init()
	{
		pos = 0;
		total = 0;
		switch(hs)
		{
			case 224:
				H[0] = 0x8C3D37C819544DA2;
				H[1] = 0x73E1996689DCD4D6;
				H[2] = 0x1DFAB7AE32FF9C82;
				H[3] = 0x679DD514582F9FCF;
				H[4] = 0x0F6D2B697BD44DA8;
				H[5] = 0x77E36F7304C48942;
				H[6] = 0x3F9D85A86A1D36C8;
				H[7] = 0x1112E6AD91D692A1;
				return;
			case 256:
				H[0] = 0x22312194FC2BF72C;
				H[1] = 0x9F555FA3C84C64C2;
				H[2] = 0x2393B86B6F53B151;
				H[3] = 0x963877195940EABD;
				H[4] = 0x96283EE2A88EFFE3;
				H[5] = 0xBE5E1E2553863992;
				H[6] = 0x2B0199FC2C85B8AA;
				H[7] = 0x0EB72DDC81C52CA2;
				return;
			case 384:
				H[0] = 0xcbbb9d5dc1059ed8;
				H[1] = 0x629a292a367cd507;
				H[2] = 0x9159015a3070dd17;
				H[3] = 0x152fecd8f70e5939;
				H[4] = 0x67332667ffc00b31;
				H[5] = 0x8eb44a8768581511;
				H[6] = 0xdb0c2e0d64f98fa7;
				H[7] = 0x47b5481dbefa4fa4;
				return;
			default:
				H[0] = 0x6a09e667f3bcc908;
				H[1] = 0xbb67ae8584caa73b;
				H[2] = 0x3c6ef372fe94f82b;
				H[3] = 0xa54ff53a5f1d36f1;
				H[4] = 0x510e527fade682d1;
				H[5] = 0x9b05688c2b3e6c1f;
				H[6] = 0x1f83d9abfb41bd6b;
				H[7] = 0x5be0cd19137e2179;
		}
		if (hs == 512)
			return;

		for (int i = 0; i < 8; i++)
			H[i] ^= 0xa5a5a5a5a5a5a5a5;
		std::string tmp = "SHA-512/" + std::to_string(hashsize());

		update((unsigned char*)&tmp[0], tmp.length());
		unsigned char buf[512 / 8];
		final(buf);
		for (int i = 0; i < 8; i++)
			H[i] = swap_uint64(H[i]);
		pos = 0;
		total = 0;
	};

	void sha512::transform(void* mp, uint64_t num_blks)
	{
		for (uint64_t blk = 0; blk < num_blks; blk++)
		{
			uint64_t M[16];
			for (uint64_t i = 0; i < 128 / 8; i++)
			{
				M[i] = swap_uint64((reinterpret_cast<const uint64_t*>(mp)[blk * 16 + i]));
			}
#ifdef	CPPCRYPTO_DEBUG
			printf("M1 - M8: %I64X %I64X %I64X %I64X %I64X %I64X %I64X %I64X %I64X %I64X %I64X %I64X %I64X %I64X %I64X %I64X\n",
				M[0], M[1], M[2], M[3], M[4], M[5], M[6], M[7], M[8], M[9], M[10], M[11], M[12], M[13], M[14], M[15]);
#endif

			uint64_t W[80];
			for (int t = 0; t <= 15; t++)
				W[t] = M[t];
			for (int t = 16; t <= 79; t++)
				W[t] = sigma1(W[t - 2]) + W[t - 7] + sigma0(W[t - 15]) + W[t - 16];

			uint64_t a = H[0];
			uint64_t b = H[1];
			uint64_t c = H[2];
			uint64_t d = H[3];
			uint64_t e = H[4];
			uint64_t f = H[5];
			uint64_t g = H[6];
			uint64_t h = H[7];

#ifdef	CPPCRYPTO_DEBUG
			printf("===============================================\n");
			printf("i = %d: %I64X %I64X %I64X %I64X %I64X %I64X %I64X %I64X\n",
				-1, a, b, c, d, e, f, g, h);
#endif

			for (int t = 0; t <= 79; t++)
			{
				uint64_t T1 = h + sum1(e) + Ch(e, f, g) + K[t] + W[t];
				uint64_t T2 = sum0(a) + Maj(a, b, c);
				h = g;
				g = f;
				f = e;
				e = d + T1;
				d = c;
				c = b;
				b = a;
				a = T1 + T2;
#ifdef	CPPCRYPTO_DEBUG
				printf("t = %d: %I64X %I64X %I64X %I64X %I64X %I64X %I64X %I64X (T1=%I64X T2=%I64X)\n",
					t, a, b, c, d, e, f, g, h, T1, T2);
#endif

			}
			H[0] += a;
			H[1] += b;
			H[2] += c;
			H[3] += d;
			H[4] += e;
			H[5] += f;
			H[6] += g;
			H[7] += h;
#ifdef	CPPCRYPTO_DEBUG
			printf("H[0] - H[7]: %I64X %I64X %I64X %I64X %I64X %I64X %I64X %I64X\n",
				H[0], H[1], H[2], H[3], H[4], H[5], H[6], H[7]);
#endif
		}
	}

	void sha512::final(unsigned char* hash)
	{
		m[pos++] = 0x80;
		if (pos > 112) {
			memset(&m[0] + pos, 0, 128 - pos);
			transfunc(&m[0], 1);
			pos = 0;
		}
		memset(&m[0] + pos, 0, 128 - pos);
		uint64_t mlen = swap_uint64(total);
		memcpy(&m[0] + (128 - 8), &mlen, 64 / 8);
		transfunc(&m[0], 1);
		for (int i = 0; i < 8; i++)
			H[i] = swap_uint64(H[i]);
		memcpy(hash, H, hashsize()/8);
	}

	void sha512::clear()
	{
		zero_memory(H.get(), H.bytes());
		zero_memory(m.data(), m.size() * sizeof(m[0]));
	}

}

