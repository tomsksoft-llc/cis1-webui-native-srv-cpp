#include "openssl_wrapper.h"

#include <openssl/engine.h>

namespace openssl
{

void init()
{
    static bool initialized = false;
    if(!initialized)
    {
        ENGINE_load_builtin_engines();
        ENGINE_register_all_complete();
        initialized = true;
    }
}

hmac::hmac()
    : ctx_(HMAC_CTX_new())
{}

hmac::~hmac()
{
    HMAC_CTX_free(ctx_);
}

void hmac::set_secret_key(
        const unsigned char* key_data,
        size_t key_size)
{
    HMAC_Init_ex(
            ctx_,
            key_data,
            key_size,
            EVP_sha1(),
            NULL);
}

std::vector<unsigned char> hmac::calc_digest(
        const unsigned char* data,
        size_t data_size)
{
    std::vector<unsigned char> result;
    uint32_t size;
    result.resize(20);
    HMAC_Update(ctx_, data, data_size);
    HMAC_Final(ctx_, result.data(), &size);
    return result;
}

bool rand(unsigned char* data, size_t size)
{
    int rc = RAND_bytes(data, size);

    if(rc != 1)
    {
        return false;
    }

    return true;
}

} // namespace openssl
