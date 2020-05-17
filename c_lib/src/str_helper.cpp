#include "str_helper.h"
#include <wchar.h>
#include <memory.h>
#include <regex>
#include <iostream>
#include <cctype>
#ifdef WIN32
#include <windows.h>
#endif

namespace htk
{
    std::vector<std::string> split(const std::string &str, 
        const std::string &delimiter, const std::string& trim_str)
    {
        std::vector<std::string> v;

        std::string::size_type sep_size = delimiter.size();
        std::string::size_type start, end;
        start = 0;
        end = str.find(delimiter);
        while (std::string::npos != end)
        {
            std::string sub_s = str.substr(start, end - start);
            if (trim_str != "")
            {
                sub_s = htk::trim(sub_s, trim_str);
            }
            v.push_back(sub_s);
            start = end + sep_size;
            end = str.find(delimiter, start);
        }
        if (start != str.length())
        {
            std::string sub_s = str.substr(start);
            if (trim_str != "")
            {
                sub_s = htk::trim(sub_s, trim_str);
            }
            v.push_back(sub_s);
        }

        return v;
    }

    std::vector<std::string> split_with_quot(const std::string & str, const char delimiter, const char quot, const std::string & trim_str)
    {
        std::vector<std::string> v;
        std::string::size_type start = 0;
        bool quot_status = false;

        std::string sub_s;
        for (size_t i = 0; i < str.size(); ++i)
        {
            char ch = str[i];

            if (ch == quot) quot_status ^= true;    // 切换引号状态
            if (!quot_status && ch == delimiter)
            {
                sub_s = str.substr(start, i - start);
                if (trim_str != "")
                {
                    sub_s = htk::trim(sub_s, trim_str);
                }
                v.push_back(sub_s);
                start = i + 1;
            }

            if ((i == str.size() - 1) && ch != delimiter)
            {
                sub_s = str.substr(start, i + 1 - start);
                if (trim_str != "")
                {
                    sub_s = htk::trim(sub_s, trim_str);
                }
                v.push_back(sub_s);
            }
        }

        return v;
    }

    std::string join(const std::vector<std::string> vec, const std::string & delimiter)
    {
        size_t n = vec.size();
        if ( 0 == n )
        {
            return std::string("");
        }

        std::string rev(vec[0]);
        for (size_t i = 1; i < n; ++i)
        {
            rev += delimiter;
            rev += vec[i];
        }

        return rev;
    }

    std::string trim_left(const std::string & str, const std::string & sub_str)
    {
        std::string rev(str);
        size_t start = rev.find_first_not_of(sub_str);
        if (start == std::string::npos)
        {
            rev.clear();
        }
        else
        {
            rev = rev.substr(start, str.size() - start + 1);
        }
        return rev;
    }

    std::string trim_right(const std::string & str, const std::string & sub_str)
    {
        std::string rev(str);
        size_t end = rev.find_last_not_of(sub_str);
        if (end == std::string::npos)
        {
            rev.clear();
        }
        else
        {
            rev = rev.substr(0, end + 1);
        }
        return rev;
    }

    std::string trim(const std::string & str, const std::string & sub_str)
    {
        return trim_right(trim_left(str, sub_str), sub_str);
    }

    std::wstring String2Wstring(const std::string & str, const char* encoding)
    {
        std::string curLocale = setlocale(LC_ALL, NULL);
        setlocale(LC_ALL, encoding);

        const char* _Source = str.c_str();
        size_t _Dsize = str.size() + 1;
        wchar_t *_Dest = new wchar_t[_Dsize];
        wmemset(_Dest, 0, _Dsize);
        mbstowcs(_Dest, _Source, _Dsize);
        std::wstring result = _Dest;
        delete[]_Dest;

        setlocale(LC_ALL, curLocale.c_str());

        return result;
    }

    std::string Wstring2String(const std::wstring & wstr, const char* encoding)
    {
        std::string curLocale = setlocale(LC_ALL, NULL);

        setlocale(LC_ALL, encoding);

        const wchar_t* _Source = wstr.c_str();
        size_t _Dsize = 2 * wstr.size() + 1;
        char *_Dest = new char[_Dsize];
        memset(_Dest, 0, _Dsize);
        wcstombs(_Dest, _Source, _Dsize);
        std::string result = _Dest;
        delete[]_Dest;

        setlocale(LC_ALL, curLocale.c_str());

        return result;
    }

