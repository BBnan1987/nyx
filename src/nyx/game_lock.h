#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace nyx {

class GameLock {
 public:
  void Open(std::chrono::milliseconds timeout = std::chrono::milliseconds(2));

  bool Acquire(std::chrono::milliseconds timeout = std::chrono::milliseconds(100));
  void Release();

  bool IsHeld() const;
  bool IsOpen() const;

 private:
  std::mutex mutex_;
  std::condition_variable cv_lock_available_;
  std::condition_variable cv_lock_released_;

  std::atomic<bool> lock_open_{false};
  std::atomic<bool> lock_held_{false};
  std::atomic<std::thread::id> owner_id_{};
  uint32_t recursive_count_{0};
};

}  // namespace nyx
