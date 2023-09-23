#include "string.h"


namespace dcm {

    string::string(std::string str) : data { std::move(str) } {
    }


    string::string(char *str) {
        int i = 0;
        while (str[i] != 0) data.push_back(str[i++]);
    }


    string& string::clear() {
        data.clear();
        return *this;
    }


    string& string::append(char ch) {
        data.push_back(ch);
        return *this;
    }


    string& string::push_back(char ch) {
        data.push_back(ch);
        return *this;
    }


    bool string::empty() const {
        return data.empty();
    }


    std::vector<string> string::split (std::string_view delims) const {
        string current { "" };
        std::vector<string> ret { };
        for (auto const ch : data) {
            bool isDelim = false;
            for (auto const d : delims) {
                if (ch == d) {
                    ret.push_back(current);
                    current.clear();
                    isDelim = true;
                    break;
                }
            }
            if (!isDelim) current.push_back(ch);
        }
        if (!current.empty()) ret.push_back(current);
        return ret;
    }


    string& string::filter (std::string_view filterchars) {
        std::string ret {};
        for (auto const &ch : data) {
            bool isFiltered = false;
            for (auto const &f : filterchars) {
                if (ch == f) {
                    isFiltered = true;
                    break;
                }
            }
            if (!isFiltered) ret.push_back(ch);
        }
        data = ret;
        return *this;
    }


    string& string::trim () {
        std::string ret { };
        int idx = 0;
        while(idx < data.size() && (data[idx] == ' ' || data[idx] == '\t')) idx++;
        for (int i = idx; i < data.size(); i++) ret.push_back(data[i]);
        data = ret;
        return *this;
    }


    string& string::rtrim () {
        std::string ret { };
        int idx = static_cast<int>(data.size()) - 1;
        while(idx >= 0 && (data[idx] == ' ' || data[idx] == '\t')) idx--;
        for (int i = idx; i >= 0; i--) ret = fmt::format("{}{}", data[i], ret);
        data = ret;
        return *this;
    }


    string& string::join (std::vector<string>& strings, std::string_view delim) {
        for (auto const& str : strings) {
            data.append(fmt::format("{}{}", str.data, delim));
        }
        return *this;
    }


    int string::indexOf(char ch) {
        int idx = 0;
        for (auto const& c : data) {
            if (c == ch) return idx;
            idx++;
        }
        return -1;
    }


    int string::lastIndexOf(char ch) {
        for (int i = static_cast<int>(data.size()) - 1; i >= 0; i--) {
            if (data[i] == ch) return i;
        }
        return -1;
    }


    string string::substr(int start, int stop) {
        string ret { };
        for (int i = start, l = static_cast<int>(data.size()); i <= stop && i < l; i++) {
            ret.append(data[i]);
        }
        return ret;
    }


}
