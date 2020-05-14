#ifndef FILE_HELPERS_H
#define FILE_HELPERS_H

#include <string>
#include <vector>
#include <map>
#include "str_helper.h"

namespace htk
{
    bool SimpleReadCSV(
        const std::string& file_path,
        std::vector< std::vector<std::string> >& res,
        const std::string& delimiter
    );

    class SimpleConfig
    {
        static const char COMMENT_CHAR;
        static const std::string WHITE_SPACES;

        typedef std::map<std::string, std::string> config_data_t;

    public:
        SimpleConfig() {}
        SimpleConfig(const std::string& filename)
        {
            LoadConfigFile(filename);
        }
        ~SimpleConfig() {}

        template<typename T>
        T get(const std::string& key)
        {
            config_data_t::iterator it = _data_.find(key);
            if (it == _data_.end())
            {
                printf("error: can`t find config key \"%s\"", key.c_str());
                exit(404);
            }
            return ConvertFromString<T>(it->second);
        }

        template<typename T>
        T get(const std::string& key, const T& default_value)
        {
            config_data_t::iterator it = _data_.find(key);
            if (it == _data_.end())
            {
                return default_value;
            }
            return ConvertFromString<T>(it->second);
        }

        template<typename T>
        bool get(const std::string& key, T& rev)
        {
            config_data_t::iterator it = _data_.find(key);
            if (it == _data_.end())
            {
                return false;
            }
            else
            {
                rev = ConvertFromString<T>(it->second);
                return true;
            }
        }

        template<typename T>
        bool get(const std::string& key, const T& default_value, T& rev)
        {
            config_data_t::iterator it = _data_.find(key);
            if (it == _data_.end())
            {
                rev = default_value;
            }
            else
            {
                get(key, rev);
            }
            return true;
        }

        bool LoadConfigFile(const std::string& filename);

        const std::map<std::string, std::string>& GetData()
        {
            return _data_;
        }

    private:

        config_data_t _data_;
    };
}

#endif // FILE_HELPERS_H
