#include "nyx/game_lock.h"

#include <thread>

namespace nyx {

void GameLock::Open(std::chrono::milliseconds timeout) {
  auto deadline = std::chrono::steady_clock::now() + timeout;

  {
    std::lock_guard<std::mutex> lock(mutex_);
    lock_open_ = true;
  }
  cv_lock_available_.notify_all();

  // Keep window open for the full duration
  // This allows multiple acquires/releases within the window
  std::this_thread::sleep_until(deadline);

  {
    std::unique_lock<std::mutex> lock(mutex_);
    lock_open_ = false;

    // If someone is still holding the lock, wait for them to release
    if (lock_held_.load()) {
      cv_lock_released_.wait(lock, [this]() { return !lock_held_.load(); });
    }
  }
}

bool GameLock::Acquire(std::chrono::milliseconds timeout) {
  auto this_thread = std::this_thread::get_id();

  // Fast path: recursive acquisition from the same thread
  if (lock_held_.load() && owner_id_.load() == this_thread) {
    recursive_count_++;
    return true;
  }

  std::unique_lock<std::mutex> lock(mutex_);

  // Wait for lock to be available AND not currently held
  bool predicate_satisfied =
      cv_lock_available_.wait_for(lock, timeout, [this]() { return lock_open_.load() && !lock_held_.load(); });

  if (predicate_satisfied) {
    // Double-check we can still acquire (window might have just closed)
    if (lock_open_.load() && !lock_held_.load()) {
      lock_held_ = true;
      owner_id_ = this_thread;
      recursive_count_ = 1;
      return true;
    }
  }

  return false;
}

void GameLock::Release() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (recursive_count_ > 1) {
      recursive_count_--;
      return;
    }
    recursive_count_ = 0;
    owner_id_ = std::thread::id{};
    lock_held_ = false;
  }
  // Notify both: game thread (in case window closed) and other waiters
  cv_lock_released_.notify_all();
  cv_lock_available_.notify_all();
}

bool GameLock::IsHeld() const {
  return lock_held_.load();
}

bool GameLock::IsOpen() const {
  return lock_open_.load();
}

}  // namespace nyx
