#include "SHA256.hpp"
#include "aes.hpp"

namespace ouchi{
    [[nodiscard]] inline aesKey makeKey(const char * password = nullptr, size_t sizeInByte = 0){
		SHA256 sha;
		aesKey k;
		if(sizeInByte == 0) return k;
		k.keyLength = 32;
		sha.Push((unsigned char *)password, (u_32)sizeInByte);
		sha.Final(k.data.bytes);
		return k;
    }
    [[nodiscard]] inline aesKey makeKey(const std::string & password){
        return makeKey(password.c_str(), password.size());
    }
    [[nodiscard]] inline aesKey makeKey(std::string && password){
        return makeKey(password);
    }

}
