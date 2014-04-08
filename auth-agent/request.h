/*
* @brief: request 
*/

#if !defined gehua_auth_agent_request_h_
#define gehua_auth_agent_request_h_

#include <string>
#include <cpplib/logger.h>
#include <protocol/protocol_v2_pt_descriptor.h>
#include "../common/widget.h"
#include "../common/auth/usercert.h"
#include "../common/auth/certmgr.h"
#include "../common/auth/desc-common.h"
#include "errcode.h"

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

const string kOdcLibPfx   = "OdcLib";
const string kUserInfoPfx = "UserInfo";
const string kTestDataPfx = "TestData";
const string kMacPfx      = "Mac";
const string kUserPublicKeyPfx = "UserPublicKey";
const string kChallengeCodePfx = "ChallengeCode";
const string kUserCertDataPfx  = "UserCertData";

struct ReqBase
{
    ReqBase(Logger &logger, vector<string> const& lines, bool case_sensitivity)
        : logger_(logger), case_sensitivity_(case_sensitivity)
    {
        // content likely "OdcLibDesc=..."
        // change to map ---> map["odclibdesc"] = ... format.
        string::size_type pos;
        logger_.Trace("Start Parse Raw Lines-------------------------------");
        for (size_t i = 0; i < lines.size(); ++i) {
            logger_.Trace("%d line: %s", i + 1, lines[i].c_str());
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
                logger_.Trace("\tParse Result[%s] = %s", key.c_str(), suffix.c_str());
            }

            all_lines_ += lines[i];
        }
        logger_.Trace("Parse Raw Line Over-------------------------------");
    }

    virtual ~ReqBase() {}

    virtual bool Parse() = 0;

    virtual string MakeResponse(bool ok) = 0;

    virtual bool Valid() = 0;

    virtual string Name() const = 0;

    ReqType Type() const { return request_; }

    bool case_sensitivity() const { return case_sensitivity_; }

    string getlines() const { return all_lines_; }

    string getvalue(string const& key) const
    {
        map<string, string>::const_iterator cit;
        if ((cit = lines_.find(key)) != lines_.end())
            return cit->second;

        return "<null>";
    }

    uint64_t errorcode() const { return error_code_; }

