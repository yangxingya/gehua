/*
* @brief: common define.
*
*/

#if !defined gehua_common_widget_h_
#define gehua_common_widget_h_

#include <cpplib/cpplibbase.h>
#include <cpplib/logger.h>

typedef uint64_t caid_t;

enum ByteOrder {
    OrderLittleEndian = 1,
    OrderBigEndian = 2,
    OrderNet = OrderBigEndian,
};

inline ByteOrder test_byte_order()
{
    uint16_t tmp = 0x1234;
    uint8_t *ptmp = (uint8_t*)&tmp;
    if (ptmp[0] == 0x12)
        return OrderLittleEndian;
    return OrderBigEndian;
}

//static ByteOrder g_byteorder = test_byte_order();

template<typename T>
inline T change_order(T const& t)
{
    T tmp;

    uint8_t *src = (uint8_t*)&t;
    uint8_t *dst = (uint8_t*)&tmp;
    for (size_t i = 0; i < sizeof(t); ++i) {
        dst[i] = src[sizeof(t) - 1 - i];
    }

    return tmp;
}

inline uint8_t* change_order(uint8_t* t, size_t sz)
{
    uint8_t tmp;
    size_t half = sz / 2;
    for (size_t i = 0; i < half; ++i) {
        tmp = t[i];
        t[i] = t[sz - 1 - i];
        t[sz - 1 - i] = tmp;
    }

    return t;
}

inline string change_order(string const& t)
{
    string dest;
    size_t len = t.length();
    dest.resize(len);

    for (size_t i = 0; i < len; ++i)
        dest[i] = t[len - 1 - i];

    return dest;
}

inline string to_lower(string const& str)
{
    string tmp = str;
    for (string::size_type i = 0; i < tmp.length(); ++i) {
        if (tmp[i] >= 'A' && tmp[i] <= 'Z')
            tmp[i] = tmp[i] - 'A' + 'a';
    }

    return tmp;
}

inline string to_higher(string const& str)
{
    string tmp = str;
    for (string::size_type i = 0; i < tmp.length(); ++i) {
        if (tmp[i] >= 'a' && tmp[i] <= 'z')
            tmp[i] = tmp[i] - 'a' + 'A';
    }

    return tmp;
}

inline size_t split(string const& str, string const& sp, 
                    vector<string> *out)
{
    string::size_type pos1, pos2;

    size_t origin_sz = out->size();
    pos1 = 0;      
    while ((pos2 = str.find(sp, pos1)) != string::npos) {
        out->push_back(str.substr(pos1, pos2 - pos1));
        pos1 = pos2 + sp.length();
    }
    out->push_back(str.substr(pos1));

    return (size_t)(out->size() - origin_sz);
}

//split string likely "key1=value1&key2=value2..." to
// map["key1"] = "value1", map["key2"] = "value2".
inline size_t split(string const& str, string const& sp1, 
                    string const& sp2, map<string, string> *out,
                    bool case_sensitivity = true)
{
    vector<string> strvec;

    split(str, sp1, &strvec);

    map<string, string> kv_map;

    string::size_type pos;
    for (size_t i = 0; i < strvec.size(); ++i) {
        pos = strvec[i].find(sp2);
        if (string::npos == pos) {
            kv_map.clear();
            break;
        }
        if (case_sensitivity)
            kv_map[strvec[i].substr(0, pos)] = strvec[i].substr(pos + sp2.length());
        else
            kv_map[to_lower(strvec[i].substr(0, pos))] = strvec[i].substr(pos + sp2.length());
    }

    *out = kv_map;

    return kv_map.size();
}

inline bool is_number(char c)
{
    return c >= '0' && c <= '9';
}

inline uint64_t to_uint64(string const& str, bool *cast_ok)
{
    // default is 10 not 16 band
    uint64_t ret = 0;
    for (size_t i = 0; i < str.length(); ++i) {
        if (!is_number(str[i])) {
            *cast_ok = false;
            return 0;
        }
        ret = ret * 10;
        ret = ret + (str[i] - '0');
    }

    *cast_ok = true;
    return ret;
}

