#ifndef STATS_TABLE__
#define STATS_TABLE__

#include <iostream>

class StatsTable {
public:
	StatsTable();
	~StatsTable();

	void WriteRow(string filename, double match_percent, string match_with, double time);

protected:
	void BeginTable();
	void EndTable();

private:
	std::ofstream *file_;
};

#endif