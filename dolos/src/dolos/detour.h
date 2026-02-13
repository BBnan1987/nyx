#pragma once

#include "dolos/pipe_log.h"
#include "nyx/util.h"

#include <MinHook.h>
#include <windows.h>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace dolos {

// XXX: This metaprogramming could be useful elsewhere, move to meta.h or something
template <typename T>
struct member_class;
template <typename Class, typename Return, typename... Args>
struct member_class<Return (Class::*)(Args...)> {
  using type = Class;
};

template <typename T>
struct member_return;
template <typename Class, typename Return, typename... Args>
struct member_return<Return (Class::*)(Args...)> {
  using type = Return;
};

template <typename T>
struct free_signature;
template <typename Class, typename Return, typename... Args>
struct free_signature<Return (__stdcall Class::*)(Args...)> {
  using type = Return(__stdcall*)(Class*, Args...);
};

class DetourBase {
 public:
  bool IsEnabled() const { return enabled_; }

 protected:
  virtual ~DetourBase() noexcept = default;

  void* target_ = nullptr;    // pointer to original function
  void* detour_ = nullptr;    // user-provided detour
  void* original_ = nullptr;  // pointer to trampoline function used to call original target
  bool enabled_ = false;
};

template <typename Fx>
class Detour;

template <typename R, typename... Args>
class Detour<R (*)(Args...)> : public DetourBase {
  // static helper thunk for detouring to class method
  template <typename DetourClass, typename R, typename... Args>
  struct MemberThunk {
    using MemFn = R (DetourClass::*)(Args...);
    static R Thunk(Args... args) { return (instance->*detour)(std::forward<Args>(args)...); }
    static inline MemFn detour;
    static inline DetourClass* instance;
  };

 public:
  using FreePtr = R (*)(Args...);

  Detour() {}
  Detour(FreePtr detour) : detour_(detour) {}
  template <typename DetourClass>
  Detour(R (DetourClass::*detour_member)(Args...), DetourClass* detour_instance) {
    using ThunkType = MemberThunk<DetourClass, R, Args...>;
    ThunkType::detour = detour_member;
    ThunkType::instance = detour_instance;
    detour_ = &ThunkType::Thunk;
  }

  bool Install(const wchar_t* mod, const char* proc) {
    CHECK_NOT_NULL(detour_);
    return Install(mod, proc, static_cast<FreePtr>(detour_));
  }

  bool Install(const wchar_t* mod, const char* proc, FreePtr detour) {
    MH_STATUS status = MH_OK;
    detour_ = detour;

    status = MH_CreateHookApiEx(mod, proc, detour_, &original_, &target_);
    if (status != MH_OK) {
      PIPE_LOG_ERROR("[Detour] Failed to create API hook {} - {}", proc, MH_StatusToString(status));
      return false;
    }

    status = MH_EnableHook(target_);
    if (status != MH_OK) {
      PIPE_LOG_ERROR("[Detour] Failed to enable API hook {} - {}", proc, MH_StatusToString(status));
      return false;
    }
    PIPE_LOG("[Detour] Installed API hook {}", proc);

    enabled_ = true;
    return true;
  }

  template <typename DetourClass>
  bool Install(const wchar_t* mod,
               const char* proc,
               R (DetourClass::*detour_member)(Args...),
               DetourClass* detour_instance) {
    using ThunkType = MemberThunk<DetourClass, R, Args...>;
    ThunkType::detour = detour_member;
    ThunkType::instance = detour_instance;
    return Install(mod, proc, &ThunkType::Thunk);
  }

  void Uninstall() {
    MH_STATUS status = MH_OK;

    if (!enabled_) {
      return;
    }

    status = MH_DisableHook(target_);
    if (status != MH_OK) {
      PIPE_LOG_ERROR("[Detour] Failed to disable hook {:p} - {}", target_, MH_StatusToString(status));
      return;
    }

    status = MH_RemoveHook(target_);
    if (status != MH_OK) {
      PIPE_LOG_ERROR("[Detour] Failed to remove hook {:p} - {}", target_, MH_StatusToString(status));
      return;
    }

    PIPE_LOG("[Detour] Uninstalled hook {:p}", target_);
    enabled_ = false;
  }