// convert <ip:port> to <ip>, 
// <ip:port> support <*:port> or <127.0.0.1:port> format
// will get local ip address, if get failure return "0.0.0.0";
inline string ip_string(string const& ip_and_port) 
{
    /*
    // listen_addr_ is likely "192.168.15.3:13333" format
    string::size_type pos = ip_and_port.find(":");
    string ip = ip_and_port.substr(0, pos);

    struct in_addr addr;

    //get local host real ip address.
    if (ip == "*" || ip == "127.0.0.1") {

        ip = "0.0.0.0";

        char host[256];
        gethostname(host, 256);
        struct hostent* hosts = gethostbyname(host);

        //internet ip.
        if (hosts->h_addrtype == AF_INET) {
            addr.s_addr = *(u_long *) hosts->h_addr_list[0];
            ip = inet_ntoa(addr);
        }
    }
    return ip;*/
    return "127.0.0.1";//??????
}

// convert ip likely "10.12.11.13" to struct in_addr format <ulong> type.
inline uint32_t ip_cast(string const& ipstr)
{
    vector<string> out;
    split(ipstr, ".", &out);
    if (out.size() != 4)
        return 0;

    #pragma pack(1)
    union tmp_t {
        struct {
            uint8_t t1;
            uint8_t t2;
            uint8_t t3;
            uint8_t t4;
        } bs;
        uint32_t value;
    };
    #pragma pack()


    tmp_t tmp;

    tmp.bs.t1 = (uint8_t)atoi(out[0].c_str());
    tmp.bs.t2 = (uint8_t)atoi(out[1].c_str());
    tmp.bs.t3 = (uint8_t)atoi(out[2].c_str());
    tmp.bs.t4 = (uint8_t)atoi(out[3].c_str());

    return tmp.value;
}

const string kHexArray = "0123456789ABCDEF";

// change data array to hex string 
// ex: 0x1357 ---> "1357"; 11 ---> "B"
inline string hex_string(uint8_t const* buf, size_t sz)
{
    string tmp;
    tmp.resize(sz);

    for (size_t i = 0; i < sz; ++i) {
        tmp[i] = kHexArray[buf[i]];
    }

    return tmp;
}

inline string hex_string(vector<uint8_t> const& buf)
{
    return hex_string(&buf[0], buf.size());
}

const int kAto0Diff = 'A' - '0';
const int kFto0Diff = 'F' - '0';
const int kato0Diff = 'a' - '0';
const int kfto0Diff = 'f' - '0';

// change string to buffer array, 'a' & 'A' -> 0xa, 
// loss case sensitivity <lower or higher to number> information
inline size_t to_array(string const& arr, uint8_t *out)
{
    assert(out != 0);

    uint8_t x;
    size_t cnt = 0;
    // it seperate to the following area:
    //  (#, 0) 
    //     [0, 9], (9, 'A'-'0'), 
    //     ['A'-'0', 'F'-'0'], ('F'-'0', 'a'-'0'),
    //     ['a'-'0', 'f'-'0'], 
    //  ('f'-'0', #)
    for (size_t i = 0; i < arr.length(); ++i) {
        x = arr[i] - '0';

        // (#, 0)
        if (x < 0) continue;

        // [0, 9]
        if (x <= 9) {
            out[i] = x;
            cnt++;
            continue;
        }

        // (9, 'A'-'0')
        if (x < kAto0Diff) continue;

        // ['A'-'0', 'F'-'0']
        if (x <= kFto0Diff) {
            out[i] = x- kAto0Diff + 10;
            cnt++;
            continue;
        }

        // ('F'-'0', 'a'-'0')
        if (x < kato0Diff) continue;

        // ['a'-'0', 'f'-'0']
        if (x <= kfto0Diff) {
            out[i] = x - kato0Diff + 10;
            cnt++;
            continue;
        }

        // ('f'-'0', #)
    }

    return cnt;
}

inline size_t to_array(string const& arr, vector<uint8_t> *out)
{
    out->resize(arr.length());
    return to_array(arr, &(*out)[0]);
}

#endif //!gehua_common_widget_h_
