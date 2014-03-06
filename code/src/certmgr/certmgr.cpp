#include "certmgr.h"

string CertMgr::GenerateChallengeCode(string const& userid, string const& ip)
{
	return userid + ip;
}

string CertMgr::GenerateCert(string const& userid)
{
	return userid;
}

void CertMgr::SaveCert(string const& userid, string const& cert)
{
	cert_map_[userid] = cert;
}

bool CertMgr::ValidCert(string const& userid, string const& cert)
{
	map<string, string>::iterator it = cert_map_.find(userid);
	if (it == cert_map_.end())
		return false;

	return it->second == cert;
}
