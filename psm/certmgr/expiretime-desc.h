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

struct ExpireTimeDescriptor 
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
        uint8_t e[4];
        uint8_t n[128];
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
    ExpireTimeDescriptor(uint8_t e[4], uint8_t n[128])
    {
        assert(e != 0);
        assert(n != 0);

        buffer.tag = TagExpireTimeDesc;
        buffer.length = sizeof(buffer.e) + sizeof(buffer.n);
        memcpy(buffer.e, e, sizeof(buffer.e));
        memcpy(buffer.n, n, sizeof(buffer.n));
    }

    uint32_t length() const { return sizeof(buffer); }

    void maker(uint8_t *buff) const
    {
        assert(buff != 0);

        buffer_t buf;
        buf.tag = buffer.tag;
        buf.length = buffer.length;
        memcpy(buf.e, buffer.e, sizeof(buffer.e));
        memcpy(buf.n, buffer.n, sizeof(buffer.n));
        if (g_byteorder == OrderLittleEndian) {
            buf.tag = buffer.tag;
            buf.length = change_order(buffer.length);
            change_order(buf.e);
            change_order(buf.n);
        } 

        memcpy(buff, &buf, sizeof(buf));
    }
};

#endif // !gehua_certmgr_expiretime_desc_h_
