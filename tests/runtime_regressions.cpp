#include "task_queue.hpp"
#include <cassert>
int main() {
 TaskQueue queue(2,4);
 auto job=queue.submit([&]{queue.shutdown(false);});
 bool rejected=false;
 try { job.get(); } catch(const std::logic_error&) {rejected=true;}
 assert(rejected);
 auto next=queue.submit([]{return 7;}); assert(next.get()==7);
 std::thread a([&]{queue.shutdown(true);}),b([&]{queue.shutdown(true);});
 a.join(); b.join();
 assert(queue.metrics().completed==1 && queue.metrics().failed==1);
}
