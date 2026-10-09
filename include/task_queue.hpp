#pragma once
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <vector>

class TaskQueue {
public:
  using Clock = std::chrono::steady_clock;
  struct Metrics {
    std::uint64_t accepted = 0, completed = 0, failed = 0, cancelled = 0;
    std::size_t queued = 0, active = 0;
  };

private:
  struct Job {
    int priority;
    std::uint64_t sequence;
    std::function<void()> run, cancel;
    bool operator<(const Job &b) const {
      return priority == b.priority ? sequence > b.sequence
                                    : priority < b.priority;
    }
  };
  mutable std::mutex mutex_;
  std::condition_variable ready_, space_, idle_;
  std::priority_queue<Job> jobs_;
  std::vector<std::thread> workers_;
  std::size_t capacity_;
  bool closed_ = false;
  Metrics metrics_;
  void worker() {
    for (;;) {
      Job job;
      {
        std::unique_lock lock(mutex_);
        ready_.wait(lock, [&] { return closed_ || !jobs_.empty(); });
        if (jobs_.empty() && closed_)
          return;
        job = jobs_.top();
        jobs_.pop();
        metrics_.active++;
        space_.notify_all();
      }
      job.run();
      {
        std::lock_guard lock(mutex_);
        metrics_.active--;
        if (!metrics_.active && jobs_.empty())
          idle_.notify_all();
      }
    }
  }

public:
  TaskQueue(std::size_t workers, std::size_t capacity) : capacity_(capacity) {
    if (!workers || !capacity)
      throw std::invalid_argument(
          "positive capacity and worker count required");
    try {
      for (std::size_t i = 0; i < workers; i++)
        workers_.emplace_back([this] { worker(); });
    } catch (...) {
      shutdown(false);
      throw;
    }
  }
  TaskQueue(const TaskQueue &) = delete;
  TaskQueue &operator=(const TaskQueue &) = delete;
  ~TaskQueue() { shutdown(true); }
  template <class Function>
  auto submit(Function function, int priority = 0,
              std::chrono::milliseconds timeout = std::chrono::seconds(5),
              Clock::time_point deadline = Clock::time_point::max())
      -> std::future<std::invoke_result_t<Function>> {
    using Result = std::invoke_result_t<Function>;
    auto promise = std::make_shared<std::promise<Result>>();
    auto future = promise->get_future();
    auto task = std::make_shared<Function>(std::move(function));
    std::unique_lock lock(mutex_);
    if (!space_.wait_for(lock, timeout,
                         [&] { return closed_ || jobs_.size() < capacity_; }))
      throw std::runtime_error("capacity timeout");
    if (closed_)
      throw std::runtime_error("queue closed");
    auto run = [this, promise, task, deadline] {
      try {
        if (Clock::now() > deadline)
          throw std::runtime_error("deadline exceeded");
        if constexpr (std::is_void_v<Result>) {
          std::invoke(*task);
          promise->set_value();
        } else
          promise->set_value(std::invoke(*task));
        std::lock_guard guard(mutex_);
        metrics_.completed++;
      } catch (...) {
        promise->set_exception(std::current_exception());
        std::lock_guard guard(mutex_);
        metrics_.failed++;
      }
    };
    auto cancel = [promise] {
      promise->set_exception(
          std::make_exception_ptr(std::runtime_error("cancelled")));
    };
    jobs_.push(
        Job{priority, metrics_.accepted++, std::move(run), std::move(cancel)});
    ready_.notify_one();
    return future;
  }
  bool wait_idle(std::chrono::milliseconds timeout) {
    std::unique_lock lock(mutex_);
    return idle_.wait_for(lock, timeout,
                          [&] { return jobs_.empty() && !metrics_.active; });
  }
  Metrics metrics() const {
    std::lock_guard lock(mutex_);
    auto m = metrics_;
    m.queued = jobs_.size();
    return m;
  }
  void shutdown(bool drain) {
    {
      std::lock_guard lock(mutex_);
      closed_ = true;
      if (!drain)
        while (!jobs_.empty()) {
          jobs_.top().cancel();
          jobs_.pop();
          metrics_.cancelled++;
        }
    }
    ready_.notify_all();
    space_.notify_all();
    idle_.notify_all();
    for (auto &thread : workers_)
      if (thread.joinable())
        thread.join();
  }
};
