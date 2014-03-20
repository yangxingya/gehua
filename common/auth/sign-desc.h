/* @brief: signature key descriptor, cert data info
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


#if !defined gehua_certmgr_sign_desc_h_
#define  gehua_certmgr_sign_desc_h_

#include <assert.h>
#include "desc-common.h"

struct SignatureDescriptor : public DescBase 
{
    uint32_t keyid_;
    uint8_t sign_[128];
public:
    SignatureDescriptor(uint32_t keyid, uint8_t sign[128])
    {
        assert(sign != 0);

        tag_ = TagSignatureDesc;
        length_ = sizeof(keyid_) + sizeof(sign_);
        keyid_ = keyid;
        memcpy(sign_, sign, sizeof(sign_));
    }

    SignatureDescriptor() 
    {
        tag_ = TagSignatureDesc;
        length_ = sizeof(keyid_) + sizeof(sign_);

        keyid_ = 0;
        memset(sign_, 0, sizeof(sign_));
    }

    virtual ByteStream getStream() 
    {
        ByteStream bs;
        bs.SetByteOrder(NETWORK_BYTEORDER);

        bs.PutUint8(tag_);
        bs.PutUint16(length_);

        // todo:: 
        bs.PutUint32(keyid_);
        bs.Add(sign_, sizeof(sign_));

        return bs;
    }
    bool operator==(SignatureDescriptor const& rhs)
    {
        return  keyid_ == rhs.keyid_ && 
                !memcmp(sign_, rhs.sign_, sizeof(sign_));
    }
    bool operator!=(SignatureDescriptor const& rhs)
    {
        return !(*this == rhs);
    }
};

#endif // !gehua_certmgr_sign_desc_h_
