#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <time.h>
#include <vector>
#include <queue>
#include <future>
#include <mutex>
#include <queue>
#include <functional>
#include <future>
#include <thread>
#include <utility>
#include <vector>
#include <condition_variable>
#include <string>
#include <shared_mutex>

namespace Util
{
template<typename T>
class TaskQueue 
{
private:
    std::queue<T> que;
    std::shared_mutex mutex;

public:
    bool empty() 
    {
        std::shared_lock<std::shared_mutex> lc(mutex);
        return que.empty();
    }

    auto size() 
    {
        std::shared_lock<std::shared_mutex> lc(mutex);
        return que.size();
    }

    void push(T& t) 
    {
        std::unique_lock<std::shared_mutex> lc(mutex);
        que.push(t);
    }

    bool pop(T& t) 
    {
        std::unique_lock<std::shared_mutex> lc(mutex);
        if (que.empty())
        {
            return false;
        }

        t = std::move(que.front());
        que.pop();

        return true;
    }
};

class ThreadPool 
{
private:
    class Worker 
    {
    public:
        ThreadPool* pool;

        Worker(ThreadPool* _pool) : pool{ _pool } {}

        void operator()() 
        {
            while (!pool->isShutDown) 
            {
                {
                    std::unique_lock<std::mutex> lock(pool->mutex);
                    pool->cv.wait(lock, [this]() {
                        return this->pool->isShutDown || !this->pool->que.empty();
                    });
                }

                std::function<void()> func;
                bool flag = pool->que.pop(func);

                if (flag) 
                {
                    func();
                }
            }
        }
    };

private:
    bool isShutDown;
    TaskQueue<std::function<void()>> que;
    std::vector<std::thread> threads;
    std::mutex mutex;
    std::condition_variable cv;

public:
    ThreadPool(size_t threadNum = std::thread::hardware_concurrency()) : isShutDown(false), threads(threadNum)
    {
        for (auto& thread: threads)
        {
            thread = std::thread{Worker(this)};
        }
    }

    ~ThreadPool() 
    {
        isShutDown = true;
        cv.notify_all();

        for (auto& t : threads) 
        {
            if (t.joinable())
            {
                t.join();
            }
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    template<typename F, typename... Args>
    auto submit(F&& f, Args &&...args) -> std::future<decltype(f(args...))>
    {
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        auto task = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
        std::function<void()> warpper = [task]() {
            (*task)();
        };
        
        que.push(warpper);
        cv.notify_one();

        return task->get_future();
    }
};
}