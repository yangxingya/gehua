/*
* @brief: request 
*/

#if !defined gehua_auth_agent_request_h_
#define gehua_auth_agent_request_h_

#include <string>
#include "../common/widget.h"
#include "../common/auth/usercert.h"
#include "../common/auth/certmgr.h"
#include "../common/auth/desc-common.h"
#include <cpplib/logger.h>
#include <protocol/protocol_v2_common.h>
#include <protocol/protocol_v2_general.h>
#include <protocol/protocol_v2_cipher.h>
#include <protocol/protocol_v2_pt_common.h>
#include <protocol/protocol_v2_pt_descriptor.h>
#include <protocol/protocol_v2_pt.h>
#include <protocol/protocol_v2_pt_message.h>

using ::std::string;

enum ReqType 
{
    RTGetSTBQRCode = 1,
    RTChallenge = 2,
    RTGetCert = 3,
    RTGetEntryAddr = 4,
    RTUnknown = 0xFF,
};

const int kReqStrArraySize = 4;
const string kReqStrArray[] = 
{
    "TERM_QRCODE_REQUEST",
    "TERM_CHALLENGE_REQUEST",
    "TERM_GET_CERT_REQUEST",
    "TERM_GET_ENTRY_REQUEST",
};

struct ReqBase
{
    ReqBase(Logger &logger, vector<string> const& lines, bool case_sensitivity)
        : logger_(logger), case_sensitivity_(case_sensitivity)
    {
        // content likely "OdcLibDesc=..."
        // change to map ---> map["odclibdesc"] = ... format.
        string::size_type pos;
        logger_.Info("开始解析-------------------------------");
        for (size_t i = 0; i < lines.size(); ++i) {
            logger_.Trace("第%d行：%s", i + 1, lines[i].c_str());
            if ((pos = lines[i].find_first_of("=")) != string::npos) {
                string key = lines[i].substr(0, pos);
                if (!case_sensitivity_)
                    key = to_lower(key);
                string suffix = lines[i].substr(pos + 1);
                string::size_type pos = suffix.rfind("\r\n");
                if (pos != string::npos)
                    if (pos == suffix.length() - 2)
                        suffix = suffix.substr(0, suffix.length() - 2);

                pos = suffix.rfind("\n");
                if (pos != string::npos)
                    if (pos == suffix.length() - 1)
                        suffix = suffix.substr(0, suffix.length() - 2);
                lines_[key] = suffix;
                logger_.Trace("\t解析结果[%s] = %s", key.c_str(), suffix.c_str());
            }
        }
        logger_.Trace("解析完毕-------------------------------");
    }

    virtual ~ReqBase() {}

    virtual bool Parse() = 0;

    virtual string MakeResponse(bool ok) = 0;

    virtual bool Valid() = 0;

    virtual string Name() const = 0;

    ReqType Type() const { return request_; }

    bool case_sensitivity() const { return case_sensitivity_; }

protected:
    Logger &logger_;
    ReqType request_;
    map<string, string> lines_;
private:
    bool case_sensitivity_;
};

struct ReqGetSTBQRCode : public ReqBase
{
    ReqGetSTBQRCode(Logger &logger, vector<string> const lines, bool case_sensitivity = false)
        : ReqBase(logger, lines, case_sensitivity)
    {
        logger_.Info("获取二维码请求...");
        request_ = RTGetSTBQRCode;
    }

    virtual string Name() const
    {
        return "获取二维码请求";
    }

    virtual bool Parse()
    {
        return false;
    }

    virtual bool Valid() 
    {
        return false;
    }

    virtual string MakeResponse(bool ok)
    {
        return "";
    }
};

const string kOdcLibPfx   = "OdcLib";
const string kUserInfoPfx = "UserInfo";
const string kTestDataPfx = "TestData";
const string kMacPfx      = "Mac";
const string kUserPublicKeyPfx = "UserPublicKey";
const string kChallengeCodePfx = "ChallengeCode";
const string kUserCertDataPfx = "UserCertData";

struct ReqChallenge : public ReqBase
{
    PT_OdcLibDescriptor   odclib_desc_;
    PT_UserInfoDescriptor userinfo_desc_;
    PT_TestDataDescriptor testdata_desc_;
    MacDescriptor         mac_desc_;

    UserInfo              *userinfo_;

    bool have_testdata_;

