#include <QCoreApplication>
#include <QTimer>
#include <QThread>

#include <iostream>
#include <sstream>
#include <assert.h>

#include "Provider.h"
#include "Consumer.h"

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
