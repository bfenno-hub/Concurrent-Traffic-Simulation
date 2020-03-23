#include <iostream>
#include <random>
#include <thread>
#include <future>
#include "TrafficLight.h"
//#include "TrafficObject.h"
/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
     // perform queue modification under the lock
    
    std::unique_lock<std::mutex> uLock(_mutex);
    _condition.wait(uLock, [this] { return !_queue.empty(); }); // pass unique lock to condition variable
    // remove last element from queue
    T msg = std::move(_queue.front());
    _queue.pop_front();
   // std::cout << "   Message " << msg << " removed from the queue" << std::endl;
    return msg; 
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    
    std::lock_guard<std::mutex> uLock(_mutex);
   // std::cout << "   Message " << msg << " will be added to the queue " << std::endl;
    _queue.push_back(std::move(msg));
    _condition.notify_one(); 
}


template <typename T>
void MessageQueue<T>::clear()
{
  	std::lock_guard<std::mutex> lck(_mutex);
  	_queue.clear();
}

/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
  
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
  	
	_messageQueue.clear(); //remove previous light changes sitting in the queue. 
  
  	while (true){
        if (_messageQueue.receive() == TrafficLight::green)	
            return;
    }
  	
}

TrafficLight::TrafficLightPhase TrafficLight::getCurrentPhase()
{
    std::lock_guard<std::mutex> lck(_lmutex);
    return _currentPhase;
  	
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    auto stopwatchstart = std::chrono::high_resolution_clock::now(); //fix this
    std::chrono::duration<double> duration;
    double seconds;
    int timetowait;
    while (true){
       
        stopwatchstart = std::chrono::high_resolution_clock::now(); //reset stopwatch
      
        timetowait = 4000 + (rand()) / (RAND_MAX/2000);
        std::this_thread::sleep_for(std::chrono::milliseconds(timetowait));
    
		//toggle light
        std::unique_lock<std::mutex> lck(_lmutex);
        switch (_currentPhase) {
            case red: _currentPhase = green;
                break;
            case green: _currentPhase = red;
                break;
            default:
                break;
        }
        lck.unlock();
      	
        _messageQueue.send(std::move(TrafficLightPhase(_currentPhase)));
      
      	duration = std::chrono::high_resolution_clock::now() - stopwatchstart;
        seconds = duration.count();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
}

