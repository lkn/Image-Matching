#include "util.h"
#include <iostream>
#include <fstream>
#include <string>

// Ex: fileName.txt contains:
//   1.png
//   2.png
// We return: <dirName/1.png, dirName/2.png>
void Util::GetFileNamesFromFile(const string& filename, string& dirName, vector<string>& trainFilenames )
{
    const char dlmtr = '/';

    trainFilenames.clear();

    ifstream file( filename.c_str() );
    if ( !file.is_open() )
        return;

    size_t pos = filename.rfind(dlmtr);
    dirName = pos == string::npos ? "" : filename.substr(0, pos) + dlmtr;
    while( !file.eof() )
    {
        string str; getline( file, str );
        if( str.empty() ) break;
        trainFilenames.push_back(str);
    }
    file.close();
}


struct tm Util::GetTimeInfo() {
  time_t rawtime;
  time(&rawtime);
  struct tm *timeinfo = localtime(&rawtime);
  return *timeinfo;
}
