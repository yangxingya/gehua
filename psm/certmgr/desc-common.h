/*
 *
 */


#if !defined gehua_certmgr_desccomm_h_
#define gehua_certmgr_desccomm_h_  

#include <string>
#include <map>
#include <vector>

namespace gehua {
namespace cert {

using ::std::string;
using ::std::map;
using ::std::vector;

enum CommonDescriptorTag {
	TagCertIdDesc = ,
	TagCertUserInfoDesc = ,
	TagOperatorInfoDesc = ,
	TagPublicKeyDesc = ,
	TagExpireTimeDesc = ,
	TagSignatureDesc = ,
};

enum ByteOrder {
	OrderLittleEndian = 1,
	OrderBigEndian = 2,
	OrderNet = OrderBigEndian,
}

inline ByteOrder test_byte_order()
{
	uint16_t tmp = 0x1234;
	uint8_t *ptmp = &tmp;
	if (ptmp[0] == 0x12)
		return OrderLittleEndian;
	return OrderBigEndian;
}

static ByteOrder g_byteorder = test_byte_order();

template<typename T>
inline T change_order(T const& t)
{
	T tmp;

	uint8_t *src = (uint8_t*)&t;
	uint8_t *dst = (uint8_t*)&tmp;
	for (size_t i = 0; i < sizeof(t); ++i) {
		dst[i] = src[sizeof(t) - i];
	}

	return tmp;
}

inline uint8_t* change_order(uint8_t* t, size_t sz)
{
	uint8_t tmp;
	size_t half = sz / 2;
	for (size_t i = 0; i < half; ++i) {
		tmp = t[i];
		t[i] = t[sz - i - 1];
		t[sz - i - 1] = tmp;
	}

	return t;
}

template<>
inline string change_order(string const& t)
{
	string dest;
	size_t len = t.length();
	dest.reserve(len);

	for (size_t i = 0; i < len; ++i)
		dest[i] = t[len - i];

	return dest;
}

inline size_t split(string const& str, string const& sp, 
                    vector<string> *out)
{
  string::size_type pos1, pos2;

  size_t origin_sz = out->size();
  pos1 = 0;      
  while ((pos2 = str.find(sp, pos1)) != string::npos) {
    out->push_back(str.substr(pos1, pos2 - pos1));
    pos1 = pos2 + sp.length();
  }
  out->push_back(str.substr(pos1));

  return (size_t)(out->size() - origin_sz);
}

//split string likely "key1=value1&key2=value2..." to
// map["key1"] = "value1", map["key2"] = "value2".
inline size_t split(string const& str, string const& sp1, 
                    string const& sp2, map<string, string> *out)
{
  vector<string> strvec;

  split(str, sp1, &strvec);

  map<string, string> kv_map;

  for (size_t i = 0; i < strvec.size(); ++i) {
    pos1 = strvec[i].find(sp2);
    if (string::npos == pos1) {
      kv_map.clear();
      break;
    }
    
    kv_map[strvec[i].substr(0, pos1 - 1)] = strvec[i].substr(pos1 + sp2.length());
  }

  *out = kv_map;

  return kv_map.size();
}

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

    user_id = out[USERINFO_STR_USERID_KEY];
    term_id = out[USERINFO_STR_TERMID_KEY];
    mac_addr = out[USERINFO_STR_TERMMAC_KEY];

    //one valid element [optional segment].
    if ((it = out.find(USERINFO_STR_CARDID_KEY)) != out.end()) {
      card_id = out[USERINFO_STR_CARDID_KEY];
    }
  }
private:
  bool valid_;
};

inline bool is_number(char c)
{
  return c >= '0' && c <= '9';
}

inline uint64_t to_uint64(string const& str, bool *cast_ok)
{
  // default is 10 not 16 band
  uint64_t ret = 0;
  for (size_t i = 0; i < str.length(); ++i) {
    if (!is_number(str[i])) {
      *cast_ok = false;
      return 0;
    }
    ret *= 10;
    ret += str[i] - '0';
  }

  *cast_ok = true;
  return ret;
}

} // namespace cert
} // namespace gehua

#endif //!gehua_certmgr_desccomm_h_
