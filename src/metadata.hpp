#pragma once

# include <vector>
# include <map>
# include <utility>
# include <string>

struct Metadata {
	unsigned long pageSize;
	unsigned long numberOfTables;
	std::vector<std::pair<unsigned long, std::string>> tables;
	std::map<std::string, std::vector<std::string>> columns;
	std::map<std::string, std::pair<std::string, unsigned long>> indexes;
};