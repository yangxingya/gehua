/*
 * @brief: cert id descriptor, cert data element
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


#if !defined gehua_certmgr_certid_desc_h_
#define  gehua_certmgr_certid_desc_h_

#include <assert.h>
#include "desc-common.h"

namespace gehua {
namespace cert {

struct CertIdDescriptor 
{
	CertIdDescriptor(uint64_t id)
	{
		buffer.tag = TagCertIdDesc;
		buffer.length = sizeof(buffer.id);
		buffer.id = id;
	}

	uint32_t length() const { return sizeof(buffer); }

	void maker(uint8_t *buff) const
	{
		assert(buff != 0);

		buffer_t buf = buffer;
		if (g_byteorder == OrderLittleEndian) {
			buf.tag = buffer.tag;
			buf.len = change_order(buffer.length);
			buf.id = change_order(buffer.id);			
		} 

		memcpy(buff, &buf, sizeof(buf));
	}
private:
	struct buffer_t {
		uint8_t tag;
		uint16_t length;
		uint64_t id;
	};

	buffer_t buffer;
};

} // namespace gehua
} // namespace cert
j
#endif //gehua_certmgr_certid_desc_h_
