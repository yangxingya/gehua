/*
 *
 */

#if !defined gehua_certmgr_h_
#define gehua_certmgr_h_

#include <string>
#include <map>

using ::std::string;
using ::std::map;

class CertMgr
{
public:
	static string GenerateChallengeCode(string const& userid, string const& ip);
	static string GenerateCert(string const& userid);

	void SaveCert(string const& userid, string const& cert);
	bool ValidCert(string const& userid, string const& cert);

private:
	map<string, string> cert_map_;
};

#endif //!defined gehua_certmgr_h_
