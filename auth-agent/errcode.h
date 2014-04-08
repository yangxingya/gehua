/*
 * @brief: define http server logic error code.
 */

#if !defined gehua_error_code_h_
#define gehua_error_code_h_


enum ErrorCode {
    EC_OdcLibLineNotExisted              = 0x80000001,         //挑战请求移植库行不存在
    EC_OdcLibDescParseFailed             = 0x80000002,
    EC_UserInfoLineNotExisted            = 0x80000003,
    EC_UserInfoDescParseFailed           = 0x80000004,
    EC_UserInfoValidFailed               = 0x80000005,
    EC_TestDataDescParseFailed           = 0x80000006,
    EC_MacLineNotExisted                 = 0x80000007,
    EC_MacDescParseFailed                = 0x80000008,
    EC_PublicKeyLineNotExisted           = 0x80000009,
    EC_PublicKeyDescParseFailed          = 0x8000000A,
    EC_ChallengeCodeDescParseFailed      = 0x8000000B,
    EC_MacOrMacPwdDescParseFailed        = 0x8000000C,
    EC_MacOrMacPwdDescParseMsgIdFailed   = 0x8000000D,
    EC_UserCertDataLineNotExisted        = 0x8000000E,
    EC_UserCertDataDescParseFailed       = 0x8000000F,

    EC_ValidLocalChallengeCodeNotExisted = 0x80000010,
    EC_ValidChallengeCodeLineNotExisted  = 0x80000011,
    EC_ValidChallengeCodeIsEmpty         = 0x80000012,
    EC_ValidChallengeCodeSizeNotMatched  = 0x80000013,
    EC_ValidChallengeCodeNotMatched      = 0x80000014,
    EC_ExtraUserCertFailed               = 0x80000015,
    EC_ValidUserCertFailed               = 0x80000016,
};

#endif // ! gehua_error_code_h_