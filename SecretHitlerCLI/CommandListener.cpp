#include "CommandListener.h"

void CommandListener::submit(std::string msg)
{
	queue.push(msg);
}

void CommandListener::loop()
{
}
