/*
 *
 */


#if !defined gehua_certmgr_desccomm_h_
#define gehua_certmgr_desccomm_h_  

#include <string>

namespace gehua {
namespace cert {

using ::std::string;

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



} // namespace cert
} // namespace gehua

#endif //!gehua_certmgr_desccomm_h_