    ReqChallenge(Logger &logger, vector<string> const& lines, bool case_sensitivity = false)
        : ReqBase(logger, lines, case_sensitivity), userinfo_(0), have_testdata_(false)
    {
        logger_.Info("挑战请求...");
        request_ = RTChallenge;   
    }

    virtual ~ReqChallenge() 
    {
        delete userinfo_;
    }

    virtual string Name() const
    {
        return "挑战请求";
    }

    virtual bool Parse()
    {
        string odclibpfx   = kOdcLibPfx;
        string userinfopfx = kUserInfoPfx;
        string testdatapfx = kTestDataPfx;
        string macpfx      = kMacPfx;

        if (!case_sensitivity()){
            odclibpfx   = to_lower(kOdcLibPfx);
            userinfopfx = to_lower(kUserInfoPfx);
            testdatapfx = to_lower(kTestDataPfx);
            macpfx      = to_lower(kMacPfx);
        }

        map<string, string>::iterator it;

        // odc lib desc.
        if ((it = lines_.find(odclibpfx)) == lines_.end()) {
            logger_.Warn("挑战请求，找不到OdcLib=...行，解析失败");
            return false;
        }

        ByteStream odclibbs;
        odclibbs.SetByteOrder(NETWORK_BYTEORDER);
        odclibbs.PutHexString(it->second);
        // need check throw an exception.
        try {
            odclib_desc_ = Descriptor(odclibbs);
        } catch (...) {
            logger_.Warn("挑战请求，解析OdcLibDescriptor失败");
            return false;
        }

        // userinfo desc.
        if ((it = lines_.find(userinfopfx)) == lines_.end()) {
            logger_.Warn("挑战请求，找不到UserInfo=...行，解析失败");
            return false;
        }

        ByteStream userinfobs;
        userinfobs.SetByteOrder(NETWORK_BYTEORDER);
        userinfobs.PutHexString(it->second);
        // need check throw an exception.
        try {
            userinfo_desc_ = Descriptor(userinfobs);
        } catch (...) {
            logger_.Warn("挑战请求，解析UserInfoDescriptor失败");
            return false;
        }

        userinfo_ = new UserInfo(userinfo_desc_.user_info_);

        if (!userinfo_->valid()) {
            logger_.Warn("挑战请求，解析UserInfoDescriptor中的userinfo失败");
            return false;
        }

        // testdata desc. [optional segment]
        if ((it = lines_.find(testdatapfx)) != lines_.end()) {

            ByteStream testdatabs;
            testdatabs.SetByteOrder(NETWORK_BYTEORDER);
            testdatabs.PutHexString(it->second);
            // need check throw an exception.
            try {
                testdata_desc_ = Descriptor(testdatabs);
            } catch (...) {
                logger_.Warn("挑战请求，解析可选TestDataDescriptor失败");
                return false;
            }
            have_testdata_ = true;
        }

        if ((it = lines_.find(macpfx)) == lines_.end()) {
            logger_.Warn("挑战请求，找不到Mac=...行，解析失败");
            return false;
        }

        ByteStream macbs;
        macbs.SetByteOrder(NETWORK_BYTEORDER);
        macbs.PutHexString(it->second);
        // need check throw an exception.
        try {
            mac_desc_ = Descriptor(macbs);
        } catch (...) {
            logger_.Warn("挑战请求，解析MacDescriptor失败");
            return false;
        }

        return true;
    }

    virtual bool Valid() 
    {
        return true;
    }