protected:
    uint64_t parse_odclibdesc(PT_OdcLibDescriptor &odclib_desc)
    {
        string odclibpfx = kOdcLibPfx;
        if (!case_sensitivity()) odclibpfx = to_lower(kOdcLibPfx);

        map<string, string>::iterator it;
        // odc lib desc.
        if ((it = lines_.find(odclibpfx)) == lines_.end()) {
            logger_.Warn("%s, can't find <OdcLib> line, parse failure, alllines: \n%s",
                        Name().c_str(), 
                        getlines().c_str());
            return EC_OdcLibLineNotExisted;
        }

        ByteStream odclibbs;
        odclibbs.SetByteOrder(NETWORK_BYTEORDER);
        odclibbs.PutHexString(it->second);
        // need check throw an exception.
        try {
            odclib_desc = Descriptor(odclibbs);
        } catch (...) {
            logger_.Warn("%s, parse <OdcLibDescriptor> failure, <value> is: \n%s",
                        Name().c_str(),
                        getvalue(odclibpfx).c_str());

            return EC_OdcLibDescParseFailed;
        }
        return 0;
    }

    uint64_t parse_userinfodesc(PT_UserInfoDescriptor &userinfo_desc)
    {
        string userinfopfx  = kUserInfoPfx;
        if (!case_sensitivity()) userinfopfx  = to_lower(kUserInfoPfx);
        
        map<string, string>::iterator it;
        // userinfo desc.
        if ((it = lines_.find(userinfopfx)) == lines_.end()) {
            logger_.Warn("%s, can't find <UserInfo> line, parse failure, alllines: \n%s",
                        Name().c_str(), 
                        getlines().c_str());
            return EC_UserInfoLineNotExisted;
        }
        ByteStream userinfobs;
        userinfobs.SetByteOrder(NETWORK_BYTEORDER);
        userinfobs.PutHexString(it->second);
        // need check throw an exception.
        try {
            userinfo_desc = Descriptor(userinfobs);
        } catch (...) {
            logger_.Warn("%s, parse <UserInfoDescriptor> failure, <value> is: \n%s",
                        Name().c_str(),
                        getvalue(userinfopfx).c_str());

            return EC_UserInfoDescParseFailed;
        }
        return 0;
    }

    uint64_t parse_testdatadesc(PT_TestDataDescriptor &testdata_desc, bool *have_testdata)
    {
        string testdatapfx = kTestDataPfx;
        if (!case_sensitivity()) testdatapfx = to_lower(kTestDataPfx);

        map<string, string>::iterator it;
        *have_testdata = false;
        // testdata desc. [optional segment]
        if ((it = lines_.find(testdatapfx)) != lines_.end()) {
            *have_testdata = true;

            ByteStream testdatabs;
            testdatabs.SetByteOrder(NETWORK_BYTEORDER);
            testdatabs.PutHexString(it->second);
            // need check throw an exception.
            try {
                testdata_desc = Descriptor(testdatabs);
            } catch (...) {
                logger_.Warn("%s, parse optional <TestDataDescriptor> failure, <value> is: \n%s",
                        Name().c_str(),
                        getvalue(testdatapfx).c_str());

                return EC_TestDataDescParseFailed;
            }      
        }

        return 0;
    }

    uint64_t parse_macdesc(MacDescriptor &mac_desc)
    {
        string macpfx = kMacPfx;
        if (!case_sensitivity()) macpfx = to_lower(kMacPfx);

        map<string, string>::iterator it;
        if ((it = lines_.find(macpfx)) == lines_.end()) {
            logger_.Warn("%s, can't find <Mac> line, parse failure, alllines: \n%s",
                        Name().c_str(), 
                        getlines().c_str());
            return EC_MacLineNotExisted;
        }

        ByteStream macbs;
        macbs.SetByteOrder(NETWORK_BYTEORDER);
        macbs.PutHexString(it->second);
        // need check throw an exception.
        try {
            mac_desc = Descriptor(macbs);
        } catch (...) {
            logger_.Warn("%s, parse <MacDescriptor> failure, <value> is: \n%s",
                        Name().c_str(),
                        getvalue(macpfx).c_str());

            return EC_MacDescParseFailed;
        }
        return 0;
    }

protected:
    uint64_t valid_userinfo(string const& userinfostr, UserInfo **userinfo)
    {
        UserInfo *ui = new UserInfo(userinfostr);

        if (!ui->valid()) {
            logger_.Warn("%s, user info string valid failure, user info string: \n%s",
                        Name().c_str(), 
                        userinfostr.c_str());
            delete ui;
            return EC_UserInfoValidFailed;
        }
        *userinfo = ui;
        return 0;
    }

