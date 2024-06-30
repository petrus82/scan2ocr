#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>

class ThreadPool {
public:
    ThreadPool(int numThreads) : numThreads_(numThreads) {
        for (int i = 0; i < numThreads_; ++i) {
            threads_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mutex_);
                        condition_.wait(lock, [this] { return !tasks_.empty(); });
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                    {
                        std::unique_lock<std::mutex> lock(mutex_);
                        --activeThreads_;
                        if (activeThreads_ == 0) {
                            condition_.notify_all();
                        }
                    }
                }
            });
        }
    }

    template<class F, class... Args>
    void enqueue(F&& f, Args&&... args) {
        std::function<void()> task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        {
            std::unique_lock<std::mutex> lock(mutex_);
            tasks_.push(std::move(task));
        }
        condition_.notify_one();
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            done_ = true;
        }
        condition_.notify_all();
        for (auto& thread : threads_) {
            thread.join();
        }
    }

private:
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable condition_;
    int numThreads_;
    int activeThreads_ = 0;
    bool done_ = false;
};