    virtual string MakeResponse(bool ok)
    {
        // generate challenge code descriptor.


        //[ok/failed]"ReturnCode=retcode\r\n"
        //[ok]"ChallengeCode=";
        //[failed optional]"Redirect=RedirectDescriptor\r\n"
        //[ok/failed]"Mac=%s\r\n";
        string ret;

        string retcode = ok ? "0" : "1";
        //return code. default is 0->successful.
        ret += "ReturnCode="; 
        ret += retcode;
        ret += "\r\n";


        if (ok) {
            ByteStream ccode;
            ccode.SetByteOrder(NETWORK_BYTEORDER);

            string cardid = userinfo_ != NULL ? userinfo_->card_id : "12345678";
            CertMgr::GenerateChallengeCode(cardid, ccode);

            vector<uint8_t> challengecode;
            ByteStream tmpccode = ccode;

            challengecode.resize(tmpccode.Size());
            tmpccode.Get(&challengecode[0], tmpccode.Size());

            CertMgr::instance().AddChallengeCode(*userinfo_, challengecode);
            //challenge code 
            ret += "ChallengeCode=";
            ByteStream ccode_desc_bs = PT_ChallengeCodeDescriptor(ccode).SerializeFull();
            ret += ccode_desc_bs.DumpHex(ccode_desc_bs.Size());
            ret += "\r\n";
        }

        //test data [optional segment]
        if (have_testdata_) {
            ret += "TestData=";
            ByteStream testdata_desc_bs = testdata_desc_.SerializeFull();
            ret += testdata_desc_bs.DumpHex(testdata_desc_bs.Size());
            ret += "\r\n";
        }

        ret += "Mac=";
        ByteStream mac_desc_bs = mac_desc_.SerializeFull();
        ret += mac_desc_bs.DumpHex(mac_desc_bs.Size());
        ret += "\r\n";

        return ret;
    }
};

struct ReqGetCert : public ReqBase
{
    PT_OdcLibDescriptor        odclib_desc_;
    PT_UserInfoDescriptor      userinfo_desc_;
    PT_UserPublicKeyDescriptor userpubkey_desc_;
    PT_TestDataDescriptor      testdata_desc_;
    PT_ChallengeCodeDescriptor challenge_desc_;

    MacDescriptor              mac_desc_;
    MacPasswordDescriptor      macpasswd_desc_;

    bool                       have_challenge_;
    bool                       have_testdata_;
    bool                       is_mac_desc_;
    UserInfo                   *userinfo_;

    ReqGetCert(Logger &logger, vector<string> const& lines, bool case_sensitivity = false)
        : ReqBase(logger, lines, case_sensitivity)
        , have_challenge_(false)
        , have_testdata_(false)
        , is_mac_desc_(true)
        , userinfo_(0)
    {
        logger_.Info("获取证书请求...");
        request_ = RTGetCert;
    }

    virtual string Name() const
    {
        return "获取证书请求";
    }

