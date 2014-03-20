/* @brief: expire time descriptor, cert data info
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


#if !defined gehua_certmgr_expiretime_desc_h_
#define  gehua_certmgr_expiretime_desc_h_

#include <assert.h>
#include "desc-common.h"

struct ExpireTimeDescriptor : public DescBase 
{
    uint8_t time_[7];

    ExpireTimeDescriptor(uint8_t time[7])
    {
        assert(time != 0);

        tag_ = TagExpireTimeDesc;
        length_ = sizeof(time_);
        memcpy(time_, time, sizeof(time_));
    }

    ExpireTimeDescriptor() 
    {
        tag_ = TagExpireTimeDesc;
        length_ = sizeof(time_);
        memset(time_, 0, sizeof(time_));
    }

    virtual ByteStream getStream() 
    {
        ByteStream bs;
        bs.SetByteOrder(NETWORK_BYTEORDER);

        bs.PutUint8(tag_);
        bs.PutUint16(length_);

        // todo:: 
        bs.Add(time_, sizeof(time_));

        return bs;
    }
    bool operator==(ExpireTimeDescriptor const& rhs)
    {
        return !memcmp(time_, rhs.time_, sizeof(time_));
    }

    bool operator!=(ExpireTimeDescriptor const& rhs)
    {
        return !(*this == rhs);
    }
};



#endif // !gehua_certmgr_expiretime_desc_h_
