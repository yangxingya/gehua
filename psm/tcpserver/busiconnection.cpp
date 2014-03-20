#include "../psmcontext.h"
#include "busiconnection.h"
#include <protocol/protocol_v2_common.h>
#include <protocol/protocol_v2_general.h>
#include <protocol/protocol_v2_cipher.h>
#include <protocol/protocol_v2_pb_common.h>
#include <protocol/protocol_v2_pb_descriptor.h>
#include <protocol/protocol_v2_pb.h>
#include <protocol/protocol_v2_pb_message.h>

void BusiConnection::OnDataReceived()
{
    while (true) {
        PbBase* msg = NULL;

        int      pkt_count = 0;
        uint32_t len       = 0;

        LOCK(lock, recv_mt_);
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


        switch ( msg->msg_id_ )
        {
        case CLOUDv2_PB_BSM_TERMSYNC_REQUEST:
            {
                PbTermSyncRequest *pkt = (PbTermSyncRequest*)msg;
                psm_ctx_->sm_request_process_svr_->AddTermSyncWork(this, pkt);
            }

            break;
        case CLOUDv2_PB_BSM_TERMREPORT_REQUEST:
            {
                PbTermReportRequest *pkt = (PbTermReportRequest*)msg;
                psm_ctx_->sm_request_process_svr_->AddTermReportWork(this, pkt);
            }

            break;
        case CLOUDv2_PB_BSM_SVCAPPLY_REQUEST:
            {
                PbSvcApplyRequest *pkt = (PbSvcApplyRequest*)msg;
                psm_ctx_->sm_request_process_svr_->AddSvcApplyWork(this, pkt);
            }

            break;
        case CLOUDv2_PB_BSM_SVCPCHANGE_REQUEST:
            {
                PbSvcPChangeRequest *pkt = (PbSvcPChangeRequest*)msg;
                psm_ctx_->sm_request_process_svr_->AddSvcPChangeWork(this, pkt);
            }

            break;
        }
    }
}

int BusiConnection::parsePacket(PbBase** msg, uint32_t* len)
{
    int ret = 0;

    recv_buffer_.SetReadPtr(0);
    ret = PbBase::CheckCompleteness(recv_buffer_,*len);
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
    *msg = PbRequestMessage::Parse(one_packet);
    if (*msg == NULL) {
        *msg = PbResponseMessage::Parse(one_packet);
    }            

    return (*msg) == NULL ? -1 : ret;
}
