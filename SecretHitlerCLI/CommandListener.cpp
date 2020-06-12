#include <iostream>

#include "CommandListener.h"

void CommandListener::submit(std::string msg)
{
	queue.push(msg);
}

void CommandListener::loop()
{
}

void CommandListener::addCommand(std::string cmd, CommandDelegate fn)
{
	if (!sameState)
		commands[cmd] = fn;
}

void CommandListener::execCommand()
{
	std::string msg;
	StringList args;
	if (sameState = queue.pop(msg))
	{
		args = splitArgs(msg);
	}
	else
	{
		return;
	}

	auto cmd = commands.find(args.size() > 0 ? args[0] : "");
	if (cmd == commands.end())
	{
		std::cout << std::endl;
		std::cout << "Invalid command." << std::endl;
		std::cout << "Commands:" << std::endl;
		for (auto [name, fn] : commands)
		{
			std::cout << name << std::endl;
		}
		std::cout << std::endl;
	}
	else
	{
		cmd->second(args);
		commands.clear();
	}
}
