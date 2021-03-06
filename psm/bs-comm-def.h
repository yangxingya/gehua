/*
* @brief: business common define
*/

#if !defined gehua_business_common_define_h_
#define gehua_business_common_define_h_

#include <cpplib/cpplibbase.h>

/**
 * @brief  协议头定义
 */
#define SERVICE_SVOD           "SuperVOD"
#define SERVICE_CloudGame      "CyberCloud"
#define SERVICE_PhoneControl   "PhoneControl"


// Business page status, 
// read from config.
enum BusinessType 
{
    BSBase = 1,
    BSPortal = 2,
    BSGame = 3,
    BSVOD = 4,
    BSPhoneControl = 5,
    BSPending = 6,
};

enum TerminalClass 
{
    TerminalSTB = 1,
    TerminalPhone = 2,
    TerminalPC = 3,
};

enum STBClass 
{
    STBOneWay = 1,
    STBTwoWayHD = 2,
    STBTwoWaySD = 3,
};

enum PhoneClass 
{
    PhoneAndriod = 1,
    PhoneIPhone = 2,
    PhoneWPhone = 3,
};

enum PCOSClass 
{
    OSWindows = 1,
    OSLinux = 2,
    OSMac = 3,
};

const string kTerminalClassName[] = {"STB", "Phone", "PC"};
const string kSubClassName[][3] = {
    { "One Way", "Two Way HD", "Two Way SD"},
    { "Anroid", "IPhone", "WPhone"},
    { "Windows", "Linux", "Mac"}
};

enum Modulation 
{
    QAM16 = 0x01,
    QAM32 = 0x02,
    QAM64 = 0x03,
    QAM128 = 0x04,
    QAM256 = 0x05,
};

const string kModulationValue[] = {
    "QAM16", "QAM32", "QAM64", "QAM128", "QAM256" };

struct ServiceGroup 
{
    uint32_t sg_id;
    uint32_t freq;
    uint32_t symbol_rate;
    Modulation modulation;
};

struct OdcInfo
{
    string version;

    /* bcd encode ex: 2014-03-10 -> 0x20140310, if it is 0,
    * indicate it is standard version, will not valid expired
    * date.
    */
    uint32_t expired_date;
};

#endif // !gehua_business_common_define_h_
