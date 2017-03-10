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

		if (p_prev_chunk != nullptr)
		{
			p_chunk = *(++ChunkList::iterator(p_prev_chunk));

			++p_prev_chunk->readCount;

			assert(p_prev_chunk->readCount <= ReaderCount);

			//all the readers unlocked this chunk
			if (p_prev_chunk->readCount == ReaderCount)
			{
				p_prev_chunk->exclude();

				freeChunks.push_back(p_prev_chunk);

				--allocatedChunkCount;

				chunk_removed = true;

				std::ostringstream out;
				out << "chunk removed " << p_prev_chunk->Buffer[0] << std::endl;
				std::cout << out.str();
			}
		}
		else
		{
			p_chunk = usedChunks.front();
		}

		//Check if there are available chunks, if not get a chunk from pending list.
		if (ChunkList::iterator(p_chunk) == usedChunks.end())
		{
			p_chunk = nullptr;
			
			while (pendingChunks.empty() && !writeCompleted)
			{
				//allocatedChunkCount can differ from zero at this point, but the list is still empty

				chunkAdded.wait(lock);
			}

			if (!pendingChunks.empty())
			{
				p_chunk = pendingChunks.pop_front();
				
				assert(p_chunk->readCount < ReaderCount);

				assert(!p_chunk->included());

				usedChunks.push_back(p_chunk);
			}
		}
	}

	if (chunk_removed)
	{
		//unlocking is done before notifying, to avoid waking up the waiting thread only to block again (some optimization)
		chunkRemoved.notify_one();
	}

	{
		std::ostringstream out;
		out << "chunk returned " << (p_chunk != nullptr ? p_chunk->Buffer[0] : -1) << std::endl;
		std::cout << out.str();
	}

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

		pendingChunks.push_back(p_chunk);

		std::ostringstream out;
		out << "chunk added " << p_chunk->Buffer[0] << std::endl;
		std::cout << out.str();
	}

	chunkAdded.notify_all();
}

void ChunkQueue::Complete()
{
	{
		std::unique_lock<std::mutex> lock(queueMutex);

		writeCompleted = true;

		std::cout << "completed" << std::endl;
	}

	chunkAdded.notify_all();
}
