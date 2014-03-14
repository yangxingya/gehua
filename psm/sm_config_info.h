#ifndef SM_CONFIG_INFO_H_
#define SM_CONFIG_INFO_H_


#include <set>
#include <map>
using namespace std;


/**
 * @brief  协议头定义
 */
#define BUSINESS_SVOD           "SuperVOD"
#define BUSINESS_CloudGame      "CyberCloud"
#define BUSINESS_PHONE_Control  "PhoneControl"


/**
 * @brief  单个SM的基础信息
 */
struct SMItemInfo
{
    UINT            sm_id_;                  // SM的标识
    string          sm_name_;                // SM名称
    string          sm_addr_;                // SM业务初始化服务的IP:Port
    string          switch_rule_;            // SM业务切换规则，支持可切换的源业务名称列表
    set<string>     suppert_business_list_;  // SM支持的服务列表，该处保存的是业务协议头，如CyberCloud、SVOD
};

typedef map<UINT, SMItemInfo*>        SMItemArray;
typedef map<string, SMItemInfo*>      SMSuppertSvrTable;


#endif
