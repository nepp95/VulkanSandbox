#pragma once

#include <functional>
#include <queue>

using RenderCommand = std::function<void()>;

class RenderCommandQueue
{
public:
	RenderCommandQueue(RenderCommandQueue&&) = delete;
	RenderCommandQueue(const RenderCommandQueue&) = delete;
	RenderCommandQueue& operator=(const RenderCommandQueue&) = delete;

	static void AddCommand(RenderCommand fn);
	static void Execute();

	static RenderCommandQueue& Get() { return s_instance; }

private:
	RenderCommandQueue() = default;

	std::queue<RenderCommand> m_commandQueue;
	uint32_t m_commandCount = 0;

	static RenderCommandQueue s_instance;
};