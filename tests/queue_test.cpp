#include "task_queue.hpp"
#include <atomic>
#include <cassert>
int main() {
  TaskQueue q(3, 5);
  std::atomic<int> count = 0;
  std::vector<std::future<void>> jobs;
  for (int i = 0; i < 1000; i++)
    jobs.push_back(q.submit([&] { count++; }));
  for (auto &job : jobs)
    job.get();
  auto failed = q.submit([]() -> int { throw std::runtime_error("failure"); });
  bool rejected = false;
  try {
    failed.get();
  } catch (const std::runtime_error &) {
    rejected = true;
  }
  assert(rejected);
  auto late = q.submit([] { return 1; }, 0, std::chrono::seconds(1),
                       TaskQueue::Clock::now() - std::chrono::seconds(1));
  rejected = false;
  try {
    late.get();
  } catch (const std::runtime_error &) {
    rejected = true;
  }
  assert(rejected);
  q.shutdown(true);
  auto m = q.metrics();
  assert(count == 1000 && m.accepted == 1002 && m.completed == 1000 &&
         m.failed == 2);
  rejected = false;
  try {
    q.submit([] {});
  } catch (const std::runtime_error &) {
    rejected = true;
  }
  assert(rejected);
}