protected:
    Logger &logger_;
    ReqType request_;
    map<string, string> lines_;
    string all_lines_;
    uint64_t error_code_;
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
        logger_.Info("[Challenge Request]......");
        request_ = RTChallenge;   
    }

    virtual ~ReqChallenge() 
    {
        delete userinfo_;
    }

    virtual string Name() const
    {
        return "[Challenge Request]";
    }

    virtual bool Parse()
    {   
        do {
            if ((error_code_ = parse_odclibdesc(odclib_desc_)) != 0) break;
            if ((error_code_ = parse_userinfodesc(userinfo_desc_)) != 0) break;
            if ((error_code_ = parse_testdatadesc(testdata_desc_, &have_testdata_)) != 0) break;
            if ((error_code_ = parse_macdesc(mac_desc_)) != 0) break;
        } while (0);
        
        return error_code_ == 0;
    }

    virtual bool Valid() 
    {
        if ((error_code_ = valid_userinfo(userinfo_desc_.user_info_, &userinfo_)) != 0)
            return false;
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
        //return code. default is 0->successful.
        ret += "ReturnCode="; 
        ret += to_hex_string(errorcode());
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
        logger_.Info("[Get Cert Request]......");
        request_ = RTGetCert;
    }

    virtual string Name() const
    {
        return "[Get Cert Request]";
    }

    virtual bool Parse()
    {
        do {
            if ((error_code_ = parse_odclibdesc(odclib_desc_)) != 0) break;
            if ((error_code_ = parse_userinfodesc(userinfo_desc_)) != 0) break;
            if ((error_code_ = parse_userpublickeydesc(userpubkey_desc_)) != 0) break;
            if ((error_code_ = parse_testdatadesc(testdata_desc_, &have_testdata_)) != 0) break;
            if ((error_code_ = parse_challengecodedesc(challenge_desc_, &have_challenge_)) != 0) break;
            if ((error_code_ = parse_macormacpwddesc(mac_desc_, macpasswd_desc_, &is_mac_desc_)) != 0) break;
        } while (0);
        
        return error_code_ == 0;
    }

    virtual bool Valid()
    {
        if ((error_code_ = valid_userinfo(userinfo_desc_.user_info_, &userinfo_)) != 0)
            return false;

        vector<uint8_t> ccode = CertMgr::instance().GetChallengeCode(*userinfo_);
        if (ccode.size() == 0) {
            if (have_challenge_) {
                logger_.Warn("%s, valid <ChallengeCode> failure, can't find challenge code in local database, but terminal had changed.",
                        Name().c_str());
                error_code_ = EC_ValidLocalChallengeCodeNotExisted;
                return false;
            }
            return true;
        }

        if (!have_challenge_) {
            logger_.Warn("%s, valid <ChallengeCode> failure, can't find challenge code line, but terminal had changed.",
                        Name().c_str());
            error_code_ = EC_ValidChallengeCodeLineNotExisted;
            return false;
        }

        ByteStream ccodebs = challenge_desc_.challenge_code_;
        if (ccodebs.Size() == 0) {
            logger_.Warn("%s, valid <ChallengeCode> failure, challenge code size is empty, but terminal had changed.",
                        Name().c_str());
            error_code_ = EC_ValidChallengeCodeIsEmpty;
            return false;
        }

        vector<uint8_t> reqccode;
        reqccode.resize(ccodebs.Size());

        if (ccode.size() != reqccode.size()) {
            logger_.Warn("%s, valid <ChallengeCode> failure, challenge code size is not equal as local challenge code, but terminal had changed.",
                        Name().c_str());
            error_code_ = EC_ValidChallengeCodeSizeNotMatched;
            return false;
        }

        ccodebs.Get(&reqccode[0], ccodebs.Size());  

        if (!memcmp(&ccode[0], &reqccode[0], reqccode.size())) {
            logger_.Warn("%s, valid <ChallengeCode> failure, challenge code is not equal as local challenge code, but terminal had changed.",
                        Name().c_str());
            error_code_ = EC_ValidChallengeCodeNotMatched; 
            return false;
        }

        error_code_ = 0;
        return true;
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
        ret += to_hex_string(errorcode());
        ret += "\r\n";


        if (ok) {
            UserCert *usercert = 
                CertMgr::instance().GenerateCert(logger_, *userinfo_, userinfo_desc_.user_info_);

            ByteStream certdata;
            certdata.SetByteOrder(NETWORK_BYTEORDER);
            ByteStream usercert_bs = usercert->getStream();
	        certdata.PutByteStream16(usercert_bs);
            PT_UserCertDataDescriptor certdata_desc(certdata);
            ByteStream rootkeydata_bs = CertMgr::GenerateRootKey(*userinfo_);
	        PT_RootKeyDescriptor rootkey_desc(rootkeydata_bs);

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
            ByteStream cipherdata_bs = CertMgr::GeneratePublicEncryptCipherData(*userinfo_);
            ByteStream pubencrypt_bs = PublicEncryptDescriptor(key_id, cipherdata_bs).SerializeFull();
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
    uint64_t parse_userpublickeydesc(PT_UserPublicKeyDescriptor &userpubkey_desc)
    {
        string pubkeypfx = kUserPublicKeyPfx;
        if (!case_sensitivity()) pubkeypfx = to_lower(kUserPublicKeyPfx);
        
        map<string, string>::iterator it;
        // public key desc.
        if ((it = lines_.find(pubkeypfx)) == lines_.end()) {
            logger_.Warn("%s, can't find <PublicKey> line, parse failure, alllines: \n%s",
                        Name().c_str(), 
                        getlines().c_str());
            return EC_PublicKeyLineNotExisted;
        }

        ByteStream pubkeybs;
        pubkeybs.SetByteOrder(NETWORK_BYTEORDER);
        pubkeybs.PutHexString(it->second);
        // need check throw an exception.
        try {
            userpubkey_desc = Descriptor(pubkeybs);
        } catch (...) {
            logger_.Warn("%s, parse <UserPublicKeyDescriptor> failure, <value> is: \n%s",
                        Name().c_str(),
                        getvalue(pubkeypfx).c_str());

            return EC_PublicKeyDescParseFailed;
        }
        return 0;
    }

    uint64_t parse_challengecodedesc(PT_ChallengeCodeDescriptor &challenge_desc, bool *have_challenge)
    {
        string challengepfx = kChallengeCodePfx;
        if (!case_sensitivity()) challengepfx = to_lower(kChallengeCodePfx);

        map<string, string>::iterator it;
        *have_challenge = false;
        // challenge code desc.
        if ((it = lines_.find(challengepfx)) != lines_.end()) {
            *have_challenge = true;

            ByteStream challengebs;
            challengebs.SetByteOrder(NETWORK_BYTEORDER);
            challengebs.PutHexString(it->second);
            // need check throw an exception.
            try {
                challenge_desc_ = Descriptor(challengebs);
            } catch (...) {
                logger_.Warn("%s, parse <ChallengeCodeDescriptor> failure, <value> is: \n%s",
                        Name().c_str(),
                        getvalue(challengepfx).c_str());

                return EC_ChallengeCodeDescParseFailed;
            }   
        }
        return 0;
    }

    uint64_t parse_macormacpwddesc(
        MacDescriptor &mac_desc, 
        MacPasswordDescriptor &macpasswd_desc, 
        bool *is_mac_desc)
    {
        string macpfx = kMacPfx;
        if (!case_sensitivity()) macpfx = to_lower(kMacPfx);

        map<string, string>::iterator it;
        // mac or macpassword desc.
        if ((it = lines_.find(macpfx)) == lines_.end()) {
            logger_.Warn("%s, can't find <Mac> line, parse failure, alllines: \n%s",
                        Name().c_str(), 
                        getlines().c_str());
            return EC_MacLineNotExisted;
        }

        ByteStream macbs;
        macbs.SetByteOrder(NETWORK_BYTEORDER);
        macbs.PutHexString(it->second);
        // need check throw an exception.
        Descriptor macdesc;
        try {
            macdesc = Descriptor(macbs);
        } catch (...) {
            logger_.Warn("%s, parse <[Mac/MacPassword]Descriptor> failure, <value> is: \n%s",
                        Name().c_str(),
                        getvalue(macpfx).c_str());

            return EC_MacOrMacPwdDescParseFailed;
        }

        switch (mac_desc.tag_) {
        case TAG_MacDescriptor:
            mac_desc = macdesc;
            *is_mac_desc = true;
            break;
        case TAG_MacPasswordDescriptor:
            macpasswd_desc = macdesc;
            *is_mac_desc = false;
            break;
        default:
            logger_.Warn("%s, parse <[Mac/MacPassword]Descriptor> failure, <value> is: \n%s",
                        Name().c_str(),
                        getvalue(macpfx).c_str());

            return EC_MacOrMacPwdDescParseMsgIdFailed;
        }
        return 0;
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
        logger_.Info("[Get Entry Addr Request]......");
        request_ = RTGetEntryAddr;
    }

    virtual string Name() const
    {
        return "[Get Entry Addr Request]";
    }

    virtual ~ReqGetEntryAddr()
    {
        delete user_cert_;
    }

    // psm addr likely: <ip:port> format.
    void SetPSMAddr(string const& addr)
    {
        logger_.Info("%s, set PSM address: %s", Name().c_str(), addr.c_str());
        psm_addr_ = addr;
    }

    virtual bool Parse()
    {
        do {
            if ((error_code_ = parse_odclibdesc(odclib_desc_)) != 0) break;
            if ((error_code_ = parse_usercertdesc(usercert_desc_)) != 0) break;
            if ((error_code_ = parse_macdesc(mac_desc_)) != 0) break;
            if ((error_code_ = parse_testdatadesc(testdata_desc_, &have_testdata_)) != 0) break;
        } while (0);
        return error_code_ == 0;
    }

    virtual bool Valid()
    {
        /*if ((error_code_ = valid_userinfo(userinfo_desc_.user_info_, &userinfo_)) != 0)
            return false;*/

        // get userinfo from usercert
        bool ok;
        usercert_desc_.user_cert_data_.SetByteOrder(NETWORK_BYTEORDER);
        uint16_t dlen = usercert_desc_.user_cert_data_.GetUint16();
        logger_.Info("%s, extra user cert, user cert length: %d", Name().c_str(), dlen);
        user_cert_ = new UserCert(logger_, usercert_desc_.user_cert_data_, ok);
        if (!ok) {
            logger_.Warn("%s, extra user cert failure");
            error_code_ = EC_ExtraUserCertFailed;
            return false;
        }

        if (!CertMgr::instance().ValidCert(
                UserInfo(user_cert_->certui_desc_.user_info_), 
                *user_cert_)) {
                    logger_.Warn("%s, user cert valid failure", Name().c_str());
                    error_code_ = EC_ValidUserCertFailed;
                    return false;
        }
        error_code_ = 0;
        return true;
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
        ret += to_hex_string(errorcode());
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
            ByteStream cipherdata_bs = CertMgr::GeneratePublicEncryptCipherData(UserInfo(user_cert_->certui_desc_.user_info_));
            ByteStream pubencrypt_bs = 
                PublicEncryptDescriptor(key_id, cipherdata_bs).SerializeFull();
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
    uint64_t parse_usercertdesc(PT_UserCertDataDescriptor &usercert_desc)
    {
        string usercertpfx = kUserCertDataPfx;
        if (!case_sensitivity()) usercertpfx = to_lower(kUserCertDataPfx);
        
        map<string, string>::iterator it;
        if ((it = lines_.find(usercertpfx)) == lines_.end()) {
            logger_.Warn("%s, can't find <UserCertData> line, parse failure, alllines: \n%s",
                        Name().c_str(), 
                        getlines().c_str());
            return EC_UserCertDataLineNotExisted;
        }
        ByteStream usercertbs;
        usercertbs.SetByteOrder(NETWORK_BYTEORDER);
        usercertbs.PutHexString(it->second);
        // need check throw an exception.
        try {
            usercert_desc = Descriptor(usercertbs);
        } catch (...) {
            logger_.Warn("%s, parse <UserCertDataDescriptor> failure, <value> is: \n%s",
                        Name().c_str(),
                        getvalue(usercertpfx).c_str());

            return EC_UserCertDataDescParseFailed;
        }
        return 0;
    }
    string psm_addr_;
};

#endif // !gehua_auth_agent_request_h_
