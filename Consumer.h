#pragma once

#include <QThread>
#include <assert.h>

#include "Chunk.h"

template <class Processor>
class Comsumer : public QThread
{
public:

    Comsumer(ChunkQueue & chunk_queue, Processor & proc = Processor()) : QThread(nullptr), chunkQueue(chunk_queue), Proc(proc)
    {
        chunk_queue.AttachReader();
    }

    void run()
    {
        Chunk * p_chunk = nullptr;

        while ((p_chunk = chunkQueue.ReadChunk(p_chunk)) != nullptr)
        {
            for (size_t i = 0; i < p_chunk->Length; ++i)
            {
                Proc.Process(p_chunk->Buffer[i]);
            }
        }
    }

	const Processor & GetProcessor() const
	{
		return Proc;
	}

private:

    ChunkQueue & chunkQueue;

    Processor & Proc;
};

//does nothing
class FakeProcessor
{
public:

    void Process(int)
    {
    }
};

//prints the numbers to std out
class OutputProcessor
{
public:

    void Process(int n)
    {
        std::ostringstream out;
        out << n << std::endl;
        std::cout << out.str();
    }
};

class SequentialProcessor
{
public:

    void Process(int n)
    {
        assert(n == Cur++);
    }

private:

    int Cur = 0;
};

class AdditionProcessor
{
public:

	void Process(int n)
	{
		Result += n;
	}

	int Result = 0;
};

class SubtractionProcessor
{
public:

	void Process(int n)
	{
		if (first)
		{
			Result = n;

			first = false;
		}
		else
		{
			Result -= n;
		}
	}

	int Result = 0;

private:

	bool first = true;
};

class XorProcessor
{
public:

	void Process(int n)
	{
		Result ^= n;
	}

	int Result = 0;
};
