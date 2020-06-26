#include "SignalLink.h"
#include <boost/thread/mutex.hpp>

// constructor
GeneticSimulation::SignalLink::SignalLink(unsigned int signal_threads, 
	unsigned int wait_threads, bool start_ready) : 
	notify_threads(signal_threads), wait_threads(wait_threads), 
	ready(start_ready), current_notify(0), current_wait(0) {}

// notify signal link
void GeneticSimulation::SignalLink::notify()
{
	// acquire mutex lock
	boost::lock_guard lock(mx);
	// increment notify count
	current_notify++;
	// if this was the final thread to notify
	if (current_notify == notify_threads) {
		// reset notify counter
		current_notify = 0;
		// set ready flag
		ready = true;
		// notify any waiting threads
		cond.notify_all();
	}
}

// wait for signal link
void GeneticSimulation::SignalLink::wait()
{
	// acquire mutex lock
	boost::unique_lock lock(mx);
	// wait until ready
	while (!ready) {
		cond.wait(lock);
	}
	// increment wait count
	current_wait++;
	// if last thread to become ready
	if (current_wait == wait_threads) {
		// reset wait counter and clear ready flag
		current_wait = 0;
		ready = false;
	}
}