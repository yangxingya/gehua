/*
* @brief: cert id descriptor, cert data element
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


#if !defined gehua_certmgr_certid_desc_h_
#define  gehua_certmgr_certid_desc_h_

#include <assert.h>
#include "desc-common.h"

struct CertIdDescriptor : public DescBase
{
    uint64_t id_;

    CertIdDescriptor() 
    {
        tag_ = TagCertIdDesc;
        length_ = sizeof(id_);
        id_ = 0;
    }

    CertIdDescriptor(uint64_t id)
    {
        tag_ = TagCertIdDesc;
        length_ = sizeof(id_);
        id_ = id;
    }

    virtual ByteStream getStream() 
    {
        ByteStream bs;
        bs.SetByteOrder(NETWORK_BYTEORDER);

        bs.PutUint8(tag_);
        bs.PutUint16(length_);

        // todo:: 
        bs.PutUint64(id_);

        return bs;
    }
    bool operator==(CertIdDescriptor const& rhs)
    {
        return id_ == rhs.id_;
    }

    bool operator!=(CertIdDescriptor const& rhs)
    {
        return !(*this == rhs);
    }
};



#endif //gehua_certmgr_certid_desc_h_
