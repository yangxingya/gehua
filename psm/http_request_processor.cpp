#include "http_request_processor.h"
#include "psmcontext.h"
#include "sessionmgr/casession.h"
#include "sessionmgr/termsession.h"

HttpRequstProcessor::HttpRequstProcessor( PSMContext *psm_context, unsigned int thread_count )
{
    psm_context_        = psm_context;
    thread_count_       = thread_count;

    if ( thread_count_ < 1 )
    {
        thread_count_ = 1;
    }

    for ( unsigned int i = 0; i < thread_count_; ++i )
    {
        HttpRequestThread *work_thread = new HttpRequestThread(psm_context_);
        work_thread->Start();
        work_thread_list_.push_back(work_thread);
    }

    index_ = 0;
}

HttpRequstProcessor::~HttpRequstProcessor()
{
    for ( unsigned int i = 0; i < thread_count_; ++i )
    {
        work_thread_list_[i]->Stop(100);
        delete work_thread_list_[i];
    }
    work_thread_list_.clear();
}

void HttpRequstProcessor::AddHttpRequestWork( SvcApplyWork *work )
{
    MutexLock lock(index_access_lock_);
    work_thread_list_[(index_++ % thread_count_)]->AddHttpRequestWork(work);
}

//////////////////////////////////////////////////////////////////////////

void HttpRequestWork::SendRequestFunc( Work *work )
{
    HttpRequestWork     *request_work   = (HttpRequestWork*)work;
    HttpRequestThread   *work_thread    = (HttpRequestThread*)request_work->thread_;

    work_thread->SendHttpRequest(request_work->svcapply_work);

    delete request_work;
    request_work = NULL;
}

//////////////////////////////////////////////////////////////////////////

HttpRequestThread::HttpRequestThread( PSMContext *psm_context )
{
    psm_context_ = psm_context;
    request_     = ghttp_request_new();
}


HttpRequestThread::~HttpRequestThread()
{
    ghttp_request_destroy(request_);
}

void HttpRequestThread::AddHttpRequestWork( SvcApplyWork *work )
{
    HttpRequestWork *request_work = new HttpRequestWork;
    request_work->svcapply_work = work;
    request_work->work_func_   = HttpRequestWork::SendRequestFunc;

    QueueWork((Work*)request_work);
}

void HttpRequestThread::SendHttpRequest( SvcApplyWork *work )
{
    ghttp_status status;
    work->http_request_info_.request_result_ = HTTPAysnRequestInfo::ConnectFailed;

    do 
    {
        if ( ghttp_set_uri(request_, (char*)(work->http_request_info_.request_url_.GetBuffer())) < 0 )
        {
            break;
        }

        if ( ghttp_set_type(request_, ghttp_type_post) < 0 )
        {
            break;
        }

        if ( ghttp_set_body(request_, (char*)(work->http_request_info_.request_body_.GetBuffer()), work->http_request_info_.request_body_.GetWritePtr()) < 0 )
        {
            break;
        }

        if ( ghttp_prepare(request_) < 0 )
        {
            break;
        }

        status = ghttp_process(request_);
        if ( status == ghttp_error )
        {
            break;
        }

        int     data_len    = ghttp_get_body_len(request_);
        char    *data_buf   = ghttp_get_body(request_);

        work->http_request_info_.responed_body_.Add(data_buf, data_len);
        work->http_request_info_.responed_body_.Add("\0");
        work->http_request_info_.request_result_ = HTTPAysnRequestInfo::OK;
    } while ( 0 );

    // add responed process work to process thread.
    psm_context_->busi_pool_->AddWork(work, work->self_session_info_->CAId());
}