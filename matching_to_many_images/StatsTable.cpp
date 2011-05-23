// TODO: this is very specific for use with SURFMatcher right now

#include <fstream>
#include <string>

#include "util.h"
#include "StatsTable.h"

StatsTable::StatsTable() {
	struct tm timeinfo = Util::GetTimeInfo();
	char buffer[80];
	strftime(buffer, 80, "MATCH-%m-%d-%Y.html", &timeinfo);

	file_ = new std::ofstream(buffer, std::ios::out);
	(*file_) << "<html><head>Match stats</head><body>\n";
	BeginTable();
}

StatsTable::~StatsTable() {
	EndTable();
	if (file_) {
		(*file_) << "\n</body></html>\n";
		file_->close();
		delete file_;
		file_ = NULL;
	}
}

void StatsTable::BeginTable() {
	(*file_) << "<table>\n\t<tr><th>filename</th><th>match %</th><th>match with</th><th>time</th></tr>\n";
}

void StatsTable::WriteRow(string filename, double match_percent, string match_with, double time) {
	(*file_) << "\t<tr><td>" << filename << "</td><td>" << match_percent << "</td><td>" << match_with << "</td><td>" << time << "<td></td></tr>\n";
}

void StatsTable::EndTable() {
	(*file_) << "</table>\n";
}