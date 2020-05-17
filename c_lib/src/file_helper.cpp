#include "file_helper.h"
#include <fstream>

using namespace std;

namespace htk
{
    const std::string NOTE_SYMBOL = "#";
    const char SimpleConfig::COMMENT_CHAR = '#';
    const std::string SimpleConfig::WHITE_SPACES = "\n\r\t\v\f ";
    
    bool SimpleReadCSV(const std::string & file_path, std::vector<std::vector<std::string> >& res, const std::string & delimiter)
    {
        if (file_path.empty())
        {
            return false;
        }

        std::ifstream reader(file_path.c_str());
        if (!reader.is_open())
        {
            return false;
        }

        std::string line;
        res.clear();
        while (reader.good() && !reader.eof())
        {
            getline(reader, line);

            // TODO: trim process

            line = trim(line, "\r");
            if (line.empty() || startswith(line, NOTE_SYMBOL))
            {
                continue;
            }

            std::vector<std::string> split_item = split(line, delimiter);
            if (split_item.empty())
            {
                continue;
            }
            res.push_back(split_item);
        }
        reader.close();
        return true;
    }

   
    bool SimpleConfig::LoadConfigFile(const std::string & filename)
    {
        ifstream *reader = new ifstream(filename.c_str());
        if (!reader) return false;

        _data_.clear();
        std::string line, key, value;
        while (getline(*reader, line))
        {
            line = trim(line, "\r");
            if (line.empty() || line.find(COMMENT_CHAR) == 0) continue;

            size_t equal_pos = line.find('=');
            if (equal_pos == std::string::npos) continue;
            else
            {
                key = trim(line.substr(0, equal_pos), WHITE_SPACES);
                value = trim(line.substr(equal_pos + 1, line.length() - equal_pos), WHITE_SPACES);
                _data_.insert(config_data_t::value_type(key, value));
            }
        }
        return true;
    }

}

