#include "dolos/main_thread.h"

#include <future>
#include <mutex>
#include <queue>

namespace dolos {

namespace {

struct Task {
  std::function<void()> fn;
  std::promise<void> promise;
};

std::mutex g_mutex;
std::queue<std::unique_ptr<Task>> g_queue;

}  // namespace

void RunOnMainThread(std::function<void()> fn) {
  auto task = std::make_unique<Task>();
  task->fn = std::move(fn);
  auto future = task->promise.get_future();

  {
    std::lock_guard lock(g_mutex);
    g_queue.push(std::move(task));
  }

  future.wait();
}

void DrainMainThreadQueue() {
  std::unique_lock lock(g_mutex);
  while (!g_queue.empty()) {
    auto task = std::move(g_queue.front());
    g_queue.pop();
    lock.unlock();
    task->fn();
    task->promise.set_value();
    lock.lock();
  }
}

}  // namespace dolos
