/* main.h
 *
 */
#include <QCoreApplication>
#include <QThread>
#include <QEvent>
#include <QSemaphore>
#include <QTime>
#include <QSemaphore>
class WorkerThread;
class MainThread;
class CustomEvent;
class NernstSim;

class MainThread : public QCoreApplication {
	public:
		//-----------------------------------------------------------
		// Functions
		//-----------------------------------------------------------
		
		// Subclass QCoreApplication so we can override the
		// event processing.
		MainThread( int param_argc, char **param_argv ) :
			QCoreApplication( param_argc, param_argv ), 
			argc(param_argc), argv(param_argv){}
		

		// Initialize here rather in the constructor.  Makes life 
		// just a tiny bit easier.
		virtual void run();

		//-----------------------------------------------------------
		// Variables
		//-----------------------------------------------------------

		int argc;
		char **argv;
		struct options *o;
		NernstSim *s;
		
		// Assume that number of workers will be fetched off of the 
		// commmand line.
		int nWorkers;

		// Our worker threads.
		class WorkerThread **worker;

	private:
		int inCount[2];
		int outCount[2];
		QSemaphore *semaphore[2];
		QSemaphore *barrier[2];


	
};


class WorkerThread : public QThread {
	public:
		// What this ought to do is:
		// 1.  Provide the only constructor for class WorkerThread.
		// 2.  Set param_parent to 0 if it is not provided.
		// 3.  Pass param_parent along to the parent class (QThread).
		// 4.  Set this->id = param_id.
		int id; 
		WorkerThread(int param_id, QObject *param_parent=0) : 
			QThread( param_parent ), id( param_id ){}
		
		// This is a pure virtual function that we have to override.
		// I think all it needs to do is call exec.
		virtual void run();

		unsigned int start_idx1, start_idx2, end_idx1, end_idx2;

		int *inCount;
		int *outCount;
		QSemaphore **semaphore;
		QSemaphore **barrier;

		int nWorkers;
		int iters;
		NernstSim *s;
	private:
		void Barrier(void);

};

