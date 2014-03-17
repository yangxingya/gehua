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

struct PublicKeyDescriptor 
{
private:
    #pragma pack(1)
    struct buffer_t {
        uint8_t tag;
        uint16_t length;
        uint8_t e[4];
        uint8_t n[128];
        buffer_t(buffer_t const& rhs)
        {
            *this = rhs;
        }
        buffer_t& operator=(buffer_t const& rhs)
        {
            if (this == &rhs)
                return *this;

            tag = rhs.tag;
            length = rhs.length;
            memcpy(e, rhs.e, sizeof(e));
            memcpy(n, rhs.n, sizeof(n));

            return *this;
        }
    } ATTR_PACKED ;
    #pragma pack(1)

    buffer_t buffer;
public:
    PublicKeyDescriptor(uint8_t e[4], uint8_t n[128])
    {
        assert(e != 0);
        assert(n != 0);

        buffer.tag = TagPublicKeyDesc;
        buffer.length = sizeof(buffer.e) + sizeof(buffer.n);
        memcpy(buffer.e, e, sizeof(buffer.e));
        memcpy(buffer.n, n, sizeof(buffer.n));
    }

    PublicKeyDescriptor() {}

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

#endif //gehua_certmgr_publickey_desc_h_
