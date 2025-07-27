#include "command.hpp"

Command::Command() {}

void Command::parse(const std::string& command) {
	std::vector<std::string> components;
	int fromPos = -1;
	bool isCondition = false;
	bool isConditionStart = false;
	
	if (command == ".dbinfo")
	{
		type = INFO;
		return;
	}
	if (command == ".tables") {
		type = TABLES;
		return ;
	}

	components = Util::split(command, " ");

	if (components.size() < 4 || Util::toUpper(components[0]) != "SELECT") {
		type = UNKNOWN;
		return ;
	}

	type = QUERY;
	queryType = SIMPLE_SELECT;

	// parse select command
	if (Util::toUpper(components[1]) == "COUNT(*)") {
		queryType = COUNT;
	}

	for (size_t i = 1; i < components.size(); i++) {
		components[i] = Util::toLower(components[i]);

		if (Util::toUpper(components[i]) == "FROM") {
			fromPos = i;

			// right after SELECT
			if (fromPos < 2) {
				type = UNKNOWN;
				return ;
			}
		}

		else if (Util::toUpper(components[i]) == "WHERE") {
			queryType = CONDITION_SELECT;
			isCondition = true;
			isConditionStart = true;
			if (fromPos == -1 || i - fromPos < 2 || (components.size() - i) < 3) {
				type = UNKNOWN;
				return ;
			}
		}
		else {
			if (isCondition) {
				if (components[i] == "=")
					continue ;
				if (isConditionStart) {
					condition.first = components[i];
					isConditionStart = false;
				} else {
					if (!condition.second.empty())
						condition.second += " ";
					condition.second += Util::trim(components[i], "'");
				}
			} else {
				if (fromPos == -1) {
					columns.push_back(Util::rtrim(components[i], ", "));
				} else {
					table = Util::toLower(components[i]);
				}
			}
		}
	}
}

Command::~Command() {}