    virtual bool Parse()
    {
        string odclibpfx    = kOdcLibPfx;
        string userinfopfx  = kUserInfoPfx;
        string pubkeypfx    = kUserPublicKeyPfx;
        string challengepfx = kChallengeCodePfx;
        string testdatapfx  = kTestDataPfx;
        string macpfx       = kMacPfx;

        if (!case_sensitivity()){
            odclibpfx    = to_lower(kOdcLibPfx);
            userinfopfx  = to_lower(kUserInfoPfx);
            pubkeypfx    = to_lower(kUserPublicKeyPfx);
            challengepfx = to_lower(kChallengeCodePfx);
            testdatapfx  = to_lower(kTestDataPfx);
            macpfx       = to_lower(kMacPfx);
        }

        map<string, string>::iterator it;

        // odc lib desc.
        if ((it = lines_.find(odclibpfx)) == lines_.end()) {
            logger_.Warn("获取证书，找不到OdcLib=...行，解析失败");
            return false;
        }

        ByteStream odclibbs;
        odclibbs.SetByteOrder(NETWORK_BYTEORDER);
        odclibbs.PutHexString(it->second);
        // need check throw an exception.
        try {
            odclib_desc_ = Descriptor(odclibbs);
        } catch (...) {
            logger_.Warn("获取证书，解析OdcLibDescriptor失败");
            return false;
        }



        // userinfo desc.
        if ((it = lines_.find(userinfopfx)) == lines_.end()) {
            logger_.Warn("获取证书，找不到UserInfo=...行，解析失败");
            return false;
        }
        ByteStream userinfobs;
        userinfobs.SetByteOrder(NETWORK_BYTEORDER);
        userinfobs.PutHexString(it->second);
        // need check throw an exception.
        try {
            userinfo_desc_ = Descriptor(userinfobs);
        } catch (...) {
            logger_.Warn("获取证书，解析UserInfoDescriptor失败");
            return false;
        }

        userinfo_ = new UserInfo(userinfo_desc_.user_info_);
        if (!userinfo_->valid()) {
            logger_.Warn("获取证书，解析UserInfoDescriptor中的userinfo失败");
            return false;
        }


        // public key desc.
        if ((it = lines_.find(pubkeypfx)) == lines_.end()) {
            logger_.Warn("获取证书，找不到UserPublicKey=...行，解析失败");
            return false;
        }

        ByteStream pubkeybs;
        pubkeybs.SetByteOrder(NETWORK_BYTEORDER);
        pubkeybs.PutHexString(it->second);
        // need check throw an exception.
        try {
            userpubkey_desc_ = Descriptor(pubkeybs);
        } catch (...) {
            logger_.Warn("获取证书，解析UserPublicKeyDescriptor失败");
            return false;
        }



        // challenge code desc.
        if ((it = lines_.find(challengepfx)) != lines_.end()) {
            ByteStream challengebs;
            challengebs.SetByteOrder(NETWORK_BYTEORDER);
            challengebs.PutHexString(it->second);
            // need check throw an exception.
            try {
                challenge_desc_ = Descriptor(challengebs);
            } catch (...) {
                logger_.Warn("获取证书，解析可选ChallengeCodeDescriptor失败");
                return false;
            }

            have_challenge_ = true;
        }



        // testdata desc. [optional segment]
        if ((it = lines_.find(testdatapfx)) != lines_.end()) {
            ByteStream testdatabs;
            testdatabs.SetByteOrder(NETWORK_BYTEORDER);
            testdatabs.PutHexString(it->second);
            // need check throw an exception.
            try {
                testdata_desc_ = Descriptor(testdatabs);
            } catch (...) {
                logger_.Warn("获取证书，解析可选TestDataDescriptor失败");
                return false;
            }
            have_testdata_ = true;
        }

        // mac or macpassword desc.
        if ((it = lines_.find(macpfx)) == lines_.end()) {
            logger_.Warn("获取证书，找不到Mac=...行，解析失败");
            return false;
        }

        ByteStream macbs;
        macbs.SetByteOrder(NETWORK_BYTEORDER);
        macbs.PutHexString(it->second);
        // need check throw an exception.
        Descriptor mac_desc;
        try {
            mac_desc = Descriptor(macbs);
        } catch (...) {
            logger_.Warn("获取证书，解析Mac[Password]Descriptor失败");
            return false;
        }
        switch (mac_desc.tag_) {
        case TAG_MacDescriptor:
            mac_desc_ = mac_desc;
            is_mac_desc_ = true;
            break;
        case TAG_MacPasswordDescriptor:
            macpasswd_desc_ = mac_desc;
            is_mac_desc_ = false;
            break;
        default:
            return false;
        }

        return true;
    }

    virtual bool Valid()
    {
        vector<uint8_t> ccode = CertMgr::instance().GetChallengeCode(*userinfo_);
        if (ccode.size() == 0) {
            if (have_challenge_) {
                logger_.Warn("获取证书请求，挑战码验证失败，找不到该用户的挑战码");
                return false;
            }
            return true;
        }

        if (!have_challenge_) {
            logger_.Warn("获取证书请求，挑战码验证失败，当前挑战码描述符不存在");
            return false;
        }

        ByteStream ccodebs = challenge_desc_.challenge_code_;
        if (ccodebs.Size() == 0) {
            logger_.Warn("获取证书请求，挑战码验证失败，当前挑战码为空");
            return false;
        }

        vector<uint8_t> reqccode;
        reqccode.resize(ccodebs.Size());

        if (ccode.size() != reqccode.size()) {
            logger_.Warn("获取证书请求，挑战码验证失败，挑战码与原来的挑战码大小不一致");
            return false;
        }

        ccodebs.Get(&reqccode[0], ccodebs.Size());  

        return !memcmp(&ccode[0], &reqccode[0], reqccode.size());
    }

