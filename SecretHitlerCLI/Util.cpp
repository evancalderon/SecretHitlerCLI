#include <string>

#include "Util.h"

std::vector<std::string> splitArgs(std::string msg)
{
	std::vector<std::string> args;

	if (msg.empty())
		return args;

	std::string current;
	bool qmode = false;
	msg += " ";

	for (int i = 0; i < msg.size(); i++)
	{
		char c = msg[i];
		if (c == '"')
		{
			qmode = !qmode;
			continue;
		}

		if (qmode || !std::isspace(c))
			current += c;
		else
		{
			if (!current.empty())
				args.push_back(current);
			current = "";
		}
	}

	return args;
}