    std::string GBK2UTF8(const std::string& strGBK)
    {
#ifdef WIN32
        int len = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);
        wchar_t* wstr = new wchar_t[len + 1];
        memset(wstr, 0, len + 1);
        MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, wstr, len);
        len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
        char* str = new char[len + 1];
        memset(str, 0, len + 1);
        WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
        std::string strTemp = str;
        if (wstr) delete[] wstr;
        if (str) delete[] str;
        return strTemp;
#else
        std::wstring w_str = String2Wstring(strGBK, "zh_CN.18030");
        return Wstring2String(w_str, "zh_CN.utf8");
#endif
    }

    std::string UTF82GBK(const std::string& strUTF8)
    {
#ifdef WIN32
        int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, NULL, 0);
        wchar_t* wszGBK = new wchar_t[len + 1];
        memset(wszGBK, 0, len * 2 + 2);
        MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, wszGBK, len);
        len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
        char* szGBK = new char[len + 1];
        memset(szGBK, 0, len + 1);
        WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
        std::string strTemp(szGBK);
        if (wszGBK) delete[] wszGBK;
        if (szGBK) delete[] szGBK;
        return strTemp;
#else
        std::wstring w_str = String2Wstring(strUTF8, "zh_CN.utf8");
        return Wstring2String(w_str, "zh_CN.gb18030");
