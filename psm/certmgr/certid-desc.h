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

struct CertIdDescriptor 
{
private:
    // portable... can't use macro to set coding easy
    // todo:: who have best idea please email: yangxingya@novel-supertv.com
#ifdef _MSC_VER
    #pragma pack(1)
#endif // _MSC_VER
    struct buffer_t {
        uint8_t tag;
        uint16_t length;
        uint64_t id;
    }
#ifdef _MSC_VER
    ;
    #pragma pack(1)
#else // _MSC_VER
# ifdef __GUNC__
    __attribute__((packed));
# endif // __GUNC__
    ;
#endif // !_MSC_VER

        buffer_t buffer;
public:
    CertIdDescriptor(uint64_t id)
    {
        buffer.tag = TagCertIdDesc;
        buffer.length = sizeof(buffer.id);
        buffer.id = id;
    }

    uint32_t length() const { return sizeof(buffer); }

    void maker(uint8_t *buff) const
    {
        assert(buff != 0);

        buffer_t buf = buffer;
        if (g_byteorder == OrderLittleEndian) {
            buf.tag = buffer.tag;
            buf.len = change_order(buffer.length);
            buf.id = change_order(buffer.id);			
        } 

        memcpy(buff, &buf, sizeof(buf));
    }
};

#endif //gehua_certmgr_certid_desc_h_
