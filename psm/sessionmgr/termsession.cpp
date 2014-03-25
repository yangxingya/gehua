

#include "termsession.h"

TermSession::TermSession(Logger &logger, PtLoginRequest *msg, TermConnection *tconn)
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
    if ( !odclib_desc_.valid_ || !user_info_desc_.valid_ || !cert_data_desc_.valid_ || !terminal_info_desc_.valid_ )
    {
        logger_.Error("验证用户登录请求失败，描述符不完整。");
        return;
    }

    // 判断TerminalInfoDescriptor内容的正确性
    if ( !IsValidTermInfoDesc(terminal_info_desc_) )
    {
        logger_.Error("验证用户登录请求失败，TerminalInfoDescriptor内容无效。");
        return;
    }

    if (!user_info_.valid()) {
        logger_.Error("PSM Business Pool new TermSession userinfo valid failure");
        return;
    }

    logger_.Trace("PSM Business Pool new TermSession userinfo:"
        "\n\tuser id: %s, \n\tcaid: %s, \n\tterminal id: %s, \n\tmac addr: %s", 
        user_info_.id.c_str(), user_info_.card_id.c_str(), 
        user_info_.term_id.c_str(), user_info_.mac_addr.c_str());

    //get ca card id.
    bool ok;
    caid_t caid = to_uint64(user_info_.card_id, &ok);
    if (!ok) {
        logger_.Error(
            "PSM Business Pool new TermSession convert string(%s) to uint64_t(caid) failure", 
            user_info_.card_id.c_str());
        return;
    }

    caid_ = caid;
    
    //cert.

    valid_ = true;
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
