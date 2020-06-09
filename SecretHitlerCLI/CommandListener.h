#pragma once

#include <string>

#include "Util.h"

class CommandListener
{
	ThreadedQueue<std::string> q;
public:
	void submit(std::string msg);
	virtual void loop();
};
