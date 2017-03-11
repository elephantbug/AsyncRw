#include <assert.h>

#include <Chunk.h>

#include <iostream>
#include <sstream>

ChunkQueue::ChunkList::~ChunkList()
{
	iterator i = begin();

	while (i != end())
	{
		delete *(i++);
	}
}

Chunk * ChunkQueue::ReadChunk(Chunk * p_prev_chunk)
{
	bool chunk_removed = false;
	
	Chunk * p_chunk;

	{
		std::unique_lock<std::mutex> lock(queueMutex);

		ChunkList::iterator i_prev = usedChunks.end();
		
		if (p_prev_chunk != nullptr)
		{
			i_prev = ChunkList::iterator(p_prev_chunk);
		}

		ChunkList::iterator i_cur = i_prev;
		++i_cur;

		//Check if there are available chunks, if not get a chunk from pending list.
		while (i_cur == usedChunks.end() && !writeCompleted)
		{
			//allocatedChunkCount can differ from zero at this point, but the list is still empty

			chunkAdded.wait(lock);

			i_cur = i_prev;
			++i_cur;
		}

		if (i_cur != usedChunks.end())
		{
			p_chunk = *i_cur;
			
            assert(p_chunk->readCount < (int)ReaderCount);

			assert(p_chunk->included());
		}
		else
		{
			assert(writeCompleted);
			
			p_chunk = nullptr;
		}

		//remove p_prev_chunk if it is not used anymore
		if (p_prev_chunk != nullptr)
		{
			++p_prev_chunk->readCount;

            assert(p_prev_chunk->readCount <= (int)ReaderCount);

            if (p_prev_chunk->readCount == (int)ReaderCount)
			{
				//we are deleting the first element
				//assert(++ChunkList::reverse_iterator(p_prev_chunk) == usedChunks.rend());
				//assert(p_prev_chunk == usedChunks.front());

				p_prev_chunk->exclude();

				freeChunks.push_back(p_prev_chunk);

				--allocatedChunkCount;

				chunk_removed = true;

				//std::ostringstream out;
				//out << "chunk removed " << p_prev_chunk->Buffer[0] << std::endl;
				//std::cout << out.str();

				//p_chunk became first
				//assert(p_chunk == usedChunks.front());
			}
		}
	}

	if (chunk_removed)
	{
		//unlocking is done before notifying, to avoid waking up the waiting thread only to block again (some optimization)
		chunkRemoved.notify_one();
	}

	//{
	//	std::ostringstream out;
	//	out << "chunk returned " << (p_chunk != nullptr ? p_chunk->Buffer[0] : -1) << std::endl;
	//	std::cout << out.str();
	//}

	return p_chunk;
}

Chunk * ChunkQueue::AllocateChunk()
{
	std::unique_lock<std::mutex> lock(queueMutex);

	while (allocatedChunkCount == MaxNumberOfChunks)
	{
		//we write faster than read
		chunkRemoved.wait(lock);
	}

	Chunk * p_chunk = freeChunks.front();

	if (ChunkList::iterator(p_chunk) == freeChunks.end())
	{
		p_chunk = new Chunk();
	}
	else
	{
		p_chunk->exclude();
	}

	//initialize the fields
	p_chunk->readCount = 0;
	p_chunk->Length = 0;

	++allocatedChunkCount;

	return p_chunk;
}

void ChunkQueue::PushChunk(Chunk * p_chunk)
{
	{
		std::unique_lock<std::mutex> lock(queueMutex);

		usedChunks.push_back(p_chunk);

		//std::ostringstream out;
		//out << "chunk added " << p_chunk->Buffer[0] << std::endl;
		//std::cout << out.str();
	}

	chunkAdded.notify_all();
}

void ChunkQueue::Complete()
{
	{
		std::unique_lock<std::mutex> lock(queueMutex);

		writeCompleted = true;

		//std::cout << "completed" << std::endl;
	}

	chunkAdded.notify_all();
}