#endif
    }

    std::string wstring2string(const std::wstring & str, const std::string & locale_str)
    {
        typedef deletable_facet<std::codecvt_byname<wchar_t, char, std::mbstate_t>> F;
        std::wstring_convert<F> conv(new F(locale_str));
        return conv.to_bytes(str);
    }

    std::wstring string2wstring(const std::string & str, const std::string & locale_str)
    {
        typedef deletable_facet<std::codecvt_byname<wchar_t, char, std::mbstate_t>> F;
        std::wstring_convert<F> conv(new F(locale_str));
        return conv.from_bytes(str);
    }

    std::string unicode_to_utf8(const std::wstring & str)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        return conv.to_bytes(str);
    }

    std::wstring utf8_to_unicode(const std::string & str)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        return conv.from_bytes(str);
    }

    std::string unicode_to_gb18030(const std::wstring & str)
    {
#ifdef WIN32
        return wstring2string(str, ".54936");
#else
        return wstring2string(str, "zh_CN.gb18030");
#endif
    }

    std::wstring gb18030_to_unicode(const std::string & str)
    {
#ifdef WIN32
        return string2wstring(str, ".54936");
#else
        return string2wstring(str, "zh_CN.gb18030");
#endif
    }

    std::string utf8_to_gb18030(const std::string & str)
    {
        return unicode_to_gb18030(utf8_to_unicode(str));
    }

    std::string gb18030_to_utf8(const std::string & str)
    {
        return unicode_to_utf8(gb18030_to_unicode(str));
    }

    bool isInt(const std::string & str)
    {
        // TODO
        return false;
    }

    bool isFloat(const std::string & str)
    {
        // TODO
        return false;
    }

    bool isBool(const std::string & str)
    {
        // TODO
        return false;
    }

    bool RegexSearch(const std::string & str, const std::string & reg_str)
    {
        try
        {
            const std::regex pattern(reg_str);
            return std::regex_search(str, pattern);
        }
        catch (const std::exception&)
        {
            std::cerr << "regex error: " << reg_str << std::endl;
            return false;
        }
    }

    bool RegexSearch(const std::wstring & wstr, const std::wstring & wreg_str)
    {
        try
        {
            const std::wregex pattern(wreg_str);
            return std::regex_search(wstr, pattern);
        }
        catch (const std::exception&)
        {
            std::wcerr << "regex error: " << wreg_str << std::endl;
            return false;
        }
    }

    bool RegexSearch(const std::string & str, std::vector<std::string>& result, const std::string & reg_str)
    {
        try
        {
            const std::regex pattern(reg_str);
            std::smatch mat;
            if (std::regex_search(str, mat, pattern))
            {
                result.clear();
                result.reserve(mat.size());
                result.insert(result.begin(), mat.begin(), mat.end());
                return true;
            }
        }
        catch (const std::exception&)
        {
            std::cerr << "regex error: " << reg_str << std::endl;
        }
        return false;
    }

    bool RegexSearch(const std::wstring & wstr, std::vector<std::wstring>& result, const std::wstring & wreg_str)
    {
        try
        {
            const std::wregex pattern(wreg_str);
            std::wsmatch mat;
            if (std::regex_search(wstr, mat, pattern))
            {
                result.clear();
                result.reserve(mat.size());
                result.insert(result.begin(), mat.begin(), mat.end());
                return true;
            }
        }
        catch (const std::exception&)
        {
            std::wcerr << "regex error: " << wreg_str << std::endl;
        }
        return false;
    }

    bool RegexMatch(const std::string & str, const std::string & reg_str)
    {
        try
        {
            const std::regex pattern(reg_str);
            return std::regex_match(str, pattern);
        }
        catch (const std::exception&)
        {
            std::cerr << "regex error: " << reg_str << std::endl;
            return false;
        }
    }

    bool RegexMatch(const std::wstring & wstr, const std::wstring & wreg_str)
    {
        try
        {
            const std::wregex pattern(wreg_str);
            return std::regex_match(wstr, pattern);
        }
        catch (const std::exception&)
        {
            std::wcerr << "regex error: " << wreg_str << std::endl;
            return false;
        }
    }

    bool RegexMatch(const std::string & str, std::vector<std::string>& result, const std::string & reg_str)
    {
        try
        {
            const std::regex pattern(reg_str);
            std::smatch mat;
            if (std::regex_match(str, mat, pattern))
            {
                result.clear();
                result.reserve(mat.size());
                result.insert(result.begin(), mat.begin(), mat.end());
                return true;
            }
        }
        catch (const std::exception&)
        {
            std::cerr << "regex error: " << reg_str << std::endl;
        }
        return false;
    }

    bool RegexMatch(const std::wstring & wstr, std::vector<std::wstring>& result, const std::wstring & wreg_str)
    {
        try
        {
            const std::wregex pattern(wreg_str);
            std::wsmatch mat;
            if (std::regex_match(wstr, mat, pattern))
            {
                result.clear();
                result.reserve(mat.size());
                result.insert(result.begin(), mat.begin(), mat.end());
                return true;
            }
        }
        catch (const std::exception&)
        {
            std::wcerr << "regex error: " << wreg_str << std::endl;
        }
        return false;
    }

    std::string RegexReplace(const std::string & str, const std::string & reg_str, const std::string & sub_str)
    {
        std::regex pattern(reg_str);
        return regex_replace(str, pattern, sub_str);
    }

    std::wstring RegexReplace(const std::wstring & wstr, const std::wstring & wreg_str, const std::wstring & wsub_str)
    {
        std::wregex pattern(wreg_str);
        return regex_replace(wstr, pattern, wsub_str);
    }

    std::string toLower(const std::string & str)
    {
        std::istringstream is(str);
        std::string lower_case_;
        char c;
        while (is.get(c)) {
            if (isalpha(c)) {
                if (isupper(c)) {
                    c = static_cast<char> (std::tolower(c));
                    lower_case_ += c;
                }
                else
                    //is lower;
                    lower_case_ += c;
            }
            else
                //is not alphabit;
                lower_case_ += c;
        }
        return lower_case_;
    }

    std::string toUpper(const std::string & str)
    {
        std::istringstream is(str);
        std::string upper_case_;
        char c;
        while (is.get(c)) {
            if (isalpha(c)) {
                if (islower(c)) {
                    c = static_cast<char> (std::toupper(c));
                    upper_case_ += c;
                }
                else
                    // is upper;
                    upper_case_ += c;
            }
            else
                // is not alphabit;
                upper_case_ += c;
        }
        return upper_case_;
    }

    enum CHARCLASS { CC_SBCSPACE, CC_DBCSPACE, CC_SBCCHAR, CC_DBCCHAR, CC_SBCCHINESE };

    CHARCLASS GetCharClass(unsigned char c1, unsigned char c2)
    {
        if (c1 == 161 && c2 == 161)
        {
            return CC_SBCSPACE;	//全角空格A1A1
        }
        else if (c1 >= 161 && c1 <= 169)
        {
            return CC_SBCCHAR;//全角字符
        }
        else if (c1 <= 127)
        {
            return (c1 != 32)
                ? CC_DBCCHAR //英文字母
                : CC_DBCSPACE;//半角空格32
        }
        else
        {
            return CC_SBCCHINESE;//汉字
        }
    }

    std::string SBC2DBC(const std::string & SBC)
    {
        int nCount = SBC.length();

        unsigned char cTmp[2] = { 0, 0 }; // 第一字节第二字节		
        int nPos = 0;

        char* pcResult = new char[nCount + 1]; // 转换后字符串,若全部是半角则需要多一位

        int nIndex = 0;
        for (nIndex = 0;
            nIndex < nCount - 1;    // 最后保留一个，避免取两字符越界
            nIndex += 2)            // 移到下一个全角字符
        {
            cTmp[0] = SBC[nIndex];      // 取前一字节
            cTmp[1] = SBC[nIndex + 1];  // 取第二字节	

            // 半角字符(33-126)与全角的对应关系是：首字节0xA3，末字节相差128 
            switch (GetCharClass(cTmp[0], cTmp[1])) // 获取分类
            {
            case CC_SBCCHAR://全角字符
            {
                if (cTmp[0] == 0xA3 && cTmp[1] == 0xA4/*￥*/)
                {
                    pcResult[nPos++] = cTmp[0];
                    pcResult[nPos++] = cTmp[1];
                }
                else if (cTmp[0] == 0xA3 && cTmp[1] >= 0xA1 && cTmp[1] < 0xFE)  // 仅33-126可视
                {
                    pcResult[nPos++] = cTmp[1] - 128;
                }
                //1.0.1 特殊处理GB18030 一区A1区全角
                else if (cTmp[0] == 0xA1 && cTmp[1] == 0xAA) // —
                {
                    pcResult[nPos++] = '-';
                }
                else if (cTmp[0] == 0xA1 && cTmp[1] == 0xAB) //～
                {
                    pcResult[nPos++] = '~';
                }
                else if (cTmp[0] == 0xA1)
                {
                    switch (cTmp[1])
                    {
                    case 0xAA: pcResult[nPos++] = '-'; break;
                    case 0xAB: pcResult[nPos++] = '~'; break;
                    case 0xA3: pcResult[nPos++] = '.'; break;
                    case 0xAE: pcResult[nPos++] = '\''; break;
                    case 0xAF: pcResult[nPos++] = '\''; break;
                    case 0xB0: pcResult[nPos++] = '"'; break;
                    case 0xB1: pcResult[nPos++] = '"'; break;
                    case 0xE7: pcResult[nPos++] = '$'; break;
                    default: // 无对应半角的不处理
                    {
                        pcResult[nPos++] = cTmp[0];
                        pcResult[nPos++] = cTmp[1];
                    }
                    }
                }
                //////////////////////////////////////////////////////////////////////////
                else // 无对应半角的不处理
                {
                    pcResult[nPos++] = cTmp[0];
                    pcResult[nPos++] = cTmp[1];
                }
                break;
            }
            case CC_SBCSPACE:   // 全角空格
                pcResult[nPos++] = ' ';
                break;
            case CC_SBCCHINESE://汉字
            {
                pcResult[nPos++] = cTmp[0];
                pcResult[nPos++] = cTmp[1];
            }
            break;
            case CC_DBCCHAR:    // 半角字符			
            case CC_DBCSPACE:   // 半角空格
            {
                pcResult[nPos++] = cTmp[0];
                nIndex--;   // 半角只有一个字符
            }
            break;
            default:
                pcResult[nPos++] = cTmp[0];
                pcResult[nPos++] = cTmp[1];
            }
        } // end for (int nIndex = 0; nIndex < nCount - 1; nIndex++) //最后一个未检查

        if (nIndex == nCount - 1) // 最后一个是半角字符,未处理, 最后一个是全角时 i == nCount
        {
            pcResult[nPos++] = SBC[nIndex];
        }

        pcResult[nPos++] = 0;   // 加0

        std::string strResult(pcResult);    // 转换结果

        delete[] pcResult;

        return strResult;
    }

}


