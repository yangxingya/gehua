/*
 *@brief: "certmgr.h" file is for user cert manage.
 *
 *@note: 
 */

#if !defined gehua_certmgr_h_
#define gehua_certmgr_h_

#include <string>
#include <vector>
#include <map>

using ::std::vector;
using ::std::string;
using ::std::map;

class CertMgr
{
public:
	/*
	 *@brief: generate challenge code by user id and local ip,
	 *        challenge code is 16(bytes) string.
	 */
	static string GenerateChallengeCode(string const& userid, string const& ip);
	/*
	 *@brief: generate cert by user id
	 */
	static string GenerateCert(string const& userid);

	/*
	 *@brief: save cert to user id map
	 */
	void SaveCert(string const& userid, string const& cert);
	
	/*
	 *@brief: valid cert by user id
	 */
	bool ValidCert(string const& userid, string const& cert) const;

private:
	map<string, string> cert_map_;
};

#endif //!defined gehua_certmgr_h_
