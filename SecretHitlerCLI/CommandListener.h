#pragma once

#include <string>

#include "Util.h"

class CommandListener
{
protected:
	ThreadedQueue<std::string> queue;
public:
	void submit(std::string msg);
	virtual void loop();
};
