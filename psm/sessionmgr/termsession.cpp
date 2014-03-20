

#include "termsession.h"

TermSession::TermSession(Logger &logger, PtLoginRequest *msg, TermConnection *tconn)
    : user_info(msg->user_info_desc_.user_info_)
    , odclib_desc(msg->odclib_desc_)
    , user_info_desc(msg->user_info_desc_)
    , cert_data_desc(msg->cert_data_desc_)
    , terminal_info_desc(msg->terminal_info_desc_)
    , term_conn(tconn), ca_session(0), valid_(false)
    , logger_(logger)
{
    // 描述符合法性验证
    if ( !odclib_desc.valid_ || !user_info_desc.valid_ || !cert_data_desc.valid_ || !terminal_info_desc.valid_ )
    {
        logger_.Error("验证用户登录请求失败，描述符不完整。");
        return;
    }

    if (!user_info.valid()) {
        logger_.Error("PSM Business Pool new TermSession userinfo valid failure");
        return;
    }

    logger_.Trace("PSM Business Pool new TermSession userinfo:"
        "\n\tuser id: %s, \n\tcaid: %s, \n\tterminal id: %s, \n\tmac addr: %s", 
        user_info.id.c_str(), user_info.card_id.c_str(), 
        user_info.term_id.c_str(), user_info.mac_addr.c_str());

    //get ca card id.
    bool ok;
    caid_t caid = to_uint64(user_info.card_id, &ok);
    if (!ok) {
        logger_.Error(
            "PSM Business Pool new TermSession convert string(%s) to uint64_t(caid) failure", 
            user_info.card_id.c_str());
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

    terminal_info_desc.business_url_ = url_desc.url_;
}

void TermSession::UpdateBackURL( PB_SvcURLDescriptor &url_desc )
{
    unsigned int stack_size = back_url_stack.size();

    // 如果返回的BackURL为空，则需要设置为上一个业务的URL
    if ( url_desc.back_url_.empty() ) 
    {
        url_desc.back_url_ = GetBackURL(url_desc.url_);
    }

    back_url_stack.push(url_desc.url_);
}

string TermSession::GetBackURL( string &curr_apply_url )
{
    const char  *tmp_str1 = curr_apply_url.c_str();
    const char  *tmp_str2 = strchr(tmp_str1, ':');
    if ( tmp_str2 == NULL )
    {
        return "";
    }

    unsigned int len = tmp_str2 - tmp_str1 + 1;

    do 
    {
        unsigned int stack_size = back_url_stack.size();
        if ( stack_size > 0 )
        {
            string tmp_url = back_url_stack.top();

            if ( tmp_url.size() > len )
            {
                if ( strncmp(tmp_url.c_str(), tmp_str1, len) == 0 )
                {
                    back_url_stack.pop();
                    continue;
                }
                else
                {
                    return tmp_url;
                }
            }
        }
    } while (1);    

    return "";
}
