/* @brief: cert data descriptor, cert data info
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


#if !defined gehua_certmgr_certdata_desc_h_
#define  gehua_certmgr_certdata_desc_h_

#include <assert.h>
#include "desc-common.h"
#include "certid-desc.h"
#include "certui-desc.h"
#include "operatori-desc.h"
#include "publickey-desc.h"
#include "expiretime-desc.h"
#include "sign-desc.h"

struct UserCertData 
{
    CertIdDescriptor certid;
    CertUiDescriptor certui;
    OperatorInfoDescriptor opinfo;
    PublicKeyDescriptor pubkey;
    ExpireTimeDescriptor exptime;
    SignatureDescriptor sign;
private:
    enum { kMagic = 0xFBEA8011, };
    uint32_t magic;
    uint16_t dlen;
public:
    UserCertData() : magic(kMagic) {}

    uint32_t length() const 
    { 
        return sizeof(magic) + sizeof(dlen) +
            certid.length() + certui.length() + opinfo.length() +
            pubkey.length() + exptime.length() + sign.length();
    }

    void maker(uint8_t *buff) const
    {
        assert(buff != 0);

        uint32_t mag = magic;
        if (g_byteorder == OrderLittleEndian)
            mag = change_order(magic);

        uint16_t dl = dlen;
        if (g_byteorder == OrderLittleEndian)
            change_order(dlen);
        
        // magic
        uint8_t *offset = buff;
        memcpy(offset, &mag, sizeof(mag));

        // dlen
        offset += sizeof(mag);
        memcpy(offset, &dl, sizeof(dlen));

        // certid
        offset += sizeof(dl);
        certid.maker(offset);

        // certui
        offset += certid.length();
        certui.maker(offset);

        // opinfo
        offset += certui.length();
        opinfo.maker(offset);

        // pubkey
        offset += opinfo.length();
        pubkey.maker(offset);

        // exptime
        offset += pubkey.length();
        exptime.maker(offset);

        // sign
        offset += exptime.length();
        sign.maker(offset);
    }
};

#endif // !gehua_certmgr_certdata_desc_h_