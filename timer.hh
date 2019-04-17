#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include <functional>
#include <chrono>
#include <thread>
#include <atomic>
#include <memory>
#include <mutex>
#include <condition_variable>

class Timer
{
public:
    Timer():myexpired(true),mytrytoexpire(false)
    {

    }

    ~Timer()
    {
        Expire();
    }

    void StartTimer(int interval, std::function<void()> task)
    {
        if (!myexpired)
        {
            return;
        }

        myexpired = false;

        std::thread([=](){
                        while (!mytrytoexpire)
                        {
                            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                            task();
                        }

                        {
                            std::lock_guard<std::mutex> locker(mymutex);
                            myexpired = true;
                            myexpiredcon.notify_one();
                        }
                    }).detach();
    }

    void Expire()
    {
        if(myexpired)
        {
            return;
        }

        if (mytrytoexpire)
        {
            return;
        }

        mytrytoexpire = true;
        std::unique_lock<std::mutex> locker(mymutex);
        myexpiredcon.wait(locker, [this]{return myexpired == true; });
        if (myexpired == true)
        {
            mytrytoexpire = false;
        }
    }

    template<typename callable, class... arguments>
    void SyncWait(int after, callable&& f, arguments&&... args){

        std::function<typename std::result_of<callable(arguments...)>::type()> task
            (std::bind(std::forward<callable>(f), std::forward<arguments>(args)...));
        std::this_thread::sleep_for(std::chrono::milliseconds(after));
        task();
    }

    template<typename callable, class... arguments>
    void AsyncWait(int after, callable&& f, arguments&&... args){
        std::function<typename std::result_of<callable(arguments...)>::type()> task
            (std::bind(std::forward<callable>(f), std::forward<arguments>(args)...));

        std::thread([after, task](){
            std::this_thread::sleep_for(std::chrono::milliseconds(after));
            task();
        }).detach();
    }


    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;


private:
    std::atomic<bool> myexpired;
    std::atomic<bool> mytrytoexpire;
    std::mutex mymutex;
    std::condition_variable myexpiredcon;
};
#endif // TIMER_H_INCLUDED
