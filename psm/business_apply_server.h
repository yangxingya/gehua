#ifndef BUSINESS_APPLY_SERVER_H_
#define BUSINESS_APPLY_SERVER_H_

#include "sm_config_info.h"
#include "http_request_processor.h"
#include <cpplib/stringtool.h>

struct PSMContext;

/*
 *	brief 业务申请服务类
 */
class BusinessApplySvr
{
public:
    BusinessApplySvr(PSMContext *psm_context, unsigned int send_thread_count);
    ~BusinessApplySvr();

    /**
     * @brief 获取目标业务所在的SM通信地址
     * @param   apply_svr_name  要申请业务的名称
     * @param   addr_str        如果函数返回成功，该参数将保存SM的通信地址
     * @return 返回获取结果
     * @retval  0    成功
     * @retval  其他 失败
     */
    int GetSMServiceAddr(string &apply_business_name, string &addr_str);

    BusinessType  GetBusinessType(string service_name);

    /**
     * @brief 判断要申请的业务是否为手机外设业务
     * @param   apply_business_name  要申请业务的名称
     * @return 返回TRUE or FALSE
     */
    bool IsPHONEControlSvc(const char *apply_business_name);

    bool IsValidServieName(string service_name);

    /**
     * @brief 添加一个业务初始化请求工作项
     * @param   work  工作项
     */
    void AddInitRequestWork(SvcApplyWork *work);

private:
    vector<SMItemInfo*> sm_item_list_;
    SMSuppertSvrTable   sm_suppertsvr_table_;

    PSMContext   *psm_context_;
    unsigned int send_thread_count_;

    HttpRequstProcessor *http_request_processor_;
};


#endif 