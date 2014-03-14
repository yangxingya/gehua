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

struct OperatorInfoDescriptor 
{
private:
    #pragma pack(1)
    struct buffer_t {
        uint8_t tag;
        uint16_t length;
        uint32_t id;
    } ATTR_PACKED ;
    #pragma pack(1)

    buffer_t buffer;
    string name;
public:
    OperatorInfoDescriptor(uint32_t id, string const& name)
    {
        buffer.tag = TagOperatorInfoDesc;
        buffer.id = id;
        this->name = name;
        buffer.length = sizeof(buffer.id) + this->name.length() + 1;
    }

    uint32_t length() const { return buffer.length + 3; }

    void maker(uint8_t *buff) const
    {
        assert(buff != 0);

        buffer_t buf = buffer;
        string na = name;
        if (g_byteorder == OrderLittleEndian) {
            buf.tag = buffer.tag;
            buf.len = change_order(buffer.length);
            buf.id = change_order(buffer.id);
            na = change_order(name);
        } 

        memcpy(buff, &buf, sizeof(buf));
        //todo::??? name element is string<8> ??? or string<0>
        uint8_t nl = name.length();
        memcpy(&buff[sizeof(buf)], &nl, sizeof(nl));
        memcpy(&buff[sizeof(buf)+sizeof(nl)], na.c_str(), na.length());
    }
};

#endif //gehua_certmgr_opertaori_desc_h_
