#pragma once

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

namespace GeneticSimulation
{
	// A synchronization object which enables one or more threads to
	// independently wait for one or more threads to signal before continuing
	class SignalLink
	{
	public:

		// constructor
		SignalLink(unsigned int signal_threads, unsigned int wait_threads, bool start_ready = false);

		// notify signal link
		void notify();

		// wait for signal link
		void wait();

	private:

		// number of signalling threads
		const unsigned int notify_threads;
		// number of waiting threads
		const unsigned int wait_threads;
		// whether all threads have signalled
		bool ready;
		// number of threads which have signalled
		unsigned int current_notify;
		// number of threads which have requested to continue
		unsigned int current_wait;
		// condition variable used for signalling
		boost::condition_variable cond;
		// mutex used to protect ready flag, notify and wait
		// counts and condition variable
		boost::mutex mx;
	};
}