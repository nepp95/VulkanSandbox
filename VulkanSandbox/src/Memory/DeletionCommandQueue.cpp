#include "DeletionCommandQueue.h"

DeletionCommandQueue DeletionCommandQueue::s_instance;

void DeletionCommandQueue::AddCommand(DeleteCommand fn)
{
	Get().m_commandQueue.push_back(fn);
	Get().m_commandCount++;
}

void DeletionCommandQueue::Execute()
{
	for (uint32_t i = 0; i < Get().m_commandCount; i++)
	{
		Get().m_commandQueue.back()();
		Get().m_commandQueue.pop_back();
	}

	Get().m_commandCount = 0;
}
