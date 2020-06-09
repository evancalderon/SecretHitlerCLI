#pragma once

#include <queue>
#include <mutex>
#include <string>

template<typename T>
class ThreadedQueue
{
	mutable std::mutex mut;
	std::queue<T> q;
public:
	ThreadedQueue() = default;
	ThreadedQueue(ThreadedQueue const& o)
	{
		std::lock_guard l(mut);
		q = o.q;
	}

	void push(T v)
	{
		std::lock_guard l(mut);
		q.push(v);
	}

	bool pop(T& v)
	{
		std::lock_guard l(mut);
		if (q.empty())
			return false;

		v = q.front();
		q.pop();
		return true;
	}
};

std::vector<std::string> splitArgs(std::string msg);
