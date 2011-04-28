#ifndef UTIL_H__
#define UTIL_H__

#include <time.h>
#include <vector>

using namespace std;

namespace Util {

void  GetFileNamesFromFile(const string& filename, string& dirName, vector<string>& trainFilenames);
struct tm GetTimeInfo();

}

#endif