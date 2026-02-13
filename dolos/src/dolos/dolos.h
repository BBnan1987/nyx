#pragma once

#include <Windows.h>

#include "dolos/game.h"

namespace dolos {

DWORD CALLBACK ThreadEntry(LPVOID lpParameter);
HANDLE SpawnWorkerThread();
std::string get_module_cwd();

}  // namespace dolos

#define DOLOS_MAIN                                                                                                     \
  static void DolosTimerRoutine_(LPVOID lpParam, BOOLEAN) {                                                            \
    CreateThread(nullptr, 0, dolos::ThreadEntry, lpParam, 0, nullptr);                                                 \
  }                                                                                                                    \
  HANDLE gDolosTimerQueue_;                                                                                            \
  HANDLE gDolosTimerQueueTimer_;                                                                                       \
  BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID) {                                                       \
    switch (dwReason) {                                                                                                \
      case DLL_PROCESS_ATTACH:                                                                                         \
        DisableThreadLibraryCalls(hModule);                                                                            \
        gDolosTimerQueue_ = CreateTimerQueue();                                                                        \
        if (gDolosTimerQueue_ &&                                                                                       \
            CreateTimerQueueTimer(                                                                                     \
                &gDolosTimerQueueTimer_, gDolosTimerQueue_, DolosTimerRoutine_, hModule, 1, 0, WT_EXECUTEDEFAULT)) {   \
          gDolosTimerQueueTimer_ = nullptr;                                                                            \
          gDolosTimerQueue_ = nullptr;                                                                                 \
        }                                                                                                              \
        break;                                                                                                         \
      case DLL_PROCESS_DETACH:                                                                                         \
        break;                                                                                                         \
    }                                                                                                                  \
    return TRUE;                                                                                                       \
  }
