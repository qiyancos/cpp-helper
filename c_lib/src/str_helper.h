#ifndef STR_HELPERS_H
#define STR_HELPERS_H

#include <string>
#include <sstream>
#include <ios>
#include <vector>
#include <locale>
#include <codecvt>

namespace htk
{

    /* ×Ö·û´®Æ¥Åä */
    inline bool startswith(const std::string& str, const std::string& head)
    {
        if (str.length() < head.length()) return false;
        if (str.substr(0, head.length()) == head) return true;
        return false;
    }
    inline bool startswith(const std::wstring& str, const std::wstring& head)
    {
        if (str.length() < head.length()) return false;
        if (str.substr(0, head.length()) == head) return true;
        return false;
    }

    inline bool endswith(const std::string& str, const std::string& tail)
    {
        if (str.length() < tail.length()) return false;
        if (str.substr(str.length() - tail.length(), tail.length()) == tail)
            return true;
        return false;
    }
    inline bool endswith(const std::wstring& str, const std::wstring& tail)
    {
        if (str.length() < tail.length()) return false;
        if (str.substr(str.length() - tail.length(), tail.length()) == tail)
            return true;
        return false;
    }

    /* ×Ö·û´®·Ö¸î/Æ´½Ó */
    std::vector<std::string> split(const std::string& str,
        const std::string& delimiter, const std::string& trim_str = "");
    std::vector<std::string> split_with_quot(const std::string& str,
        const char delimiter, const char quot = '"', const std::string& trim_str = "");

    std::string join(const std::vector<std::string> vec, const std::string& delimiter);

    /* ×Ö·û´®¼ô²Ã */
    std::string trim_left(const std::string& str, const std::string& sub_str);
    std::string trim_right(const std::string& str, const std::string& sub_str);
    std::string trim(const std::string& str, const std::string& sub_str);

    /* ¿í×Ö·û×ª»» */
    std::wstring String2Wstring(const std::string & str, const char* encoding = "chs");
    std::string Wstring2String(const std::wstring & wstr, const char* encoding = "chs");

    /* ±àÂë×ª»» */
    std::string GBK2UTF8(const std::string& strGBK);
    std::string UTF82GBK(const std::string& strUTF8);

    /* ±àÂë×ª»»(C++11) */
    // utility wrapper to adapt locale-bound facets for wstring/wbuffer convert
    template<class Facet>
    struct deletable_facet : Facet
    {
        using Facet::Facet; // inherit constructors
        ~deletable_facet() {}
    };
    std::string wstring2string(const std::wstring& str, const std::string& locale_str);
    std::wstring string2wstring(const std::string& str, const std::string& locale_str);
    std::string unicode_to_utf8(const std::wstring& str);
    std::wstring utf8_to_unicode(const std::string& str);
    std::string unicode_to_gb18030(const std::wstring& str);
    std::wstring gb18030_to_unicode(const std::string& str);
    std::string utf8_to_gb18030(const std::string& str);
    std::string gb18030_to_utf8(const std::string& str);

    /* ×Ö·û´®ÅÐ¶Ï TODO */
    bool isInt(const std::string& str);
    bool isFloat(const std::string& str);
    bool isBool(const std::string& str);

    /* ÕýÔòÏà¹Ø */
    bool RegexSearch(const std::string &str, const std::string &reg_str);
    bool RegexSearch(const std::wstring &wstr, const std::wstring &wreg_str);
    bool RegexSearch(const std::string &str, std::vector<std::string> &result, const std::string &reg_str);
    bool RegexSearch(const std::wstring &wstr, std::vector<std::wstring> &result, const std::wstring &wreg_str);

    bool RegexMatch(const std::string &str, const std::string &reg_str);
    bool RegexMatch(const std::wstring &wstr, const std::wstring &wreg_str);
    bool RegexMatch(const std::string &str, std::vector<std::string> &result, const std::string &reg_str);
    bool RegexMatch(const std::wstring &wstr, std::vector<std::wstring> &result, const std::wstring &wreg_str);

    std::string RegexReplace(const std::string &str, const std::string &reg_str, const std::string &sub_str);
    std::wstring RegexReplace(const std::wstring &wstr, const std::wstring &wreg_str, const std::wstring &wsub_str);

    /* ´óÐ¡Ð´×ª»» */
    std::string toLower(const std::string& str);
    std::string toUpper(const std::string& str);

    /* ×Ö·û´®ÓëÆäËûÀàÐÍ»¥×ª */
    template <typename T>
    T ConvertFromString(const std::string &s) {
        T value;
        std::stringstream ss(s);
        ss >> value;
        return value;
    }

    template <typename T>
    std::string ConvertToString(const T &s) {
        std::stringstream ss;
        ss << std::fixed << s;
        return ss.str();
    }

    // È«½Ç×ª°ë½Ç
    std::string SBC2DBC(const std::string &SBC);
}


#endif // STR_HELPERS_H
