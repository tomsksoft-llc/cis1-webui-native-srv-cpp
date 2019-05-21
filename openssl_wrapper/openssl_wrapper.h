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

} // namespace openssl
