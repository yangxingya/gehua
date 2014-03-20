/*
*
*/


#if !defined gehua_certmgr_desccomm_h_
#define gehua_certmgr_desccomm_h_  

#include <string>
#include <map>
#include <vector>
#include "../widget.h"
#include "../../../include/cpplib/bytestream.h"

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

    UserInfo(string const& user_info, bool case_sensitivity = false)
        : valid_(false)
    {
        map<string, string> out;
        split(user_info, USERINFO_STR_SP1, USERINFO_STR_SP2, &out, case_sensitivity);

        string uid = case_sensitivity ? USERINFO_STR_USERID_KEY : to_lower(USERINFO_STR_USERID_KEY);
        string tid = case_sensitivity ? USERINFO_STR_TERMID_KEY : to_lower(USERINFO_STR_TERMID_KEY);
        string mac = case_sensitivity ? USERINFO_STR_TERMMAC_KEY : to_lower(USERINFO_STR_TERMMAC_KEY);
        string cid = case_sensitivity ? USERINFO_STR_CARDID_KEY : to_lower(USERINFO_STR_CARDID_KEY);
        //three valid element. [must be existed].
        map<string, string>::iterator it;
        if ((it = out.find(uid)) == out.end()) return;
        if ((it = out.find(tid)) == out.end()) return;
        if ((it = out.find(mac)) == out.end()) return;

        id = out[uid];
        term_id = out[tid];
        mac_addr = out[mac];

        //one valid element [optional segment].
        if ((it = out.find(cid)) != out.end()) {
            card_id = out[cid];
        }
        valid_ = true;
    }
    bool valid() const { return valid_; }
    bool operator==(UserInfo const& rhs)
    {
        return id == rhs.id && card_id == rhs.card_id &&
            term_id == rhs.term_id;
    }

    bool operator!=(UserInfo const& rhs)
    {
        return !(*this == rhs);
    }
private:
    bool valid_;
};

inline bool operator<(UserInfo const& fst, UserInfo const& rhs)
{
    if (fst.card_id < rhs.card_id) return true;
    if (fst.card_id > rhs.card_id) return false;

    if (fst.term_id < rhs.term_id) return true;
    if (fst.term_id > rhs.term_id) return false;

    if (fst.id < rhs.id) return true;
    if (fst.id > rhs.id) return false;

    return false;
}


struct DescBase
{
    uint8_t  tag_;
    uint16_t length_;

    DescBase() {}
    virtual ~DescBase() {}
    virtual ByteStream getStream() = 0;

    uint32_t length() const { return sizeof(tag_) + sizeof(length_) + length_; }
};

#endif //!gehua_certmgr_desccomm_h_
