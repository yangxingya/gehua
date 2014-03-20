/* @brief: cert user info descriptor, cert data element
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


#if !defined gehua_certmgr_certui_desc_h_
#define  gehua_certmgr_certui_desc_h_

#include <assert.h>
#include <string>
#include <map>
#include "desc-common.h"

using ::std::string;
using ::std::map;

struct CertUiDescriptor : public DescBase
{
    string user_info_;

    CertUiDescriptor(map<string, string> const& ui_kv_pair)
    {
        
        map<string, string>::const_iterator it = ui_kv_pair.begin();
        for (/* without init */; it != ui_kv_pair.end(); ++it) {
            user_info_ += it->first + "=" + it->second;
            user_info_ += "&";
        }

        tag_ = TagCertUserInfoDesc;
        length_ = user_info_.length();
    }

    CertUiDescriptor(string const& str)
        : user_info_(str)
    {
        tag_ = TagCertUserInfoDesc;
        length_ = user_info_.length();
    }
    

    CertUiDescriptor() 
    {
        user_info_ = "";
        tag_ = TagCertUserInfoDesc;
        length_ = user_info_.length();
    }

    virtual ByteStream getStream()  
    {
        ByteStream bs;
        bs.SetByteOrder(NETWORK_BYTEORDER);

        bs.PutUint8(tag_);
        bs.PutUint16(length_);

        // todo:: 
        bs.PutString16(user_info_);

        return bs;
    }

    bool operator==(CertUiDescriptor const& rhs)
    {
        return user_info_ == rhs.user_info_;
    }

    bool operator!=(CertUiDescriptor const& rhs)
    {
        return !(*this == rhs);
    }
};



#endif //gehua_certmgr_certui_desc_h_
