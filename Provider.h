#pragma once

#include <QThread>

#include "Chunk.h"

class Provider : public QThread
{
public:

    Provider(ChunkQueue & chunk_queue) : QThread(nullptr), chunkQueue(chunk_queue)
    {
    }

    void run();

private:

    ChunkQueue & chunkQueue;
};
