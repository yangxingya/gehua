#ifndef SM_CONFIG_INFO_H_
#define SM_CONFIG_INFO_H_


#include <set>
#include <map>
using namespace std;


/**
 * @brief  Э��ͷ����
 */
#define BUSINESS_SVOD           "SuperVOD"
#define BUSINESS_CloudGame      "CyberCloud"
#define BUSINESS_PHONE_Control  "PhoneControl"


/**
 * @brief  ����SM�Ļ�����Ϣ
 */
struct SMItemInfo
{
    UINT            sm_id_;                  // SM�ı�ʶ
    string          sm_name_;                // SM����
    string          sm_addr_;                // SMҵ���ʼ�������IP:Port
    string          switch_rule_;            // SMҵ���л�����֧�ֿ��л���Դҵ�������б�
    set<string>     suppert_business_list_;  // SM֧�ֵķ����б��ô��������ҵ��Э��ͷ����CyberCloud��SVOD
};

typedef map<UINT, SMItemInfo*>        SMItemArray;
typedef map<string, SMItemInfo*>      SMSuppertSvrTable;


#endif
