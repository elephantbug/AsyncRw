#include <assert.h>

#include <iostream>
#include <sstream>

#include "Provider.h"

void RandomProvider::run()
{
    Chunk * p_chunk = nullptr;

    size_t buf_pos = 0;

    for (size_t i = 0; i < 10000; ++i)
    {
        if (buf_pos == Chunk::BufferSize)
        {
            chunkQueue.PushChunk(p_chunk);

            p_chunk = nullptr;
        }

        if (p_chunk == nullptr)
        {
            p_chunk = chunkQueue.AllocateChunk();

            buf_pos = 0;
        }

        p_chunk->Buffer[buf_pos++] = (int)i;

        p_chunk->Length = buf_pos;
    }

    //push the last incomplete buffer
    chunkQueue.PushChunk(p_chunk);

    chunkQueue.Complete();
}

void FileProvider::run()
{
	Chunk * p_chunk = nullptr;

	size_t buf_pos = 0;

	int n;

	while (std::cin >> n)
	{
		if (buf_pos == Chunk::BufferSize)
		{
			chunkQueue.PushChunk(p_chunk);

			p_chunk = nullptr;
		}

		if (p_chunk == nullptr)
		{
			p_chunk = chunkQueue.AllocateChunk();

			buf_pos = 0;
		}

		p_chunk->Buffer[buf_pos++] = n;

		p_chunk->Length = buf_pos;
	}
	
	//push the last incomplete buffer
	if (p_chunk != nullptr)
	{
		chunkQueue.PushChunk(p_chunk);
	}

	chunkQueue.Complete();
}
