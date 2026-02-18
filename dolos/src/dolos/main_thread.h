#pragma once

#include <functional>

namespace dolos {

// Queue a function to run on the main thread and block until it completes.
// Safe to call while holding the game lock — the main thread pump will execute
// it during the open window and fulfill the result before releasing.
void RunOnMainThread(std::function<void()> fn);

// Execute all queued main-thread tasks. Called by the game lock pump on the
// main thread during each open window. Must only be called from the main thread.
void DrainMainThreadQueue();

}  // namespace dolos
