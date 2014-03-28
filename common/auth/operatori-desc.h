/* @brief: operator info descriptor, cert data element.
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


#if !defined gehua_certmgr_operatori_desc_h_
#define  gehua_certmgr_operatori_desc_h_

#include <assert.h>
#include <string>
#include "desc-common.h"

using ::std::string;

struct OperatorInfoDescriptor : public DescBase
{
    uint32_t id_;
    string name_;
    OperatorInfoDescriptor(uint32_t id, string const& name)
    {
        tag_ = TagOperatorInfoDesc;
        id_ = id;
        name_ = name;
        length_ = sizeof(id_) + name_.length() + String1ExtraLen;
    }

    OperatorInfoDescriptor() 
    {
        tag_ = TagOperatorInfoDesc;
        id_ = 0;
        name_ = "gehua";
        length_ = sizeof(id_) + name_.length() + String1ExtraLen;
    }
    
    virtual ByteStream getStream() 
    {
        ByteStream bs;
        bs.SetByteOrder(NETWORK_BYTEORDER);

        bs.PutUint8(tag_);
        bs.PutUint16(length_);

        // todo:: 
        bs.PutUint32(id_);
        bs.PutString8(name_);

        return bs;
    }

    bool operator==(OperatorInfoDescriptor const& rhs)
    {
        return id_ == rhs.id_ && name_ == rhs.name_;
    }

    bool operator!=(OperatorInfoDescriptor const& rhs)
    {
        return !(*this == rhs);
    }
  
};

#endif //gehua_certmgr_opertaori_desc_h_
