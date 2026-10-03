#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>
#include <iostream>

class Queue {
    std::mutex m;
    std::condition_variable ready, space;
    std::deque<std::function<void()>> jobs;
    size_t capacity;
    bool stopping = false;
public:
    explicit Queue(size_t n): capacity(n) {}
    bool push(std::function<void()> f) {
        std::unique_lock lock(m);
        space.wait(lock, [&]{ return jobs.size() < capacity || stopping; });
        if (stopping) return false;
        jobs.push_back(std::move(f));
        ready.notify_one();
        return true;
    }
    void worker() {
        for (;;) {
            std::function<void()> job;
            {
                std::unique_lock lock(m);
                ready.wait(lock, [&]{ return stopping || !jobs.empty(); });
                if (stopping && jobs.empty()) return;
                job = std::move(jobs.front());
                jobs.pop_front();
                space.notify_one();
            }
            job();
        }
    }
    void close() {
        { std::lock_guard lock(m); stopping = true; }
        ready.notify_all();
        space.notify_all();
    }
};
int main() {
    Queue q(8);
    std::vector<std::thread> workers;
    for (int i=0; i<2; ++i) workers.emplace_back([&]{ q.worker(); });
    for (int i=1; i<=20; ++i) q.push([i]{ std::cout << "processed " << i << '\n'; });
    q.close();
    for (auto& worker : workers) worker.join();
}