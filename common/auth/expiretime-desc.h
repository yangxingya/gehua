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
    #pragma pack(1)
    struct buffer_t {
        uint8_t tag;
        uint16_t length;
        uint8_t time[7];
    } ATTR_PACKED ;
    #pragma pack(1)

    buffer_t buffer;
public:
    ExpireTimeDescriptor(uint8_t time[7])
    {
        assert(time != 0);

        buffer.tag = TagExpireTimeDesc;
        buffer.length = sizeof(buffer.time);
        memcpy(buffer.time, time, sizeof(buffer.time));
    }

    ExpireTimeDescriptor() {}

    uint32_t length() const { return sizeof(buffer); }

    void maker(uint8_t *buff) const
    {
        assert(buff != 0);

        buffer_t buf;
        buf.tag = buffer.tag;
        buf.length = buffer.length;
        memcpy(buf.time, buffer.time, sizeof(buffer.time));
        if (g_byteorder == OrderLittleEndian) {
            buf.tag = buffer.tag;
            buf.length = change_order(buffer.length);
            change_order(buf.time);
        } 

        memcpy(buff, &buf, sizeof(buf));
    }
};

#endif // !gehua_certmgr_expiretime_desc_h_
