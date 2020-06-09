#include "CommandListener.h"

void CommandListener::submit(std::string msg)
{
	q.push(msg);
}

void CommandListener::loop()
{
}
