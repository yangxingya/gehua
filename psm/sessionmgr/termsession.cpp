
#include "termsession.h"
#include <protocol/protocol_v2_pt_message.h>

TermSession::TermSession(Logger &logger, PtLoginRequest *msg, TermConnection *tconn, LoginError *error)
    : ca_session_(0), term_conn_(tconn)
    , user_info_(msg->user_info_desc_.user_info_)
    , odclib_desc_(msg->odclib_desc_)
    , user_info_desc_(msg->user_info_desc_)
    , cert_data_desc_(msg->cert_data_desc_)
    , terminal_info_desc_(msg->terminal_info_desc_)
    , id_(0),caid_(0),valid_(false)
    , logger_(logger)
{
    // 描述符合法性验证
    if (!odclib_desc_.valid_) {
        *error = LoginOdcLibDescIncompleted;
        logger_.Warn("[Terminal Login Valid] OdcLib Descriptor incompleted");
        return;
    }
    if (!user_info_desc_.valid_) {
        *error = LoginUserInfoDescIncompleted;
        logger_.Warn("[Terminal Login Valid] UserInfo Descriptor incompleted");
        return;
    }
    if (!cert_data_desc_.valid_) {
        *error = LoginCertDataDescIncompleted;
        logger_.Warn("[Terminal Login Valid] CertData Descriptor incompleted");
        return;
    }
    if (!terminal_info_desc_.valid_) {
        *error = LoginTerminalInfoDescIncompleted;
        logger_.Warn("[Terminal Login Valid] Terminal Info Descriptor incompleted");
        return;
    }

    // 判断TerminalInfoDescriptor内容的正确性
    if (!IsValidTermInfoDesc(terminal_info_desc_)) {
        *error = LoginTerminalInfoInvalided;
        logger_.Warn("[Terminal Login Valid] TerminalInfo Descriptor Context Invalided, terminalinfo:"
                "\n\tclass: %s, \n\tsubclass: %s, \n\tmodel: %s, \n\tname: %s, \n\tcurr url: %s", 
                kTerminalClassName[terminal_info_desc_.terminal_class_ - 1].c_str(),
                kSubClassName[terminal_info_desc_.terminal_class_ - 1][terminal_info_desc_.terminal_sub_class_ - 1].c_str(),
                terminal_info_desc_.terminal_model_.c_str(),
                terminal_info_desc_.terminal_name_.c_str(),
                terminal_info_desc_.business_url_.c_str());
        return;
    }

    if (!user_info_.valid()) {
        *error = LoginUserInfoInvalided;
        logger_.Error("[Terminal Login Valid] UserInfo Invalided, userinfo: \n\t%s", msg->user_info_desc_.user_info_.c_str());
        return;
    }
    
    

    //get ca card id.
    bool cast_ok;
    caid_t caid = to_uint64(user_info_.card_id, &cast_ok);
    if (!cast_ok) {
        *error = LoginCastCAIdFailed;
        logger_.Error("[Terminal Login Valid] new TermSession convert string(%s) to uint64_t(caid) failure", 
                user_info_.card_id.c_str());
        return;
    }

    caid_ = caid;
    
    //cert.

    valid_ = true;
    *error = LoginOK;

    logger_.Trace("[Terminal Login Valid] new TermSession userinfo:"
        "\n\tuser id: %s, \n\tcaid: %s, \n\tterminal id: %s, \n\tmac addr: %s", 
        user_info_.id.c_str(), user_info_.card_id.c_str(), 
        user_info_.term_id.c_str(), user_info_.mac_addr.c_str());

    logger_.Trace("Terminal Login Valid] new TermSession terminalinfo:"
        "\n\tclass: %s, \n\tsubclass: %s, \n\tmodel: %s, \n\tname: %s, \n\tcurr url: %s", 
        kTerminalClassName[terminal_info_desc_.terminal_class_ - 1].c_str(),
        kSubClassName[terminal_info_desc_.terminal_class_ - 1][terminal_info_desc_.terminal_sub_class_ - 1].c_str(),
        terminal_info_desc_.terminal_model_.c_str(),
        terminal_info_desc_.terminal_name_.c_str(),
        terminal_info_desc_.business_url_.c_str());
}

void TermSession::UpdateSessionInfo(PB_SvcURLDescriptor &url_desc)
{
    //更新会话URL信息
    url_desc = url_desc;

    terminal_info_desc_.business_url_ = url_desc.url_;
}

bool TermSession::IsValidTermInfoDesc( PT_TerminalInfoDescriptor term_info_desc )
{
    // 判断终端类型的正确性
    switch ( term_info_desc.terminal_class_ )
    {
    case TerminalSTB:
    {
        if ( term_info_desc.terminal_sub_class_ != STBOneWay && term_info_desc.terminal_sub_class_ != STBTwoWayHD && term_info_desc.terminal_sub_class_ != STBTwoWaySD )
        {
            return false;
        }
        break;
    }
    case TerminalPhone:
    {
        if ( term_info_desc.terminal_sub_class_ != PhoneAndriod && term_info_desc.terminal_sub_class_ != PhoneIPhone && term_info_desc.terminal_sub_class_ != PhoneWPhone )
        {
            return false;
        }
        break;
    }
    case TerminalPC:
    {
        if ( term_info_desc.terminal_sub_class_ != OSWindows && term_info_desc.terminal_sub_class_ != OSLinux && term_info_desc.terminal_sub_class_ != OSMac )
        {
            return false;
        }
        break;
    }
    default:
        return false;
    }

    return true;
}
