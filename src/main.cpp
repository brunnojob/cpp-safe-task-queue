#include "task_queue.hpp"
#include <iostream>
int main() {
  TaskQueue queue(4, 16);
  std::vector<std::future<std::uint64_t>> jobs;
  for (std::uint64_t n = 1; n <= 100; n++)
    jobs.push_back(queue.submit([n] { return n * n; }, int(n % 3)));
  std::uint64_t sum = 0;
  for (auto &job : jobs)
    sum += job.get();
  queue.shutdown(true);
  auto m = queue.metrics();
  std::cout << "{\"sumSquares\":" << sum << ",\"accepted\":" << m.accepted
            << ",\"completed\":" << m.completed << ",\"failed\":" << m.failed
            << "}\n";
}