    virtual string MakeResponse(bool ok)
    {
        //ReturnCode=0\r\n
        //EncryptData=被加密数据的十六进制字符串\r\n
        //PublicEncrypt=PublicEncryptDescriptor\r\n 
        //Redirect=RedirectDescriptor\r\n [failed optional]
        //TestData=TestDataDescriptor\r\n [optional]
        //Mac=MacDescriptor\r\n [failed ]

        //"ReturnCode=0\r\n"
        //[ok]"EncryptData=UserCertData=xxx&RootKey=xxx\r\n";
        //[ok]"PublicEncrypt=PublicEncryptDescriptor\r\n"
        //[optional segment]"TestData=TestDataDescriptor\r\n"
        //[failed ]"Mac=MacDescriptor"\r\n"
        string ret;

        //return code. default is 0->successful.
        ret += "ReturnCode=";
        ret += (ok ? "0" : "1");
        ret += "\r\n";


        if (ok) {
            UserCert *usercert = 
                CertMgr::instance().GenerateCert(logger_, *userinfo_, userinfo_desc_.user_info_);

            ByteStream certdata;
            certdata.SetByteOrder(NETWORK_BYTEORDER);
            certdata.PutByteStream16(usercert->getStream());
            PT_UserCertDataDescriptor certdata_desc(certdata);
            PT_RootKeyDescriptor rootkey_desc(CertMgr::GenerateRootKey(*userinfo_));

            ByteStream certdata_bs = certdata_desc.SerializeFull();
            ByteStream rootkey_bs  = rootkey_desc.SerializeFull();
            ret += "EncryptData=";
            
            // user cert data
            ret += "UserCertData=";
            ret += certdata_bs.DumpHex(certdata_bs.Size());

            // seperator
            ret += "&";

            // root key
            ret += "RootKey=";
            ret += rootkey_bs.DumpHex(rootkey_bs.Size());
            ret += "\r\n";

            // PublicEncryptDescriptor
            ret += "PublicEncrypt=";
            uint32_t key_id = 0;
            ByteStream pubencrypt_bs = PublicEncryptDescriptor(key_id, CertMgr::GeneratePublicEncryptCipherData(*userinfo_)).SerializeFull();
            ret += pubencrypt_bs.DumpHex(pubencrypt_bs.Size());
            ret += "\r\n";
        }

        //test data [optional segment]
        if (have_testdata_) {
            ret += "TestData=";
            ByteStream testdata_desc_bs = testdata_desc_.SerializeFull();
            ret += testdata_desc_bs.DumpHex(testdata_desc_bs.Size());
            ret += "\r\n";
        }

        if (!ok) {
            ret += "Mac=";
            ByteStream mac_bs = mac_desc_.SerializeFull();
            ret += mac_bs.DumpHex(mac_bs.Size());
            ret += "\r\n";
        }

        return ret;
    }
};

struct ReqGetEntryAddr : public ReqBase
{
    PT_OdcLibDescriptor        odclib_desc_;
    PT_UserCertDataDescriptor  usercert_desc_;
    PT_TestDataDescriptor      testdata_desc_;
    MacDescriptor              mac_desc_;
    bool                       have_testdata_;

    UserCert                   *user_cert_;

    ReqGetEntryAddr(Logger &logger, vector<string> const& lines, bool case_sensitivity = false)
        : ReqBase(logger, lines, case_sensitivity)
        , have_testdata_(false)
        , user_cert_(0)
    {
        logger_.Info("获取接入地址请求...");
        request_ = RTGetEntryAddr;
    }

    virtual string Name() const
    {
        return "获取接入地址请求";
    }

    virtual ~ReqGetEntryAddr()
    {
        delete user_cert_;
    }

    // psm addr likely: <ip:port> format.
    void SetPSMAddr(string const& addr)
    {
        logger_.Info("获取接入地址请求，设置PSM地址%s", addr.c_str());
        psm_addr_ = addr;
    }

