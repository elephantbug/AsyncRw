#pragma once

#include <mutex>
#include <condition_variable>

#include "Awl/QuickList.h"

//Locking and unlocking a std::mutex has a cost. In practice, it would probably be worthwhile to divide the buffer into chunks and to operate on chunks 
//instead of individual elements. The buffer size is also a parameter that must be selected carefully, based on experimentation.
class Chunk : public awl::quick_link<Chunk>
{
public:

	static const size_t BufferSize = 1000;
	
	//the number of times the Chunk was read
	int readCount = 0;
	
	//the number of elements actually written to the buffer
	size_t Length = 0;
	
	int Buffer[BufferSize];
};

class ChunkQueue
{
private:

	class ChunkList : public awl::quick_list<Chunk>
	{
	public:

		~ChunkList();
	};

public:

	//Returns the next chunk for reading
	Chunk * ReadChunk(Chunk * p_prev_chunk);

	//Allocates a chunk for writing
	Chunk * AllocateChunk();

	//Makes the written chunk available for reading
	void PushChunk(Chunk * p_chunk);

	void Complete();

	void AttachReader()
	{
		++ReaderCount;
	}

private:

	size_t ReaderCount = 0;
	static const size_t MaxNumberOfChunks = 10;

	std::mutex queueMutex;
	std::condition_variable chunkAdded;
	std::condition_variable chunkRemoved;

	//Chunks being processed
	ChunkList usedChunks;

	//The nuber of allocated chunks, but not the count of elements in usedChunks list.
	size_t allocatedChunkCount = 0;

	//We recycle freed queue elements to avoid superfluous dynamic memory allocation
	ChunkList freeChunks;

	bool writeCompleted = false;
};