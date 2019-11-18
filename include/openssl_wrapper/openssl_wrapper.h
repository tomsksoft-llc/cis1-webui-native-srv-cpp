/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#pragma once

#include <vector>

#include <openssl/hmac.h>

namespace openssl
{

void init();

class hmac
{
public:
    hmac();
    ~hmac();
    void set_secret_key(
            const unsigned char* key_data,
            size_t key_size);
    std::vector<unsigned char> calc_digest(
            const unsigned char* data,
            size_t data_size);
private:
    HMAC_CTX* ctx_;
};

bool rand(unsigned char* data, size_t size);

} // namespace openssl