    virtual bool Parse()
    {
        string odclibpfx    = kOdcLibPfx;
        string usercertpfx  = kUserCertDataPfx;
        string testdatapfx  = kTestDataPfx;
        string macpfx       = kMacPfx;

        if (!case_sensitivity()) {
            odclibpfx    = to_lower(kOdcLibPfx);
            usercertpfx  = to_lower(kUserCertDataPfx);
            testdatapfx  = to_lower(kTestDataPfx);
            macpfx       = to_lower(kMacPfx);
        }

        map<string, string>::iterator it;


        // odc lib desc.
        if ((it = lines_.find(odclibpfx)) == lines_.end()) {
            logger_.Warn("获取接入地址，找不到OdcLib=...行，解析失败");
            return false;
        }
        ByteStream odclibbs;
        odclibbs.SetByteOrder(NETWORK_BYTEORDER);
        odclibbs.PutHexString(it->second);
        // need check throw an exception.
        try {
            odclib_desc_ = Descriptor(odclibbs);
        } catch (...) {
            logger_.Warn("获取接入地址，解析OdcLibDescriptor失败");
            return false;
        }



        // usercert desc.
        if ((it = lines_.find(usercertpfx)) == lines_.end()) {
            logger_.Warn("获取接入地址，找不到UserCertData=...行，解析失败");
            return false;
        }
        ByteStream usercertbs;
        usercertbs.SetByteOrder(NETWORK_BYTEORDER);
        usercertbs.PutHexString(it->second);
        // need check throw an exception.
        try {
            usercert_desc_ = Descriptor(usercertbs);
        } catch (...) {
            logger_.Warn("获取接入地址，解析UserCertDataDescriptor失败:\n\t%s", it->second.c_str());
            return false;
        }


        // testdata desc. [optional segment]
        if ((it = lines_.find(testdatapfx)) != lines_.end()) {
            ByteStream testdatabs;
            testdatabs.SetByteOrder(NETWORK_BYTEORDER);
            testdatabs.PutHexString(it->second);
            // need check throw an exception.
            try {
                testdata_desc_ = Descriptor(testdatabs);
            } catch (...) {
                logger_.Warn("获取接入地址，解析可选TestDataDescriptor失败");
                return false;
            }
            have_testdata_ = true;
        }


        // mac desc.
        if ((it = lines_.find(macpfx)) == lines_.end()) {
            logger_.Warn("获取接入地址，找不到Mac=...行，解析失败");
            return false;
        }

        // mac desc.
        ByteStream macbs;
        macbs.SetByteOrder(NETWORK_BYTEORDER);
        macbs.PutHexString(it->second);
        // need check throw an exception.
        try {
            mac_desc_ = Descriptor(macbs);
        } catch (...) {
            logger_.Warn("获取接入地址，解析MacDescriptor失败");
            return false; 
        }

        logger_.Info("获取接入地址，解析成功");

        return true;
    }

    virtual bool Valid()
    {
        // get userinfo from usercert
        bool ok;
        usercert_desc_.user_cert_data_.SetByteOrder(NETWORK_BYTEORDER);
        uint16_t dlen = usercert_desc_.user_cert_data_.GetUint16();
        logger_.Info("获取接入地址，提取用户证书，长度%d", dlen);
        user_cert_ = new UserCert(logger_, usercert_desc_.user_cert_data_, ok);
        if (!ok) {
            logger_.Warn("获取接入地址，提取用户证书失败");
            return false;
        }

        return CertMgr::instance().ValidCert(UserInfo(user_cert_->certui_desc_.user_info_), *user_cert_);
    }

    virtual string MakeResponse(bool ok)
    {
        //ReturnCode=返回码 [0<ok>/1<failed>]
        //EncryptData=被加密数据的十六进制字符串 [ok]
        //PublicEncrypt=PublicEncryptDescriptor [ok]
        //Redirect=RedirectDescriptor [failed optional]
        //TestData=TestDataDescriptor [optional
        //Mac=MacDescriptor [failed]

        string  ret = "";

        string retcode = ok ? "0" : "1";

        ret += "ReturnCode=";
        ret += retcode;
        ret += "\r\n";

        if (ok) {
            // encrypt data : Redirect=RedirectDescriptor 

            ret += "EncryptData=";
            ret += "Redirect=";
            ByteStream bs = PT_RedirectDescriptor("psm://" + psm_addr_).SerializeFull();
            ret += bs.DumpHex(bs.Size());
            ret += "\r\n";


            // public encrypt 
            ret += "PublicEncrypt=";
            uint32_t key_id = 0;
            ByteStream pubencrypt_bs = 
                PublicEncryptDescriptor(key_id, CertMgr::GeneratePublicEncryptCipherData(UserInfo(user_cert_->certui_desc_.user_info_))).SerializeFull();
            ret += pubencrypt_bs.DumpHex(pubencrypt_bs.Size());
            ret += "\r\n";
        }

        //test data [optional segment]
        if (have_testdata_) {
            ret += "TestData=";
            ByteStream testdata_desc_bs = testdata_desc_.SerializeFull();
            ret += testdata_desc_bs.DumpHex(testdata_desc_bs.Size());
            ret += "\r\n";
        }

        if (!ok) {
            ret += "Mac=";
            ByteStream mac_bs = mac_desc_.SerializeFull();
            ret += mac_bs.DumpHex(mac_bs.Size());
            ret += "\r\n";
        }

        return ret;
    }

private:
    string psm_addr_;
};

#endif // !gehua_auth_agent_request_h_