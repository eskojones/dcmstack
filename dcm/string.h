#ifndef DCM_STRING_H
#define DCM_STRING_H
#include <iostream>
#include <map>
#include <fmt/core.h>
#include <vector>

namespace dcm {
    class string {
        public:
            typedef typename std::string array_type;
            typedef typename array_type::iterator iterator;
            typedef typename array_type::const_iterator const_iterator;
            inline iterator begin() noexcept;
            [[nodiscard]] inline const_iterator cbegin() const noexcept;
            inline iterator end() noexcept;
            [[nodiscard]] inline const_iterator cend() const noexcept;
            std::string data;

            explicit string(std::string str = "");
            explicit string(char *str);
            string& clear();
            string& append(char ch);
            string& push_back(char ch);
            [[nodiscard]] bool empty() const;
            [[nodiscard]] std::vector<string> split(std::string_view delims) const;
            string& filter(std::string_view filterchars);
            string& trim();
            string& rtrim();
            string& join (std::vector<string>& strings, std::string_view delim);
    };
}

#endif //DCM_STRING_H
