/* @brief: public key descriptor, cert data info
*         cert data include 6 parts:
*             1. cert id descriptor
*             2. cert user info descriptor
*             3. operator info descriptor
*             4. public key descriptor
*             5. expire time descriptor
*             6. signature descriptor
*
* @note: v1.0 6parts insert no used info.
*/


#if !defined gehua_certmgr_publickey_desc_h_
#define  gehua_certmgr_publickey_desc_h_

#include <assert.h>
#include "desc-common.h"

struct PublicKeyDescriptor : public DescBase 
{
    uint8_t e_[4];
    uint8_t n_[128];

    PublicKeyDescriptor(uint8_t e[4], uint8_t n[128])
    {
        assert(e != 0);
        assert(n != 0);

        tag_ = TagPublicKeyDesc;
        length_ = sizeof(e_) + sizeof(n_);
        memcpy(e_, e, sizeof(e_));
        memcpy(n_, n, sizeof(n_));
    }

    PublicKeyDescriptor() 
    {
        tag_ = TagPublicKeyDesc;
        length_ = sizeof(e_) + sizeof(n_);
        memset(e_, 0, sizeof(e_));
        memset(n_, 0, sizeof(n_));
    }

    virtual ByteStream getStream() 
    {
        ByteStream bs;
        bs.SetByteOrder(NETWORK_BYTEORDER);

        bs.PutUint8(tag_);
        bs.PutUint16(length_);

        // todo:: 
        bs.Add(e_, sizeof(e_));
        bs.Add(n_, sizeof(n_));

        return bs;
    }
    bool operator==(PublicKeyDescriptor const& rhs)
    {
        return  !memcmp(e_, rhs.e_, sizeof(e_)) && 
                !memcmp(n_, rhs.n_, sizeof(n_));
    }

    bool operator!=(PublicKeyDescriptor const& rhs)
    {
        return !(*this == rhs);
    }
};

#endif //gehua_certmgr_publickey_desc_h_
