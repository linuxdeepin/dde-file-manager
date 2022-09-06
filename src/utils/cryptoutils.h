// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <limits>
#include <stdexcept>
#include <cstring>

#include <openssl/crypto.h>

namespace CryptoUtils {
#define UNUSED(x) (void)x;
template <typename T>
struct zallocator
{
public:
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    pointer address (reference v) const {return &v;}
    const_pointer address (const_reference v) const {return &v;}

    pointer allocate (size_type n, const void* hint = nullptr) {
        UNUSED(hint)
        if (n > std::numeric_limits<size_type>::max() / sizeof(T))
            throw std::bad_alloc();
        return static_cast<pointer> (::operator new (n * sizeof (value_type)));
    }

    void deallocate(pointer p, size_type n) {
        OPENSSL_cleanse(p, n*sizeof(T));
        ::operator delete(p);
    }

    size_type max_size() const {
        return std::numeric_limits<size_type>::max() / sizeof (T);
    }

    template<typename U>
    struct rebind
    {
        typedef zallocator<U> other;
    };

    void construct (pointer ptr, const T& val) {
        new (static_cast<T*>(ptr) ) T (val);
    }

    void destroy(pointer ptr) {
        static_cast<T*>(ptr)->~T();
    }

#if __cpluplus >= 201103L
    template<typename U, typename... Args>
    void construct (U* ptr, Args&&  ... args) {
        ::new (static_cast<void*> (ptr) ) U (std::forward<Args> (args)...);
    }

    template<typename U>
    void destroy(U* ptr) {
        ptr->~U();
    }
#endif
}; // struct zallocator

typedef unsigned char byte;
typedef std::basic_string<char, std::char_traits<char>, zallocator<char> > secure_string;

static const unsigned int AES_128_KEY_SIZE = 16;
static const unsigned int AES_128_BLOCK_SIZE = 16;

void init_aes_128_cipher();
void gen_aes_128_params(byte key[AES_128_KEY_SIZE], byte iv[AES_128_BLOCK_SIZE]);
void aes_128_encrypt(const byte key[AES_128_KEY_SIZE], const byte iv[AES_128_BLOCK_SIZE], const secure_string& ptext, secure_string& ctext);
void aes_128_decrypt(const byte key[AES_128_KEY_SIZE], const byte iv[AES_128_BLOCK_SIZE], const secure_string& ctext, secure_string& rtext);
secure_string bin_to_hex(const byte *bin, const int length);
void hex_to_bin(const secure_string &hexStr, byte * outputBuffer, const size_t length);

}; // namespace CryptoUtils
