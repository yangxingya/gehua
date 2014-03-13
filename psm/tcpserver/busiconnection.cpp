

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
            LOG_FATAL("[%s:TCP] 数据包解析失败：%s, dump:\n%s", IdString().c_str(), GetPeerAddr().c_str(), recv_buffer_.DumpHex(16,true).c_str());
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


        //todo:: work to add 
        Work *wk; // = new ...;
        //psm_ctx_->busi_pool_.Assign(wk, term_session_->CAId());
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
