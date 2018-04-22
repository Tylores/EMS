#include "utility.h"
using namespace std;

map<string, string> Utility::setConfig(string t_fileName)
{
    map<string, string> configMap;
    string line;
    string key;
    string value;
    ifstream config(t_fileName);

    if (config.is_open()){
        while (getline(config,line)){
            if (line.front() == '#') continue;
            istringstream is_line(line);
            if (getline(is_line,key,'=')){
                if (getline(is_line, value)){
                    configMap.insert(pair<string, string>(key,value));
                } //END GET VALUE
            } //END GET LINES
        } //END WHILE LINE
    } else {
        cout << "Unable to open file " << t_fileName << endl;
    }//END IF/ELSE FILE IPEN

    return configMap;
} //END SET CONFIG

vector<vector<string>> Utility::readCSV(string t_fileName)
{
    string line;
    string cell;
    vector<vector<string>> csvMap;

    ifstream file(t_fileName);
    if (file.is_open()){
        cout << "\tInitializing modBus register map from " << t_fileName << endl;
        while (getline(file,line)){
            vector<string> csvRow;
            istringstream s(line);
            while (getline(s,cell,',')){
                csvRow.push_back(cell);
            } //END GET COLUMN
            csvMap.push_back(csvRow);
        } //END GET ROW
    file.close();
    } else {
        cerr << "Unable to open register initialization file" << t_fileName << endl;
    } //END IF/ELSE OPEN
    return csvMap;
} //END READ CSV


void Utility::writeCSV(vector<vector<string>> t_data, string t_fileName)
{
    int ncol = t_data[0].size();
    int nrow = t_data.size();

    ofstream file(t_fileName.c_str());
    if (file.is_open()){
        cout << "\tWriting to file: " << t_fileName << endl;
        for(int i = 0; i < nrow; i++){
            for(int j = 0; j < ncol; j++){
                file << t_data[i][j] << ", ";
            }
            file << endl;
        }

        file.close();
    } else {
        cout << "Unable to open file" << endl;
    }

} //END READ CSV

vector<vector<string>> Utility::filterData(vector<vector<string>> t_data, string t_header, string t_criterion)
{
    vector<vector<string>> filter;
    vector<string> header = t_data[0];
    int nrow = t_data.size();

    ptrdiff_t col = find(header.begin(), header.end(), t_header) - header.begin();
    for(int i = 1; i < nrow; i++){
        if (t_data[i][col] == t_criterion){
            filter.push_back(t_data[i]);
        }
    }
    return filter;
} //END FILTER DATA

void Utility::writeText(string t_fileName, string t_message)
{
    ofstream file(t_fileName,ios::app);
    if (file.is_open())
    {
        file << t_message << "\n";
        file.close();
    } else {
        cout << "Unable to open file" << endl;
    }
} //END WRITE TEXT

string Utility::getDateTime()
{
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer,sizeof(buffer),"%d-%b-%Y %H:%M:%S",timeinfo);
    return buffer;
} //END GET DATE/TIME

vector<string> Utility::stringDelim(string t_string, char t_delim)
{
    string temp;
    vector<string> deliminated;

    istringstream s(t_string);
    while (getline(s,temp, t_delim)){
        deliminated.push_back(temp);
    } //END GET COLUMN

    return deliminated;
} //END READ CSV

vector<double> Utility:: ImportSchedule(string tFileName)
{
    string line;
    vector<double> schedule;

    ifstream file(tFileName.c_str());
    if (file.is_open()){

        while (getline(file,line)){
            schedule.push_back(stod(line));
        } //while

    file.close();
    } else {
        cerr << "Unable to open register initialization file" << tFileName << endl;
    } //if/else

    return schedule;
} //END IMPORT SCHEDULE
