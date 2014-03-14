#ifndef HTTP_REQUEST_PROCESSOR_H_
#define HTTP_REQUEST_PROCESSOR_H_

#include "work_def.h"
#include "ghttp.h"

struct PSMContext;

struct HttpRequestWork : public Work
{
    SvcApplyWork *svcapply_work;

    HTTPAysnRequestInfo http_request_info_;

    static void SendRequestFunc(Work *work);
};


class HttpRequestThread : public WorkQueue
{
public:
    HttpRequestThread(PSMContext *psm_context);
    virtual ~HttpRequestThread();

    void AddHttpRequestWork(SvcApplyWork *work);
    void SendHttpRequest(SvcApplyWork *work);

protected:
private:
    PSMContext              *psm_context_;
    ghttp_request           *request_;
};


class HttpRequstProcessor
{
public:
    HttpRequstProcessor(PSMContext *psm_context, unsigned int thread_count);
    ~HttpRequstProcessor();

    void AddHttpRequestWork(SvcApplyWork *work);
protected:
private:
    PSMContext                  *psm_context_;
    unsigned int                thread_count_;
    vector<HttpRequestThread*>  work_thread_list_;

    Mutex        index_access_lock_;
    unsigned int index_;
};


#endif