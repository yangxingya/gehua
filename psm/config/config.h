/*
* @brief: psm config info
*/

#if !defined gehua_config_h_
#define gehua_config_h_

#include <string>
#include <sstream>
#include <fstream>
#include "comm-def.h"

using ::std::string;
using ::std::ostringstream;
using ::std::fstream;

//define config format
//$<config name>  -> psm-config
//
//#v_type:<type>  -> int/string...
//#v_range:[min,max]/(min,max]/[min,max)/(min,max)/{el1, el2, el3, ...}
//#desc:
//key1=value1
//
//

template<typename T>
class VType 
{ 
public:
    VType(T) { line = "#v_type:" + typeid(T).name(); }
    string getline() const { return line; }
private:
    string line;
};

enum RangeClass {
    RangeArea,
    RangeSetElement,
};

enum RangeAreaClass {
    RALess,     // <  ---> "("
    RALOE,      // <= ---> "["       
    RAGreater,  // >  ---> ")"
    RAGOE,      // >= ---> "]"
};

struct RangleLimit {
    RangeClass cls;
    RangleAreaClass left_rel;
    RangleAreaClass right_rel;
};

template<typename T>
class VRange
{
public:
    VRange(RangleLimit limit, vector<T> const& range) 
        : limit_(limit), range_(range)
    {
        line_ = "#v_range:";

        size_t sz = range.size(); 
        switch (limit.cls) {
case RangeArea: 
    sz = 2;
    switch (limit.left_rel) {
case RALess:
    line_ += "(";
    break;
case RALOE:
    line_ += "[";
    break;
    }
    break;
case RangeSetElement:
    line_ += "{";
    break;
        }

        ostringstream oss;
        for (size_t i = 0; i < sz - 1; ++i) {
            oss << range[i] << ",";
        }
        oss << range[sz - 1];

        line_ += oss.str();

        switch (limit.cls) {
case RangeArea: 
    switch (limit.right_rel) {
case RAGreater:
    line_ += ")";
    break;
case RAGOE:
    line_ += "]";
    break;
    }
    break;
case RangeSetElement:
    line_ += "}";
    break;
        }
    }

    VRange(string const& line) {
        string pfx = "#v_range:";
        string::size_type pos = line.find(pfx);
        if (pos != string::npos && pos == 0) {
            RangeLimit limit;
            string value = line.substr(pos + pfx.length());
            if (value[0] == '{' && 
                value[value.length - 1] == '}') {
                    limit.cls = RangeSetElement;
            } else {
                limit.cls = RangeArea;
                if (value[0] == '(') {
                    limit.left_rl = RALess;
                } else if (value[0] == '[') {
                    limit.left_rl = RALOE;
                } else {
                    valid_ = false;
                    return;
                }

                if (value[value.length - 1] == ')') {
                    limit.right_rl = RAGreater;
                } else if (value[value.length - 1] == ']') {
                    limit.right_rl = RAGOE;
                } else {
                    valid_ = false;
                    return;
                }
            }

            vector<string> rangestr;
            split(value.substr(1, value.length() - 2), ",", &rangestr);

            if (rangestr.size() == 0) {
                valid_ = false;
                return;
            }

            if (limit.cls == RangeArea && rangestr.size() != 2) {
                valid_ = false;
                return;
            }

            vector<T> range;
            for (size_t i = 0; i < rangestr.size(); ++i) {
                istringstream iss(rangestr[i]);
                T tmp;
                iss >> tmp;
                range.push_back(tmp);
            }

            limit_ = limit;
            range_ = range;
            valid_ = true;
        }
    }

    string getline() const { return line_; }
    bool valid() const { return valid_; }

    RangeLimit limit() const { return limit_; }
    vector<T>& range() const { return range_; }
private:
    string line_;
    RangeLimit limit_;
    vector<T> range_;
    bool valid_;
};

struct Desc
{
    Desc(string const& desc) {
        desc_ = "#desc:" + desc;
    }
    string getline() const { return desc_; }
private:
    string desc_;
};

template<typename T>
class KeyValue
{
public:
    KeyValue(string const& key, T const& value) 
    {
        ostringstream oss;
        oss << value;
        line_ = key + "=" + oss.str();
    }
    KeyValue(string const& line) 
    {
        string::size_type pos = line.find("=");
        valid_ = false;
        if (pos != string::npos) {
            string key = line.substr(0, pos);
            string vstr = line.substr(pos + 1);
            T tmp;
            istringstream iss(vstr);
            iss >> tmp;

            key_ = key;
            value_ = tmp;
            valid_ = true;
            return;
        }
    }
    string getline() const { return line_; }
    bool valid() const { return valid_; }
    string key() const { return key_; }
    T& value() const { return value_; }
private:
    string line_;
    string key_;
    T value_;
};

const int kOneLineMaxCnt = 512;

template<typename T> 
class ConfigItem
{
public:
    ConfigItem(vector<string> const& item)
    {

    }
};

struct Config
{
    uint16_t term_tcpserver_port;
    uint16_t busi_tcpserver_port;

    Config(string const& file) 
    {
        fstream fs;
        fs.open(file.c_str(); ios::in | ios::nocreate);
        if (!fs.is_open()) {
            create(file);
        } else {
            read(fs);
        }
    }

    bool read(fstream &fs)
    {
        char *buff = new char[kOneLineMaxCnt];

        string line;
        fs.getline(buf, kOneLineMaxCnt);
        line = buf;
        if (line[0] != '$') {
            // read config name.
            return false;
        }

        vector<ConfigItem> cfgitems;

        while (!fs.eof()) {
            fs.getline(buf, kOneLineMaxCnt);
            line = buf;
            vector<string> item;
            if (line.find("#v_type:") == 0) {
                // v_type:
                item.push_back(line);

                // v_range:
                fs.getline(buf, kOneLineMaxCnt);
                item.push_back(buf);

                // desc:
                fs.getline(buf, kOneLineMaxCnt);
                item.push_back(buf);

                //key=value
                fs.getline(buf, kOneLineMaxCnt);
                item.push_back(buf);

                ConfigItem cfgitem(item);

            }
        }

    }
};

#endif // !gehua_config_h_