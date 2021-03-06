#ifndef SM_CONFIG_INFO_H_
#define SM_CONFIG_INFO_H_

#include "bs-comm-def.h"


/**
 * @brief  单个SM的基础信息
 */
struct SMItemInfo
{
    uint64_t        sm_id_;                  // SM的标识
    string          sm_name_;                // SM名称
    string          sm_addr_;                // SM业务初始化服务的IP:Port
    string          switch_rule_;            // SM业务切换规则，支持可切换的源业务名称列表
    list<string>    suppert_business_list_;  // SM支持的服务列表，该处保存的是业务协议头，如CyberCloud、SVOD
};

typedef map<string, SMItemInfo*>      SMSuppertSvrTable;

#endif
