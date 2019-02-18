#ifndef CUCKOO_HASHING_H
#define CUCKOO_HASHING_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <stdexcept>
#include "BOBHash32.h"

namespace cuckoo {
template<uint32_t keylen, int d = 4>
class CuckooHashing
{
    struct KVPair
    {
        char key[keylen];
        uint32_t val;

        KVPair()
        {
            memset(key, 0, keylen);
            val = 0;
        }

        KVPair(const KVPair & b)
        {
            memcpy(key, b.key, keylen);
            val = b.val;
        }

        bool is_empty()
        {
            for (int i = 0; i < keylen; ++i) {
                if (key[i] != 0)
                    return false;
            }
            return true;
        }

        bool is_key_match(const char * n_key)
        {
            return memcmp(key, n_key, keylen) == 0;
        }

        void set(const char * i_key, uint32_t i_val)
        {
            memcpy(key, i_key, keylen);
            val = i_val;
        }
    };

    uint32_t w;
    KVPair** buckets;
    BOBHash32 * hashs[2];

public:
    CuckooHashing()
    {
        random_device rd;
        int seed_a, seed_b;
        seed_a = rd() % MAX_PRIME32;
        do {
            seed_b = rd() % MAX_PRIME32;
        } while (seed_a == seed_b);
        hashs[0] = new BOBHash32(uint32_t(seed_a));
        hashs[1] = new BOBHash32(uint32_t(seed_b));
    }
	void init(int capacity)
	{
		w = capacity / d;
		buckets = new KVPair*[w];
		for (int  i = 0; i < w; i++)
		{
			buckets[i] = new KVPair[d];
			memset(buckets[i], 0, sizeof(KVPair)*d);
		}
	}
    bool insert(const char * key, uint32_t val, int from_k = -1, int remained = 5)
    {
        if (remained == 0)
            return false;
        uint32_t hv[2];
        for (int k = 0; k < 2; ++k) {
            hv[k] = hashs[k]->run(key, keylen) % w;
            if (k == from_k)
                continue;
            for (int i = 0; i < d; ++i) {
                if (buckets[hv[k]][i].is_empty()) {
                    buckets[hv[k]][i].set(key, val);
                    return true;
                }
            }

        }

        // find a path
        for (int k = 0; k < 2; ++k) {
            uint32_t nhv = hv[k];
            if (k == from_k)
                continue;
            for (int i = 0; i < d; ++i) {
                if (insert(buckets[nhv][i].key, buckets[nhv][i].val, k, remained - 1)) {
                    buckets[nhv][i].set(key, val);
                    return true;
                }
            }
        }

        return false;
    }

    uint32_t & operator[](const char * key)
    {
        uint32_t hv[2];
        for (int k = 0; k < 2; ++k) {
            hv[k] = hashs[k]->run(key, keylen) % w;
            for (int i = 0; i < d; ++i) {
                if (buckets[hv[k]][i].is_key_match(key)) {
                    return buckets[hv[k]][i].val;
                }
            }
        }

        bool suc = insert(key, 0);
        if (!suc) {
            throw std::logic_error("Hash table is full.");
        }

        return (*this)[key];
    }

    bool query(const char * key, uint32_t & val)
    {
        uint32_t hv[2];
        for (int k = 0; k < 2; ++k) {
            hv[k] = hashs[k]->run(key, keylen) % w;
            for (int i = 0; i < d; ++i) {
                if (buckets[hv[k]][i].is_key_match(key)) {
                    val = buckets[hv[k]][i].val;
                    return true;
                }
            }
        }

        return false;
    }

    bool find(const char * key)
    {
        uint32_t val;
        return query(key, val);
    }

    bool erase(const char * key)
    {
        uint32_t hv[2];
        for (int k = 0; k < 2; ++k) {
            hv[k] = hashs[k]->run(key, keylen) % w;
            for (int i = 0; i < d; ++i) {
                if (buckets[hv[k]][i].is_key_match(key)) {
                    memset(buckets[hv[k]][i].key, 0, keylen);
                    return true;
                }
            }
        }

        return false;
    }

    ~CuckooHashing()
    {
        for (int i = 0; i < 2; ++i) {
            delete hashs[i];
        }
		for (int i = 0; i < w; ++i) {
			delete buckets[i];
		}
		delete [] buckets;
    }
};
}

#endif //CUCKOO_HASHING_H
