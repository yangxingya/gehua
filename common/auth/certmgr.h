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
#include <cpplib/bytestream.h>
#include "../widget.h"
#include "desc-common.h"
#include "../../../include/cpplib/timetool.h"

using ::std::vector;
using ::std::string;
using ::std::map;

class CertMgr
{
public:

    static uint32_t LocalHostIp()
    {
        static uint32_t ip = ip_cast(ip_string("*"));
        return ip;
    }

    /*
    *@brief: generate challenge code by user id and local ip,
    *        challenge code is 16(bytes) string.
    */
    static ByteStream GenerateChallengeCode(string const& caid, ByteStream &bs)
    {
        static uint32_t gs_cnt = 0;
        struct tmp_t {
            uint32_t ip;
            uint32_t tm;
            uint32_t cnt;
            uint32_t caid;
        };

        tmp_t tmp;

        bool ok;
        uint64_t id = to_uint64(caid, &ok);

        //TODO:: don't check ok???
        tmp.caid = (uint32_t)(id & 0x00000000FFFFFFFF);
        tmp.ip = LocalHostIp();
        tmp.tm = (uint32_t)get_up_time();
        
        //TODO:: the following '++gs_cnt' should be a atomic operate
        tmp.cnt = ++gs_cnt;

        bs.Add(&tmp, sizeof(tmp));
        
        return bs;
    }

    static uint64_t GenerateCertId(UserInfo const& userinfo)
    {
        // 
        uint64_t caid;
        bool ok;
        caid = to_uint64(userinfo.card_id, &ok);

        string userid = userinfo.id;
        string termid = userinfo.term_id;
        
        string uid_num = "";
        for (size_t i = 0; i < userid.length(); ++i) {
            if (userid[i] >= '0' && userid[i] <= '9')
                uid_num += userid[i];
        }

        string tid_num = "";
        for (size_t i = 0; i < termid.length(); ++i) {
            if (termid[i] >= '0' && termid[i] <= '9')
                tid_num += termid[i];
        }

        uint64_t uid = to_uint64(uid_num, &ok);
        uint64_t tid = to_uint64(tid_num, &ok);

        ByteStream bs;
        bs.PutUint16((uint16_t)uid);
        bs.PutUint16((uint16_t)tid);
        bs.PutUint32((uint32_t)caid);

        return bs.GetUint64();
    }

    static ByteStream GenerateRootKey(UserInfo const& userinfo)
    {
        //

        uint8_t buf[128];
        memset(buf, 0, sizeof(buf));

        ByteStream bs;
        bs.Add(buf, sizeof(buf));

        return bs;
    }

    static ByteStream GeneratePublicEncryptCipherData(UserInfo const& userinfo)
    {
        //被加密数据格式为Counter||KEY||M:
        // Counter固定为0（32比特）
        // 被加密数据为加密MessageCont()的密钥KEY（密钥随机产生，128比特）；
        // M=CMAC(KEY,Counter||MessageCont())，长度为0x04；
        // MessageCont()采用ChaCha算法用KEY进行序列加密。

        uint32_t counter = 0;
        uint32_t key[16];
        uint32_t m = 0;
        
        memset(key, 0, sizeof(key));

        ByteStream bs;
        bs.PutUint32(counter);
        bs.Add(key, sizeof(key));
        bs.PutUint32(m);

        // pad for length to 0x80 [keyid used 4 bytes];
        uint8_t pad[0x80 - sizeof(counter) - sizeof(key) - sizeof(m)];
        memset(pad, 0, sizeof(pad));
        bs.Add(pad, sizeof(pad));

        return bs;
    }

    /*
    *@brief: generate cert by user id
    */
    UserCert* GenerateCert(Logger &logger, UserInfo const& userinfo, string const& ui_str)
    {
        map<UserInfo, UserCert*>::iterator it;
        it = cert_map_.find(userinfo);
        if (it != cert_map_.end()) 
            return it->second;

        uint32_t operid   = 0;
        string   opername = "gehua";
        UserCert *cert = new UserCert(logger, CertMgr::GenerateCertId(userinfo), ui_str, operid, opername);
        cert_map_[userinfo] = cert;

        return cert;
    }

    /*
    *@brief: valid cert by user id
    */
    bool ValidCert(UserInfo const& userinfo, ByteStream const& certbs) const
    {
        map<UserInfo, UserCert*>::const_iterator cit;
        cit = cert_map_.find(userinfo);
        if (cit == cert_map_.end()) return false;

        UserCert *cert = cit->second;
        
        return const_cast<ByteStream&>(certbs) == cert->getStream();
    }

    bool ValidCert(UserInfo const& userinfo, UserCert const& usercert) const
    {
        map<UserInfo, UserCert*>::const_iterator cit;
        cit = cert_map_.find(userinfo);
        if (cit == cert_map_.end()) return false;

        UserCert *cert = cit->second;
        
        return *cert == usercert;
    }

    static CertMgr& instance() 
    {
        return instance_;
    }

    void AddChallengeCode(UserInfo const& userinfo, vector<uint8_t> challengecode)
    {
        challenge_code_map_[userinfo] = challengecode;
    }

    vector<uint8_t> GetChallengeCode(UserInfo const& userinfo)
    {
        map<UserInfo, vector<uint8_t> >::const_iterator cit;
        cit = challenge_code_map_.find(userinfo);
        if (cit != challenge_code_map_.end()) 
            return cit->second;

        return vector<uint8_t>();
    }

private:
    map<UserInfo, UserCert*> cert_map_;
    map<UserInfo, vector<uint8_t> > challenge_code_map_;

    static CertMgr instance_;

    CertMgr() {}
};

CertMgr CertMgr::instance_;

#endif //!defined gehua_certmgr_h_
