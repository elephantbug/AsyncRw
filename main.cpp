#include <QCoreApplication>
#include <QTimer>
#include <QThread>

#include <iostream>
#include <sstream>
#include <assert.h>

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

void Provider::run()
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

class MainTask : public QObject
{
	Q_OBJECT

public:

	MainTask(QObject *parent = nullptr) : QObject(parent) {}

public slots :

	void run();

signals:

	void finished();
};

void MainTask::run()
{
	ChunkQueue chunk_queue;
	
	Provider producer(chunk_queue);
	Comsumer<SequentialProcessor> comsumer1(chunk_queue);
	Comsumer<SequentialProcessor> comsumer2(chunk_queue);
	Comsumer<OutputProcessor> comsumer3(chunk_queue);

	producer.start();
	comsumer1.start();
	comsumer2.start();
	comsumer3.start();

	producer.wait();
	comsumer1.wait();
	comsumer2.wait();
	comsumer3.wait();

	emit finished();
}

#include "main.moc"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	// MainTask parented to the application so that it
	// will be deleted by the application.
	MainTask *task = new MainTask(&a);

	// This will cause the application to exit when
	// the task signals finished.    
	QObject::connect(task, SIGNAL(finished()), &a, SLOT(quit()));

	// This will run the task from the application event loop.
	QTimer::singleShot(0, task, SLOT(run()));

	return a.exec();
}
