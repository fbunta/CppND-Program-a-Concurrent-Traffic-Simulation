#include <iostream>
#include <random>
#include <thread>
#include <future>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


TrafficLightPhase MessageQueue<TrafficLightPhase>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.

    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this] { return !_queue.empty(); });
    TrafficLightPhase phase = std::move(_queue.back());
    _queue.pop_back();
    -- _size;
    return phase;
}

void MessageQueue<TrafficLightPhase>::send(TrafficLightPhase &&phase)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lck(_mutex);
    _queue.push_back(std::move(phase));
    ++_size;
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
    while (true)
    {
        if (TrafficLightPhase::green == _phaseQueue.receive())
        {
            break;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::setCurrentPhase(TrafficLightPhase phase)
{
    _currentPhase = phase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public
    // method „simulate“ is called. To do this, use the thread queue in the base class.
    TrafficObject::threads.emplace_back(std::async(std::launch::async, &TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // make this random later
        if (_currentPhase == TrafficLightPhase::red) {
            this->setCurrentPhase(TrafficLightPhase::green);
        } else {
            this->setCurrentPhase(TrafficLightPhase::red);
        }
        _phaseQueue.send(std::move(_currentPhase));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}