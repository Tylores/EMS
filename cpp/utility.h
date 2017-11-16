#ifndef UTILITY_H
#define UTILITY_H

#include <string>
#include <vector>
#include <map>

using std::vector;
using std::string;

class Utility
{
    public:
        vector<vector<string>> readCSV(string t_fileName);
	void writeCSV(vector<vector<string>> t_data, string t_fileName);
	vector<vector<string>>  filterData(vector<vector<string>> t_data, string t_header, string t_criterion);
	void writeText(string t_fileName, string t_message);
	string getDateTime();
        std::map<string, string> setConfig(string t_fileName);
	vector<string> stringDelim(string t_string, char t_delim);
    protected:
    private:
};

#endif // UTILITY_H
