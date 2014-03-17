/*
*
*/


#if !defined gehua_certmgr_desccomm_h_
#define gehua_certmgr_desccomm_h_  

#include <string>
#include <map>
#include <vector>
#include "../widget.h"

using ::std::string;
using ::std::map;
using ::std::vector;

enum CommonDescriptorTag {
    TagCertIdDesc = 0x01,
    TagCertUserInfoDesc = 0x02,
    TagOperatorInfoDesc = 0x03,
    TagPublicKeyDesc = 0x04,
    TagSignatureDesc = 0x05,
    TagExpireTimeDesc = 0x06,
};

#define USERINFO_STR_SP1 "&"
#define USERINFO_STR_SP2 "="

#define USERINFO_STR_USERID_KEY  "UserID"
#define USERINFO_STR_CARDID_KEY  "CardID"
#define USERINFO_STR_TERMID_KEY  "TerminalID"
#define USERINFO_STR_TERMMAC_KEY "MAC"

struct UserInfo
{
    string id;
    string card_id;
    string term_id;
    string mac_addr;

    UserInfo(string const& user_info)
        : valid_(true)
    {
        map<string, string> out;
        split(user_info, USERINFO_STR_SP1, USERINFO_STR_SP2, &out);

        //three valid element. [must be existed].
        map<string, string>::iterator it;
        if ((it = out.find(USERINFO_STR_USERID_KEY)) == out.end()) {
            valid_ = false;
            return;
        }
        if ((it = out.find(USERINFO_STR_TERMID_KEY)) == out.end()) {
            valid_ = false;
            return;
        }
        if ((it = out.find(USERINFO_STR_TERMMAC_KEY)) == out.end()) {
            valid_ = false;
            return;
        }

        id = out[USERINFO_STR_USERID_KEY];
        term_id = out[USERINFO_STR_TERMID_KEY];
        mac_addr = out[USERINFO_STR_TERMMAC_KEY];

        //one valid element [optional segment].
        if ((it = out.find(USERINFO_STR_CARDID_KEY)) != out.end()) {
            card_id = out[USERINFO_STR_CARDID_KEY];
        }
    }
    bool valid() const { return valid_; }
private:
    bool valid_;
};

#endif //!gehua_certmgr_desccomm_h_
