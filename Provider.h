#pragma once

#include <QThread>

#include "Chunk.h"

class RandomProvider : public QThread
{
public:

    RandomProvider(ChunkQueue & chunk_queue) : QThread(nullptr), chunkQueue(chunk_queue)
    {
    }

    void run();

private:

    ChunkQueue & chunkQueue;
};

class FileProvider : public QThread
{
public:

	FileProvider(ChunkQueue & chunk_queue) : QThread(nullptr), chunkQueue(chunk_queue)
	{
	}

	void run();

private:

	ChunkQueue & chunkQueue;
};
