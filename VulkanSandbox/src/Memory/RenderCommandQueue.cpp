#include "RenderCommandQueue.h"

RenderCommandQueue RenderCommandQueue::s_instance;

void RenderCommandQueue::AddCommand(RenderCommand fn)
{
	Get().m_commandQueue.push(fn);
	Get().m_commandCount++;
}

void RenderCommandQueue::Execute()
{
	for (uint32_t i = 0; i < Get().m_commandCount; i++)
	{
		Get().m_commandQueue.front()();
		Get().m_commandQueue.pop();
	}

	Get().m_commandCount = 0;
}
