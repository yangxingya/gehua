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

struct UserCert 
{
    CertIdDescriptor certid_desc_;
    CertUiDescriptor certui_desc_;
    OperatorInfoDescriptor opinfo_desc_;
    PublicKeyDescriptor pubkey_desc_;
    ExpireTimeDescriptor exptime_desc_;
    SignatureDescriptor sign_desc_;
private:
    enum { kMagic = 0xFBEA8011, };
    uint32_t magic;
    uint16_t dlen;
public:
    UserCert(uint64_t certid, string const& userinfo,
             uint32_t operid, string const& opername) 
        : magic(kMagic) 
        , certid_desc_(certid)
        , certui_desc_(userinfo)
        , opinfo_desc_(operid, opername)
        , pubkey_desc_()
        , exptime_desc_()
        , sign_desc_()
    {
        dlen = certid_desc_.length() + certui_desc_.length() + opinfo_desc_.length() +
               pubkey_desc_.length() + exptime_desc_.length() + sign_desc_.length();
    }

    UserCert(ByteStream &usercert, bool &valid) 
        : certid_desc_()
        , certui_desc_()
        , opinfo_desc_()
        , pubkey_desc_()
        , exptime_desc_()
        , sign_desc_()
    {
        magic = usercert.GetUint32();
        dlen  = usercert.GetUint16();

        if (magic != kMagic) {
            valid = false;
            return;
        }

        uint8_t tag;
        uint16_t len;

        bool have_certid = false;
        bool have_certui = false;
        bool have_operif = false;
        bool have_pubkey = false;
        bool have_signal = false;
        bool have_exptim = false;
        
        int sum_desc = 6;
        int last_desc = 0;
        for (int i = 0; i < sum_desc; ++i) {
            tag = usercert.GetUint8();
            len = usercert.GetUint16();
            switch (tag) {
            case TagCertIdDesc:
                certid_desc_ = CertIdDescriptor(usercert.GetUint64());
                have_certid = true;
                break;
            case TagCertUserInfoDesc:
                certui_desc_ = CertUiDescriptor(usercert.GetString(len));
                have_certui = true;
                break;
            case TagOperatorInfoDesc: 
                {
                    uint32_t operid   = usercert.GetUint32();
                    string   opername = usercert.GetString8();
                    opinfo_desc_ = OperatorInfoDescriptor(operid, opername);
                    have_operif = true;
                }
                break;
            case TagPublicKeyDesc:
                {
                    uint8_t e[4];
                    uint8_t n[128];
                    usercert.Get(e, sizeof(e));
                    usercert.Get(n, sizeof(n));
                    pubkey_desc_ = PublicKeyDescriptor(e, n);
                    have_pubkey = true;
                }
                break;
            case TagSignatureDesc:
                {
                    uint32_t keyid;
                    uint8_t  sign[128];

                    keyid = usercert.GetUint32();
                    usercert.Get(sign, sizeof(sign));

                    sign_desc_ = SignatureDescriptor(keyid, sign);
                    last_desc = i;
                    have_signal = true;
                }
                break;
            case TagExpireTimeDesc:
                {
                    uint8_t time[7];
                    usercert.Get(time, sizeof(time));
                    exptime_desc_ = ExpireTimeDescriptor(time);
                    have_exptim = true;
                }
                break;
            default:
                valid = false;
                return;
            }
        }

        if (last_desc != sum_desc - 1) {
            valid = false;
            return;
        }

        if (!have_certid ||
            !have_certui ||
            !have_operif ||
            !have_pubkey ||
            !have_signal ||
            !have_exptim) {
                valid = false;
                return;
        }
        valid = true;
    }

    

    ByteStream getStream() 
    {
        ByteStream bs;
        bs.SetByteOrder(NETWORK_BYTEORDER);

        bs.PutUint32(magic);
        bs.PutUint16(dlen);

        bs.Add(certid_desc_.getStream());
        bs.Add(certui_desc_.getStream());
        bs.Add(opinfo_desc_.getStream());
        bs.Add(pubkey_desc_.getStream());
        bs.Add(exptime_desc_.getStream());
        bs.Add(sign_desc_.getStream());

        return bs;
    }

    bool operator==(UserCert const& rhs)
    {
        return  certid_desc_ == rhs.certid_desc_ &&
                certui_desc_ == rhs.certui_desc_ &&
                opinfo_desc_ == rhs.opinfo_desc_ &&
                pubkey_desc_ == rhs.pubkey_desc_ &&
                exptime_desc_ == rhs.exptime_desc_ &&
                sign_desc_ == rhs.sign_desc_;
    }

    bool operator!=(UserCert const& rhs)
    {
        return !(*this == rhs);
    }
};



#endif // !gehua_certmgr_certdata_desc_h_