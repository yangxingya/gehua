

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
}
