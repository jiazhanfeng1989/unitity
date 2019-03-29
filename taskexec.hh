#ifndef TASKEXECUTOR_HH_INCLUDED
#define TASKEXECUTOR_HH_INCLUDED
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>
#include <deque>
#include <future>
#include <iostream>

template<typename T>
class TaskExec final
{
public:
    using Handle = std::function<void(T&)>;
    TaskExec(size_t theThreadNum = std::thread::hardware_concurrency()):myThreadNum(theThreadNum)
    {

    }
    ~TaskExec() = default;

    void Start()
    {
        myStop  = false;
        for(size_t i = 0; i < myThreadNum ; i++)
        {
            myThreads.emplace_back(std::bind(&TaskExec<T>::run, this));
        }
    }

    void Stop()
    {
         std::call_once(myFlag,[this](){
                                std::unique_lock<std::mutex> aLock(myMutex);
                                myStop = true;
                                myCond.notify_all();
                                aLock.unlock();

                                for (auto & athread : myThreads)
                                {
                                    athread.join();
                                }}
						);
    }

    void Add(T task)
    {
        std::lock_guard<std::mutex> aLock(myMutex);
        if(myStop)
        {
            return;
        }
        else
        {
            myTasks.push_back(std::move(task));
        }

        myCond.notify_one();
    }

    void TaskClearAndStop()
    {
        waittaskclear();
        Stop();
    }

    void SetHandle(Handle thehandle)
    {
        myHandler = std::move(thehandle);
    }

private:
    void run()
    {
        while(!myStop)
        {
            std::unique_lock<std::mutex> aLock(myMutex);
            myCond.wait(aLock, [this](){return (!myTasks.empty() || myStop || myPromise);});

            if(myStop)
            {
                continue;
            }

            if(myTasks.empty() && myPromise)
            {
                myPromise->set_value();
                myPromise.reset(nullptr);
                continue;
            }

            T aTask = myTasks.front();
            myTasks.pop_front();
            aLock.unlock();

            if(myHandler)
            {
                try
                {
                    myHandler(aTask);
                }
                catch(...)
                {
                   std::cout <<"TaskExecutor catch an exception." << std::endl;
                }
            }
        }
    }
	
    void waittaskclear()
    {
        std::unique_lock<std::mutex> aLock(myMutex);
        if(!myTasks.empty() && !myPromise)
        {
            myPromise.reset(new std::promise<void>());
        }
        aLock.unlock();

        if(myPromise)
        {
            myPromise->get_future().get();
        }
    }

private:
    size_t myThreadNum;
    std::mutex myMutex;
    std::condition_variable myCond;
    Handle myHandler;
    bool myStop;
    std::once_flag myFlag;
    std::unique_ptr<std::promise<void>> myPromise;
    std::deque<T> myTasks;
    std::deque<std::thread> myThreads;
};
#endif // TASKEXECUTOR_HH_INCLUDED
