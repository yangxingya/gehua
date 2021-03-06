#include "../psmcontext.h"
#include "termconnection.h"
#include "../sessionmgr/casession.h"
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
			LOG_TRACE("[%s:TCP] [Receive Data Packet]: %s, dump:\n%s", IdString().c_str(), GetPeerAddr().c_str(), recv_buffer_.DumpHex(16,true).c_str());
		}

		pkt_count = parsePacket(&msg, &len);
		if (pkt_count < 0) {
			// it is a invalid packet.
			LOG_FATAL("[%s:TCP] [Data Packet Parse Failured]: %s, dump:\n%s", IdString().c_str(), GetPeerAddr().c_str(), recv_buffer_.DumpHex(16,true).c_str());
			SetDirty();
			break;
		}
		if ( pkt_count == 0 ) {
			// it is a incompleted packet.
			break;
		}

		// parse successful 
		ByteStream one_packet(recv_buffer_.GetBuffer(),len);            

		uint64_t ts_id = msg->ParseSessionID(one_packet);

		recv_buffer_.SetReadPtr(len);
		recv_buffer_.FlushReadPtr();

		if (!login_) {
			switch (msg->msg_id_) {
			case CLOUDv2_PT_TERM_HEARTBEAT_REQUEST:
				//重用会话。
				login_ = reused_termsession(ts_id);
				logger_->Info("[Terminal New Connection] %s session id: 0x"SFMT64X"", login_ ? "reuse" : "no reuse", ts_id);
				break;
			case CLOUDv2_PT_TERM_LOGIN_REQUEST:
				login_ = gen_termsession((PtLoginRequest*)msg);
				logger_->Info("[Terminal New Connection] %s generate session id: 0x"SFMT64X"", login_ ? "success" : "fail", ts_id);
				break;
			default:
				// set session invalid
				LOG_WARN("[%s:TCP] [Terminal New Connection] the first request is not login request: %s, XXXXX:\n%s", IdString().c_str(), GetPeerAddr().c_str(), recv_buffer_.DumpHex(16,true).c_str());
				break;
			}
		}

		if (!login_) {
			SetDirty();
			logger_->Warn("[Terminal New Connection] login failure, disconnection!!!");
			break;
		}

		switch ( msg->msg_id_ )
		{
		case CLOUDv2_PT_TERM_LOGIN_REQUEST:      
			{
				PtLoginRequest *pkt = (PtLoginRequest*)msg;
				psm_ctx_->term_request_process_svr_->AddLoginRequestWork(term_session_, pkt);
			}

			break;
		case CLOUDv2_PT_TERM_LOGOUT_REQUEST:                
			{
				PtLogoutRequest *pkt = (PtLogoutRequest*)msg;
				psm_ctx_->term_request_process_svr_->AddLogoutRequestWork(term_session_, pkt);
			}

			break;
		case CLOUDv2_PT_TERM_HEARTBEAT_REQUEST:  
			{
				PtHeartbeatRequest *pkt = (PtHeartbeatRequest*)msg;
				psm_ctx_->term_request_process_svr_->AddHeartbeatWork(term_session_, pkt);
			}

			break;
		case CLOUDv2_PT_TERM_SVCAPPLY_REQUEST:
			{
				PtSvcApplyRequest *pkt = (PtSvcApplyRequest *)msg;
				psm_ctx_->term_request_process_svr_->AddSvcApplyWork(term_session_, pkt);
			}

			break;
		case CLOUDv2_PT_TERM_STATUSQUERY_REQUEST:
			{
				PtStatusQueryRequest *pkt = (PtStatusQueryRequest *)msg;
				psm_ctx_->term_request_process_svr_->AddStatusQueryWork(term_session_, pkt);
			}

			break;
		case CLOUDv2_PT_TERM_KEYMAPPING_REQUEST:
			{
				PtKeyMappingRequest *pkt = (PtKeyMappingRequest *)msg;
				psm_ctx_->term_basic_func_svr_->AddKeyTransmitWork(term_session_, pkt);
			}

			break;
		case CLOUDv2_PT_TERM_GETSVCGROUP_REQUEST:
			{
				PtGetSvcGroupRequest *pkt = (PtGetSvcGroupRequest *)msg;
				psm_ctx_->term_request_process_svr_->AddGetSvrGroupWork(term_session_, pkt);
			}
			break;

		case CLOUDv2_PT_TERM_SVCSWITCH_RESPONSE:
			{
				PtSvcSwitchResponse *pkt = (PtSvcSwitchResponse *)msg;
				psm_ctx_->term_basic_func_svr_->AddSvcSwitchNotifyWork(term_session_, pkt);
			}

			break;
		case CLOUDv2_PT_TERM_STATUSNOTIFY_RESPONSE:
			{
				PtStatusNotifyResponse *pkt = (PtStatusNotifyResponse *)msg;
				psm_ctx_->term_basic_func_svr_->AddStatusPChangeNotifyWork(term_session_, pkt);
			}

			break;
		default:
			LOG_WARN("[%s:TCP] receive unknown request!!! ignore it!!!. msg_id:%d", IdString().c_str(), msg->msg_id_);           
			break;
		}
	}
}

