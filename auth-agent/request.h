/*
 * @brief: request 
 */

#if !defined gehua_auth_agent_request_h_
#define gehua_auth_agent_request_h_

#include <string>
#include "../common/widget.h"
#include "../common/auth/certmgr.h"
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
    ReqBase(vector<string> const& lines, bool case_sensitivity)
        : case_sensitivity_(case_sensitivity)
    {
        // content likely "OdcLibDesc=..."
        // change to map ---> map["odclibdesc"] = ... format.
        string::size_type pos;
        for (size_t i = 0; i < lines.size(); ++i) {
            if ((pos = lines[i].find_first_of("=")) != string::npos) {
                vector<uint8_t> value;
                to_array(lines[i].substr(pos + 1), &value);
                string key = lines[i].substr(0, pos);
                if (!case_sensitivity_)
                    key = to_lower(key);
                lines_[key] = value;
            }
        }
    }

    virtual ~ReqBase() {}

    virtual bool Parse() = 0;

    virtual string MakeResponse() = 0;

    ReqType Type() const { return request_; }

    bool case_sensitivity() const { return case_sensitivity_; }

protected:
    ReqType request_;
    map<string, vector<uint8_t> > lines_;
private:
    bool case_sensitivity_;
};

struct ReqGetSTBQRCode : public ReqBase
{
    ReqGetSTBQRCode(vector<string> const lines, bool case_sensitivity = false)
        : ReqBase(lines, case_sensitivity)
    {
        request_ = RTGetSTBQRCode;
    }

    virtual bool Parse()
    {
        return false;
    }

    virtual string MakeResponse()
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
    ReqChallenge(vector<string> const& lines, bool case_sensitivity = false)
        : ReqBase(lines, case_sensitivity)
    {
        request_ = RTChallenge;   
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

        map<string, vector<uint8_t> >::iterator it;
        
        // odc lib desc.
        if ((it = lines_.find(odclibpfx)) == lines_.end())
            return false;


        
        return true;
    }

    virtual string MakeResponse()
    {
        string ret = "ReturnCode=0\n"
                     "ChallengeCode=%s\n"
                     //"Redirect=RedirectDescriptor\n"
                     "Mac=%s";



        return ret;
    }
};

struct ReqGetCert : public ReqBase
{
    ReqGetCert(vector<string> const& lines, bool case_sensitivity = false)
        : ReqBase(lines, case_sensitivity)
    {
        request_ = RTGetCert;
    }

    virtual bool Parse()
    {
        return false;
    }

    virtual string MakeResponse()
    {
        return "";
    }
};

struct ReqGetEntryAddr : public ReqBase
{
    ReqGetEntryAddr(vector<string> const& lines, bool case_sensitivity = false)
        : ReqBase(lines, case_sensitivity)
    {
        request_ = RTGetEntryAddr;
    }

    virtual bool Parse()
    {
        return false;
    }

    virtual string MakeResponse()
    {
        return "";
    }
};

#endif // !gehua_auth_agent_request_h_