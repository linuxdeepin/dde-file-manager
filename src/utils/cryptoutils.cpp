/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "cryptoutils.h"

#include <openssl/evp.h>
#include <openssl/rand.h>

namespace CryptoUtils {

using EVP_CIPHER_CTX_free_ptr = std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)>;

void gen_aes_128_params(byte key[], byte iv[])
{
    int rc = RAND_bytes(key, AES_128_KEY_SIZE);
    if (rc != 1) {
        throw std::runtime_error("RAND_bytes key failed");
    }

    rc = RAND_bytes(iv, AES_128_BLOCK_SIZE);
    if (rc != 1) {
        throw std::runtime_error("RAND_bytes for iv failed");
    }
}

void aes_128_encrypt(const byte key[], const byte iv[], const secure_string &ptext, secure_string &ctext)
{
    EVP_CIPHER_CTX_free_ptr ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);
    int rc = EVP_EncryptInit_ex(ctx.get(), EVP_aes_128_cfb(), NULL, key, iv);
    if (rc != 1) {
        throw std::runtime_error("EVP_EncryptInit_ex failed");
    }

    // Recovered text expands upto BLOCK_SIZE
    ctext.resize(ptext.size() + AES_128_BLOCK_SIZE);
    int out_len1 = (int)ctext.size();

    rc = EVP_EncryptUpdate(ctx.get(), (byte *)&ctext[0], &out_len1, (const byte *)&ptext[0], (int)ptext.size());
    if (rc != 1)
        throw std::runtime_error("EVP_EncryptUpdate failed");

    int out_len2 = (int)ctext.size() - out_len1;
    rc = EVP_EncryptFinal_ex(ctx.get(), (byte *)&ctext[0] + out_len1, &out_len2);
    if (rc != 1)
        throw std::runtime_error("EVP_EncryptFinal_ex failed");

    // Set cipher text size now that we know it
    ctext.resize(out_len1 + out_len2);
}

void aes_128_decrypt(const byte key[], const byte iv[], const secure_string &ctext, secure_string &rtext)
{
    EVP_CIPHER_CTX_free_ptr ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);
    int rc = EVP_DecryptInit_ex(ctx.get(), EVP_aes_128_cfb(), NULL, key, iv);
    if (rc != 1) {
        throw std::runtime_error("EVP_DecryptInit_ex failed");
    }

    // Recovered text contracts upto BLOCK_SIZE
    rtext.resize(ctext.size());
    int out_len1 = (int)rtext.size();

    rc = EVP_DecryptUpdate(ctx.get(), (byte *)&rtext[0], &out_len1, (const byte *)&ctext[0], (int)ctext.size());
    if (rc != 1) {
        throw std::runtime_error("EVP_DecryptUpdate failed");
    }

    int out_len2 = (int)rtext.size() - out_len1;
    rc = EVP_DecryptFinal_ex(ctx.get(), (byte *)&rtext[0] + out_len1, &out_len2);
    if (rc != 1) {
        throw std::runtime_error("EVP_DecryptFinal_ex failed");
    }

    // Set recovered text size now that we know it
    rtext.resize(out_len1 + out_len2);
}

secure_string bin_to_hex(const CryptoUtils::byte *bin, const int length)
{
    secure_string str;
    char twoChar[3];
    for (int i = 0; i < length; i++) {
        snprintf(twoChar, 3, "%02X", bin[i]);
        str += twoChar;
    }
    return str;
}

void hex_to_bin(const secure_string &hexStr, byte *outputBuffer, const size_t length)
{
    const char *hex = hexStr.c_str();
    memset(outputBuffer, 0, length);
    for (unsigned int i = 0; i < length; i++) {
        unsigned int d;
        sscanf(hex + i * 2, "%02X", &d);
        outputBuffer[i] = (byte)d;
    }
}

void init_aes_128_cipher()
{
    EVP_add_cipher(EVP_aes_128_cfb());
}

} // namespace CryptoUtils

