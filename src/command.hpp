#pragma once

# include <string>
# include <iostream>
# include <vector>
# include <utility>

# include "util.hpp"

enum CommandType {
	INFO,
	TABLES,
	QUERY,
	UNKNOWN
};

enum QueryType {
	COUNT,
	SIMPLE_SELECT,
	CONDITION_SELECT
};

class Command {
	public:
		Command();
		~Command();

		void parse(const std::string&);

		std::string table;
		std::vector<std::string> columns;
		std::pair<std::string, std::string> condition;
		CommandType type;
		QueryType queryType;
};