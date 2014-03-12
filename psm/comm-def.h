/*
 * @brief: common define.
 *
 */

#if !defined gehua_common_define_h_
#define gehua_common_define_h_

#include <string>
#include <vector>
#include <map>
#ifdef _MSC_VER
#include <cpplib/cpplibbase.h>
#endif // _MSC_VER

#ifdef _MSC_VER
typedef unsigned __int64 caid_t;
#else // _MSC_VER
# ifdef __GUNC__
typedef uint64_t caid_t;
# endif // __GUNC__
#endif // !_MSC_VER

enum ByteOrder {
	OrderLittleEndian = 1,
	OrderBigEndian = 2,
	OrderNet = OrderBigEndian,
};

inline ByteOrder test_byte_order()
{
	uint16_t tmp = 0x1234;
	uint8_t *ptmp = (uint8_t*)&tmp;
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

using ::std::string;
using ::std::vector;
using ::std::map;

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

  string::size_type pos;
  for (size_t i = 0; i < strvec.size(); ++i) {
    pos = strvec[i].find(sp2);
    if (string::npos == pos) {
      kv_map.clear();
      break;
    }
    
    kv_map[strvec[i].substr(0, pos - 1)] = strvec[i].substr(pos + sp2.length());
  }

  *out = kv_map;

  return kv_map.size();
}

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
    ret = ret * 10;
    ret = ret + (str[i] - '0');
  }

  *cast_ok = true;
  return ret;
}

#endif //!gehua_common_define_h_
