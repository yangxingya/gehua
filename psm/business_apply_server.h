#ifndef BUSINESS_APPLY_SERVER_H_
#define BUSINESS_APPLY_SERVER_H_

#include "sm_config_info.h"
#include "http_request_processor.h"
#include <cpplib/stringtool.h>

struct PSMContext;

/*
 *	brief ҵ�����������
 */
class BusinessApplySvr
{
public:
    BusinessApplySvr(PSMContext *psm_context, unsigned int send_thread_count);
    ~BusinessApplySvr();

    /**
     * @brief ��ȡĿ��ҵ�����ڵ�SMͨ�ŵ�ַ
     * @param   apply_svr_name  Ҫ����ҵ�������
     * @param   addr_str        ����������سɹ����ò���������SM��ͨ�ŵ�ַ
     * @return ���ػ�ȡ���
     * @retval  0    �ɹ�
     * @retval  ���� ʧ��
     */
    int GetSMServiceAddr(string &apply_business_name, string &addr_str);

    BusinessType  GetBusinessType(string service_name);

    /**
     * @brief �ж�Ҫ�����ҵ���Ƿ�Ϊ�ֻ�����ҵ��
     * @param   apply_business_name  Ҫ����ҵ�������
     * @return ����TRUE or FALSE
     */
    bool IsPHONEControlSvc(const char *apply_business_name);

    bool IsValidServieName(string service_name);

    /**
     * @brief ���һ��ҵ���ʼ����������
     * @param   work  ������
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