  R CallOriginal(Args... args) { return static_cast<FreePtr>(original_)(std::forward<Args>(args)...); }
};

template <typename Class, typename R, typename... Args>
class Detour<R (Class::*)(Args...)> : public DetourBase {
  // static helper thunk for detouring to class method
  template <typename Class, typename DetourClass, typename R, typename... Args>
  struct MemberThunk {
    using MemFn = R (DetourClass::*)(Class*, Args...);
    static R Thunk(Class* obj, Args... args) { return (instance->*detour)(obj, std::forward<Args>(args)...); }
    static inline MemFn detour;
    static inline DetourClass* instance;
  };

 public:
  using MemberPtr = R (Class::*)(Args...);
  using ClassType = typename member_class<MemberPtr>::type;
  using ReturnType = typename member_return<MemberPtr>::type;
  using FreePtr = typename free_signature<MemberPtr>::type;

  Detour() {}
  Detour(FreePtr detour) : detour_(detour) {}
  template <typename DetourClass>
  Detour(R (DetourClass::*detour_member)(ClassType*, Args...), DetourClass* detour_instance) {
    using ThunkType = MemberThunk<ClassType, DetourClass, R, Args...>;
    ThunkType::detour = detour_member;
    ThunkType::instance = detour_instance;
    detour_ = &ThunkType::Thunk;
  }

  bool Install(ClassType* instance, std::size_t index) {
    CHECK_NOT_NULL(detour_);
    return Install(instance, index, static_cast<FreePtr>(detour_));
  }

  bool Install(ClassType* instance, std::size_t index, FreePtr detour) {
    MH_STATUS status = MH_OK;
    void** vtable = *reinterpret_cast<void***>(instance);
    target_ = vtable[index];
    detour_ = detour;

    status = MH_CreateHook(target_, detour_, &original_);
    if (status != MH_OK) {
      PIPE_LOG_ERROR("[Detour] Failed to create VTable hook at {:p}[{}] - {}",
                     static_cast<void*>(vtable),
                     index,
                     MH_StatusToString(status));
      return false;
    }

    status = MH_EnableHook(target_);
    if (status != MH_OK) {
      PIPE_LOG_ERROR("[Detour] Failed to enable VTable hook at {:p}[{}] - {}",
                     static_cast<void*>(vtable),
                     index,
                     MH_StatusToString(status));
      return false;
    }

    PIPE_LOG("[Detour] Installed VTable hook at {:p}[{}]", static_cast<void*>(vtable), index);
    enabled_ = true;
    return true;
  }

  template <typename DetourClass>
  bool Install(ClassType* instance,
               std::size_t index,
               R (DetourClass::*detour_member)(ClassType*, Args...),
               DetourClass* detour_instance) {
    using ThunkType = MemberThunk<ClassType, DetourClass, R, Args...>;
    ThunkType::detour = detour_member;
    ThunkType::instance = detour_instance;
    return Install(instance, index, &ThunkType::Thunk);
  }

  void Uninstall() {
    if (!enabled_) {
      return;
    }
    enabled_ = false;

    MH_STATUS status = MH_DisableHook(target_);
    if (status != MH_OK) {
      PIPE_LOG_ERROR("[Detour] Failed to disable VTable hook {:p} - {}", target_, MH_StatusToString(status));
    }

    status = MH_RemoveHook(target_);
    if (status != MH_OK) {
      PIPE_LOG_ERROR("[Detour] Failed to remove VTable hook {:p} - {}", target_, MH_StatusToString(status));
    }
  }

  ReturnType CallOriginal(ClassType* instance, Args... args) {
    return static_cast<FreePtr>(original_)(instance, std::forward<Args>(args)...);
  }
};

}  // namespace dolos
