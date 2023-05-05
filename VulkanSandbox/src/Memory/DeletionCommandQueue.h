#pragma once

#include <deque>
#include <functional>

using DeleteCommand = std::function<void()>;

class DeletionCommandQueue
{
public:
	DeletionCommandQueue(DeletionCommandQueue&&) = delete;
	DeletionCommandQueue(const DeletionCommandQueue&) = delete;
	DeletionCommandQueue& operator=(const DeletionCommandQueue&) = delete;

	static void AddCommand(DeleteCommand fn);
	static void Execute();

	static DeletionCommandQueue& Get() { return s_instance; }

private:
	DeletionCommandQueue() = default;

	std::deque<DeleteCommand> m_commandQueue;
	uint32_t m_commandCount = 0;

	static DeletionCommandQueue s_instance;
};