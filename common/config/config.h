/*
 * @brief: node---> is a keyvalue or a group
 *         node { comment, keyvalue/group };
 */

#if !defined gehua_config_h_
#define gehua_config_h_

#include <string>
#include <vector>
#include <map>
#include <fstream>
#if defined _MSC_VER
# include <memory>
#else
# if defined __GNUC__
#  include <tr1/memory>
# endif
#endif
#include <cpplib/logger.h>

using ::std::string;
using ::std::vector;
using ::std::map;
using ::std::ifstream;
using ::std::ios;
#if (defined _MSC_VER) && (_MSC_VER == 1500)
using ::std::tr1::shared_ptr;
#else
using ::std::shared_ptr;
#endif

enum ItemType 
{
    ITComment,     // //comment
    ITKeyValue,    // key=value
    ITGroupBegin,  // groupname{
    ITGroupEnd,    // }
    ITUnknown,     // unknown type.
};

const string kSpace     = " ";
const string kTab       = "\t";
const string kEnter     = "\r";
const string kNewLine   = "\n";
const string kAllTrimed = "\r\n\t ";

// trim string.
inline string trim(string const& str, string const& trim_str = kAllTrimed)
{
    if (str.length() == 0)
        return "";

    size_t start = str.length(), end = str.length();
    bool started = false;
    bool is_trimed_str;
    for (size_t i = 0; i < str.length(); ++i) {
        is_trimed_str = false;
        for (size_t j = 0; j < trim_str.length(); ++j) {
            if (str[i] == trim_str[j]) {
                is_trimed_str = true;
                continue;
            }
        }

        if (is_trimed_str)
            continue;

        if (!started) {
            start = i;
            started = true;
        }
        end = i;
    }

    if (start == str.length())
        return "";

    return str.substr(start, end - start + 1);
}

const string kCommentFlag    = "//";
const string kKeyValueFlag   = "=";
const string kGroupBeginFlag = "{";
const string kGroupEndFlag   = "}";

inline ItemType getItemType(string const& trimed_str)
{
    string::size_type pos;

    if ((pos = trimed_str.find(kCommentFlag)) == 0)
        return ITComment;

    if ((pos = trimed_str.find(kGroupBeginFlag)) == trimed_str.length() - 1)
        return ITGroupBegin;

    if ((pos = trimed_str.find(kGroupEndFlag)) == 0)
        return ITGroupEnd;

    if ((pos = trimed_str.find(kKeyValueFlag)) != string::npos)
        return ITKeyValue;

    return ITUnknown;
}

struct Item
{
    ItemType type;
    string   value;
    string   key;
    Item(string const &line)
    {
        string trimed_line = trim(line);

        switch (type = getItemType(trimed_line)) {
        case ITComment:
            value = trim(trimed_line.substr(kCommentFlag.length()));
            break;
        case ITKeyValue:
            {
                string::size_type pos = trimed_line.find(kKeyValueFlag);
                key = trim(trimed_line.substr(0, pos));
                value = trim(trimed_line.substr(pos + 1));
            }
            break;
        case ITGroupBegin:
            {
                value = trim(trimed_line.substr(0, trimed_line.length() - 1));
            }
            break;
        case ITGroupEnd:
            value = "}";
            break;
        case ITUnknown:
            value = trimed_line;
        }
    }
};

struct Node
{
    vector<shared_ptr<Item> > comment_list;
    shared_ptr<Item>         entity;
};

struct Group
{
    Node grp_begin;
    Node grp_end;
    map<string, Node> keyvalue_pairs;
};

struct Config
{
    Config(Logger &logger, string const& file)
        : logger_(logger)
    {
        valid_ = true;
        ifstream ifs;
        ifs.open(file.c_str(), /*ios::nocreate | */ios::in);

        vector<shared_ptr<Item> > comment_list;
        string line;
        while (getline(ifs, line)) {

            shared_ptr<Item> item(new Item(line));
            if (item->value == "") continue;
           
            if (item->type == ITComment) {
                comment_list.push_back(item);
                continue;
            }

            if (item->type != ITGroupBegin) {
                valid_ = false;
                logger_.Error("配置文件起点类型非GroupBegin");
                break;
            }

            Group gp;
            gp.grp_begin.comment_list = comment_list;
            gp.grp_begin.entity = item;

            if (!read_group(ifs, &gp)) {
                valid_ = false;
                logger_.Error("配置文件读取组%s失败", item->value.c_str());
                break;
            }
            
            group_list_[item->value] = gp;

            comment_list.clear();      
        }
    }

    bool valid() const { return valid_; }

    string getOption(string const& group, string const& option)
    {
        map<string, Group>::iterator git = group_list_.find(group);
        if (git == group_list_.end())
            return "";

        map<string, Node>::iterator nit = git->second.keyvalue_pairs.find(option);
        if (nit == git->second.keyvalue_pairs.end())
            return "";

        return nit->second.entity->value;
    }



    
private:
    Logger             &logger_;
    bool               valid_;
    map<string, Group> group_list_;

    bool read_group(ifstream &ifs, Group *gp)
    {
        string line;
        vector<shared_ptr<Item> > comment_list;

        while (getline(ifs, line)) {

            shared_ptr<Item> item(new Item(line));
            if (item->value == "") continue;

            if (item->type == ITComment) {
                comment_list.push_back(item);
                continue;
            }

            if (item->type == ITGroupBegin) {
                logger_.Error("配置文件组中存在GroupBegin");
                return false;
            }

            if (item->type == ITGroupEnd) {
                gp->grp_end.comment_list = comment_list;
                gp->grp_end.entity = item;
                return true;
            }

            Node nd;
            nd.comment_list = comment_list;
            nd.entity = item;
            gp->keyvalue_pairs[item->key] = nd;

            comment_list.clear();
        }

        return false;
    }
};

#endif // !gehua_config_h_
