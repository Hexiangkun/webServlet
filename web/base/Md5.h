//
// Created by 37496 on 2024/2/24.
//

#ifndef WEBSERVER_MD5_H
#define WEBSERVER_MD5_H

#include <string>
#include <iostream>
#include <openssl/evp.h>
#include <cstring>

namespace Tiny_muduo
{
    class MD5{
    public:
        static std::string md5_encryption(const std::string &src){
            std::string md5_string;

            EVP_MD_CTX* ctx = EVP_MD_CTX_new();

            const EVP_MD* md = EVP_md5();
            int ret = EVP_DigestInit(ctx, md);
            ret = EVP_DigestUpdate(ctx, src.c_str(), src.size());

            unsigned char sMD5[16] = {0};
            unsigned int nMD5Len = 0;
            ret = EVP_DigestFinal(ctx, sMD5, &nMD5Len);

            if (ret == 1)
            {
                char tmp[33] = { 0 };
                for (int i = 0; i < nMD5Len; ++i)
                {
                    ::memset(tmp, 0x00, sizeof(tmp));
                    sprintf(tmp, "%02X", sMD5[i]);
                    md5_string += tmp;
                }
            }

            EVP_MD_CTX_free(ctx);
            return md5_string;
        }
    private:
        MD5() = default;
    };
}

#endif //WEBSERVER_MD5_H
