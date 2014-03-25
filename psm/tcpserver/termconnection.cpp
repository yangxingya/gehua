#include "../psmcontext.h"
#include "termconnection.h"
#include <protocol/protocol_v2_common.h>
#include <protocol/protocol_v2_general.h>
#include <protocol/protocol_v2_cipher.h>
#include <protocol/protocol_v2_pt_common.h>
#include <protocol/protocol_v2_pt_descriptor.h>
#include <protocol/protocol_v2_pt.h>
#include <protocol/protocol_v2_pt_message.h>

void TermConnection::OnDataReceived()
{
    while (true) {
        PtBase* msg = NULL;

        int      pkt_count = 0;
        uint32_t len       = 0;

        LOCK(lock, recv_mt_);

        if ( recv_buffer_.Size() > 0 )
        {
            recv_buffer_.SetReadPtr(0);
            LOG_TRACE("[%s:TCP] ���յ��ն����ݰ���%s, dump:\n%s", IdString().c_str(), GetPeerAddr().c_str(), recv_buffer_.DumpHex(16,true).c_str());
        }

        pkt_count = parsePacket(&msg, &len);
        if (pkt_count < 0) {
            // it is a invalid packet.
            LOG_FATAL("[%s:TCP] ���ݰ�����ʧ�ܣ�%s, dump:\n%s", IdString().c_str(), GetPeerAddr().c_str(), recv_buffer_.DumpHex(16,true).c_str());
            SetDirty();
            break;
        }
        if ( pkt_count == 0 ) {
            // it is a incompleted packet.
            break;
        }

        // parse successful 
        ByteStream one_packet(recv_buffer_.GetBuffer(),len);            

        recv_buffer_.SetReadPtr(len);
        recv_buffer_.FlushReadPtr();

        //login valid.
        if (!login_) {
            if (msg->msg_id_ != CLOUDv2_PT_TERM_LOGIN_REQUEST) {
                // set session invalid
                LOG_WARN("[%s:TCP] �ն����Ӻ��һ������ǵ�¼����%s, XXXXX:\n%s", IdString().c_str(), GetPeerAddr().c_str(), recv_buffer_.DumpHex(16,true).c_str());
                SetDirty();
                break;  
            }

            term_session_ = psm_ctx_->busi_pool_->GenTermSession((PtLoginRequest *)msg, this);
            if (!term_session_) {
                LOG_WARN("[%s:TCP] �ն����Ӻ��һ��������֤ʧ�ܣ�%s", 
                         IdString().c_str(), 
                         GetPeerAddr().c_str());
                SetDirty();
                break;
            }
            login_ = true;
        }

        switch ( msg->msg_id_ )
        {
        case CLOUDv2_PT_TERM_LOGIN_REQUEST:      
            {
                PtLoginRequest *pkt = (PtLoginRequest*)msg;
                psm_ctx_->term_request_process_svr_->AddLoginRequestWork(this, pkt);
            }

            break;
        case CLOUDv2_PT_TERM_LOGOUT_REQUEST:                
            {
                PtLogoutRequest *pkt = (PtLogoutRequest*)msg;
                psm_ctx_->term_request_process_svr_->AddLogoutRequestWork(this, pkt);
            }

            break;
        case CLOUDv2_PT_TERM_HEARTBEAT_REQUEST:  
            {
                PtHeartbeatRequest *pkt = (PtHeartbeatRequest*)msg;
                psm_ctx_->term_request_process_svr_->AddHeartbeatWork(this, pkt);
            }

            break;
        case CLOUDv2_PT_TERM_SVCAPPLY_REQUEST:
            {
                PtSvcApplyRequest *pkt = (PtSvcApplyRequest *)msg;
                psm_ctx_->term_request_process_svr_->AddSvcApplyWork(this, pkt);
            }

            break;
        case CLOUDv2_PT_TERM_STATUSQUERY_REQUEST:
            {
                PtStatusQueryRequest *pkt = (PtStatusQueryRequest *)msg;
                psm_ctx_->term_request_process_svr_->AddStatusQueryWork(this, pkt);
            }

            break;
        case CLOUDv2_PT_TERM_KEYMAPPING_REQUEST:
            {
                PtKeyMappingRequest *pkt = (PtKeyMappingRequest *)msg;
                psm_ctx_->term_basic_func_svr_->AddKeyTransmitWork(this, pkt);
            }

            break;
        case CLOUDv2_PT_TERM_GETSVCGROUP_REQUEST:
            {
                PtGetSvcGroupRequest *pkt = (PtGetSvcGroupRequest *)msg;
                psm_ctx_->term_request_process_svr_->AddGetSvrGroupWork(this, pkt);
            }
            break;

        case CLOUDv2_PT_TERM_SVCSWITCH_RESPONSE:
            {
                PtSvcSwitchResponse *pkt = (PtSvcSwitchResponse *)msg;
                psm_ctx_->term_basic_func_svr_->AddSvcSwitchNotifyWork(this, pkt);
            }

            break;
        case CLOUDv2_PT_TERM_STATUSNOTIFY_RESPONSE:
            {
                PtStatusNotifyResponse *pkt = (PtStatusNotifyResponse *)msg;
                psm_ctx_->term_basic_func_svr_->AddStatusPChangeNotifyWork(this, pkt);
            }

            break;
        default:
            LOG_WARN("[%s:TCP] ���յ�δ֪�����󣬺���. msg_id:%d", IdString().c_str(), msg->msg_id_);           
            break;
        }
    }
}

int TermConnection::parsePacket( PtBase** msg, uint32_t* len )
{
    int ret = 0;

    recv_buffer_.SetReadPtr(0);
    ret = PtBase::CheckCompleteness(recv_buffer_,*len);
    if (ret < 0) {
        return -1;  //packet parse failed
    }

    if (ret == 0) {
        return 0;   //packet is not a completed packet.
    }

    //ret == 1 / 2
    ByteStream one_packet(recv_buffer_.GetBuffer(),*len);

    //first this packet parse as a request packet, if it
    // is not a request packet, parse as a response packet.
    *msg = PtRequestMessage::Parse(one_packet);
    if (*msg == NULL) {
        *msg = PtResponseMessage::Parse(one_packet);
    }            

    return (*msg) == NULL ? -1 : ret;
}

void TermConnection::SendHeartbeatResponse(string request_str)
{
    PT_TestDataDescriptor test_data_desc(request_str, to_string("%.3lf",get_up_time()));
    PtHeartbeatResponse   hb_response;
    ByteStream            bs;

    hb_response.Add(test_data_desc);
    bs = hb_response.Serialize();
    Write(bs.GetBuffer(),bs.Size());
}