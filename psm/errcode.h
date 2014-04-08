/*
 *
 */


#if !defined gehua_error_code_h_
#define gehua_error_code_h_

enum LoginError {
    LoginOK = 0,
    LoginWorkPoolNotStarted = 0x88880001,
    LoginOdcLibDescIncompleted = 0x88880002,
    LoginUserInfoDescIncompleted = 0x88880003,
    LoginCertDataDescIncompleted = 0x88880004,
    LoginTerminalInfoDescIncompleted = 0x88880005,
    LoginTerminalInfoInvalided = 0x88880006,
    LoginUserInfoInvalided = 0x88880007,
    LoginCastCAIdFailed = 0x88880008,

};


#endif // ! gehua_error_code_h_