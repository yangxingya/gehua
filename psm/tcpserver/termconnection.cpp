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

    //login valid.
    if (!login_) {
      if (msg->msg_id_ != CLOUDv2_PT_TERM_LOGIN_REQUEST) {
        // set session invalid
        LOG_WARN("[%s:TCP] 终端连接后第一个请求非登录请求：%s, dump:\n%s", IdString().c_str(), GetPeerAddr().c_str(), recv_buffer_.DumpHex(16,true).c_str());
        SetDirty();
        break;  
      }

      term_session_ = psm_ctx_->busi_pool_.GenTermSession((PtLoginRequest *)msg, this);
      if (term_session_ == NULL) {
        LOG_WARN("[%s:TCP] 终端连接后第一个请求验证失败：%s, dump:\n%s", IdString().c_str(), GetPeerAddr().c_str(), recv_buffer_.DumpHex(16,true).c_str());
        SetDirty();
        break;
      }
      login_ = true;
    }

    //todo:: work to add 
    Work *wk; // = new ...;
    //psm_ctx_->busi_pool_.Assign(wk, term_session_->CAId());

    /*
    //business logic 
    PtHeartbeatRequest* hb_msg = dynamic_cast<PtHeartbeatRequest*>(msg);
    if ( hb_msg != NULL && hb_msg->test_data_desc_.valid_ ) {
    last_heartbeat_time_ = get_up_time();
    SendHeartbeatResponse( hb_msg->test_data_desc_.request_str_ );
    }
    delete msg;
    */
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