bool TermConnection::reused_termsession(uint64_t ts_id)
{
	if (ts_id == (uint64_t)-1) return false;

	logger_->Info("[Terminal Reconnection] terminal session id: "SFMT64U"", ts_id);
	weak_ptr<TermSession> ts = psm_ctx_->busi_pool_->FindTermSessionById(ts_id);
	shared_ptr<TermSession> sp_ts(ts.lock());
	if (!sp_ts) {
		logger_->Warn(
			"[Terminal Reconnection] terminal session id: 0x"SFMT64X
			" not existed! no need response, terminal will reassign PSM with http", ts_id);
		return false; 
	}

	term_session_ = sp_ts;

	MutexLock lock(sp_ts->termconn_mtx_);
	if (sp_ts->term_conn_)
		sp_ts->term_conn_->SetDirty();
	sp_ts->term_conn_ = this;

	return true;
}

bool TermConnection::valid_login(PtLoginRequest *msg)
{
	Logger *logger = (Logger*)logger_;
	TermSession ts(*logger, msg, NULL);
	if (!ts.valid()) return false;

	if (ts.terminal_info_desc_.terminal_class_ != TerminalSTB) return true;

	// if find STB terminal in one group with same card id, 
	// then login a STB terminal, will be replace old stb session.
	shared_ptr<TermSession> old_stb_session(psm_ctx_->busi_pool_->GetSTBTermSession(ts.CAId()).lock());
	if (old_stb_session) {
		psm_ctx_->busi_pool_->DelTermSession(old_stb_session);
		psm_ctx_->busi_pool_->RemoveFromTimer(old_stb_session);

		MutexLock lock(old_stb_session->termconn_mtx_);
		if (old_stb_session->term_conn_) {
			old_stb_session->term_conn_->SetDirty();
			old_stb_session->term_conn_ = NULL;
		}
		logger_->Info(
			"[Terminal Login] Delete old STB Terminal session, id: 0x"SFMT64X" CA id: "SFMT64U"", 
			old_stb_session->Id(), old_stb_session->CAId());

	}

	return true;

}

bool TermConnection::gen_termsession(PtLoginRequest *msg)
{
	if (!valid_login(msg)) return false;

	weak_ptr<TermSession> term_session = psm_ctx_->busi_pool_->GenTermSession((PtLoginRequest *)msg, this);
	shared_ptr<TermSession> sp_term_session(term_session.lock());
	if (!sp_term_session) {
		LOG_WARN("[%s:TCP] 终端连接后第一个请求验证失败：%s", 
			IdString().c_str(), 
			GetPeerAddr().c_str());
		SetDirty();
		return false;
	}
	term_session_ = sp_term_session;
	psm_ctx_->busi_pool_->AddToTimer(term_session_);

	return true;
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
	PT_TestDataDescriptor test_data_desc;
	PtHeartbeatResponse   hb_response;
	ByteStream            bs;

	hb_response.Add(test_data_desc);
	bs = hb_response.Serialize();
	Write(bs.GetBuffer(),bs.Size());
}