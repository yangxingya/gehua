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

namespace gehua {
namespace cert {

using ::std::string;
using ::std::map;

struct CertUiDescriptor 
{
private:
	struct buffer_t {
		uint8_t tag;
		uint16_t length;
	}__attribute__((packed));
	
	buffer_t buffer;
	string user_info;
public:
	CertUiDescriptor(map<string, string> const& ui_kv_pair)
	{
		buffer.tag = TagCertUserInfoDesc;
		map<string, string>::const_iterator it = ui_kv_pair.begin();
		for (/* without init */; it != ui_kv_pair.end(); ++it) {
			user_info += it->first + "=" + it->second;
			user_info += "&";
		}

		buffer.length = user_info.length();
	}

	uint32_t length() const { return sizeof(buffer) + user_info.length(); }

	void maker(uint8_t *buff) const
	{
		assert(buff != 0);

		buffer_t buf = buffer;
		string ui = user_info;
		if (g_byteorder == OrderLittleEndian) {
			buf.tag = buffer.tag;
			buf.len = change_order(buffer.length);
			ui = change_order(user_info);
		} 

		memcpy(buff, &buf, sizeof(buf));
		memcpy(&buff[sizeof(buf)], ui.c_str(), ui.length());
	}
};

} // namespace gehua
} // namespace cert
#endif //gehua_certmgr_certui_desc_h_
