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

struct SignatureDescriptor 
{
private:
    #pragma pack(1)
    struct buffer_t {
        uint8_t tag;
        uint16_t length;
        uint32_t keyid;
        uint8_t sign[128];
        buffer_t(buffer_t const& rhs) { *this = rhs; }
        buffer_t& operator=(buffer_t const& rhs)
        {
            if (this == &rhs) return *this;
            tag = rhs.tag;
            length = rhs.length;
            keyid = rhs.keyid;
            memcpy(sign, rhs.sign, sizeof(sign));

            return *this;
        }
    } ATTR_PACKED ;
    #pragma pack(1)

    buffer_t buffer;
public:
    SignatureDescriptor(uint32_t keyid, uint8_t sign[128])
    {
        assert(e != 0);
        assert(n != 0);

        buffer.tag = TagSignatureDesc;
        buffer.length = sizeof(buffer.keyid) + sizeof(buffer.sign);
        memcpy(&buffer.keyid, keyid, sizeof(buffer.keyid));
        memcpy(buffer.sign, sign, sizeof(buffer.sign));
    }

    SignatureDescriptor() {}

    uint32_t length() const { return sizeof(buffer); }

    void maker(uint8_t *buff) const
    {
        assert(buff != 0);

        buffer_t buf;
        buf.tag = buffer.tag;
        buf.length = buffer.length;
        memcpy(&buf.keyid, buffer.keyid, sizeof(buffer.keyid));
        memcpy(buf.sign, buffer.sign, sizeof(buffer.sign));
        if (g_byteorder == OrderLittleEndian) {
            buf.tag = buffer.tag;
            buf.length = change_order(buffer.length);
            buf.keyid = change_order(buf.keyid);
            change_order(buf.sign);
        } 

        memcpy(buff, &buf, sizeof(buf));
    }
};

#endif // !gehua_certmgr_sign_desc_h_
