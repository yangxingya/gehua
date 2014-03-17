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
#include "../widget.h"
#include "../../../include/cpplib/timetool.h"

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
    static string GenerateChallengeCode(string const& caid, uint32_t ip)
    {
        static uint32_t gs_cnt = 0;
        #pragma pack(1)
        struct tmp_t {
            uint32_t ip;
            uint32_t tm;
            uint32_t cnt;
            uint32_t caid;
        } ATTR_PACKED ;
        #pragma pack()

        tmp_t tmp;

        bool ok;
        uint64_t id = to_uint64(caid, &ok);

        //TODO:: don't check ok???
        tmp.caid = (uint32_t)(id & 0x00000000FFFFFFFF);
        tmp.ip = ip;
        tmp.tm = (uint32_t)get_up_time();
        
        //TODO:: the following '++gs_cnt' should be a atomic operate
        tmp.cnt = ++gs_cnt;
        
        return hex_string((uint8_t*)&tmp, sizeof(tmp));
    }

    /*
    *@brief: generate cert by user id
    */
    static string GenerateCert(string const& caid)
    {
        return "";
    }

    /*
    *@brief: save cert to user id map
    */
    void SaveCert(string const& caid, string const& cert)
    {
    
    }

    /*
    *@brief: valid cert by user id
    */
    bool ValidCert(string const& userid, string const& cert) const
    {
        return false;
    }

private:
    map<string, string> cert_map_;
};

#endif //!defined gehua_certmgr_h_
