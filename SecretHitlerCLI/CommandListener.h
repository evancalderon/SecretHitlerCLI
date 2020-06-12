#pragma once

#include <string>
#include <unordered_map>
#include <functional>

#include "Util.h"

typedef std::vector<std::string> StringList;
typedef std::function<void(StringList)> CommandDelegate;

class CommandListener
{
	std::unordered_map<std::string, CommandDelegate> commands;
	bool sameState = false;
protected:
	ThreadedQueue<std::string> queue;
public:
	void submit(std::string msg);
	virtual void loop();

	void addCommand(std::string cmd, CommandDelegate fn);
	void execCommand();
};
