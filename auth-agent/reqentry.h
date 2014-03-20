
/*
 * @brief: request entry:
 */

#if !defined gehua_auth_agent_reqest_entry_h_
#define gehua_auth_agent_reqest_entry_h_

#include <string>
#include <cpplib/logger.h>
#include "../common/widget.h"
#include "request.h"

using ::std::string;

const string kQueryStringPfx = "QUERY_STRING=";
const string kUserIdStr = "userid";
const string kReqStr = "req";

struct RequestEntry 
{
    string ca_id;

    RequestEntry(Logger &logger, string const& query_str, vector<string> const& content)
        : logger_(logger), valid_(false), request_(0)
    {
        //query string like "key1=value1&key2=value2...
        map<string, string> kv_pair;
        split(query_str, "&", "=", &kv_pair);

        string userid, req;
        
        map<string, string>::iterator it;

        // find userid.
        if ((it = kv_pair.find(kUserIdStr)) == kv_pair.end()) {
            logger_.Warn("查询字符串userid=xxx&req=xxx 中无法找到userid字符");
            return;
        }

        userid = it->second;

        // find req
        if ((it = kv_pair.find(kReqStr)) == kv_pair.end()) {
            logger_.Warn("查询字符串userid=xxx&req=xxx 中无法找到req字符");
            return;
        }

        req = it->second;
        logger_.Info("用户%s发来%s请求", userid.c_str(), req.c_str());

        switch (requestType(req)) {
        case RTGetSTBQRCode:
            request_ = new ReqGetSTBQRCode(logger_, content);
            break;
        case RTChallenge: 
            request_ = new ReqChallenge(logger_, content);
            break;
        case RTGetCert:
            request_ = new ReqGetCert(logger_, content);
            break;
        case RTGetEntryAddr:
            request_ = new ReqGetEntryAddr(logger_, content);
            break;
        case RTUnknown:
            return;
        }

        if (!request_->Parse()) {
            delete request_;
            request_ = 0;
            return;
        }

        ca_id = userid;
        valid_ = true;
    }

    ReqType requestType(string const& req) const
    {
        int ret = 0xFF;
        for (int i = 0; i < kReqStrArraySize; ++i) {
            if (req == kReqStrArray[i]) {
                ret = i + 1;
                break;
            }
        }

        return (ReqType)ret;
    }

    string MakeResponse()
    {
        if (request_ == NULL)
            return "";

        return request_->MakeResponse(request_->Valid());
    }

    bool valid() const { return valid_; }

    bool ConnKeepAlive() const 
    {
        if (request_ == 0) return false;
        switch (request_->Type()) {
        case RTGetSTBQRCode:   
        case RTGetEntryAddr:
            return false;
        case RTChallenge:
        case RTGetCert:
            return true;
        }

        return false;
    }
private:
    bool valid_;
    ReqBase *request_;
    Logger &logger_;
};

#endif // gehua_auth_agent_reqest_entry_h_