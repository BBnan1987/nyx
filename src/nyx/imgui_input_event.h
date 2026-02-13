#pragma once

#include <imgui.h>

namespace nyx {

struct ImGuiInputEvent {
  enum Type {
    kMousePos,
    kMouseButton,
    kMouseWheel,
    kMouseSource,
    kKey,
    kChar,
    kCharUTF16,
    kFocus,
    kDisplaySize,
  };

  Type type;

  union {
    struct {
      float x, y;
    } mouse_pos;
    struct {
      int button;
      bool down;
    } mouse_button;
    struct {
      float x, y;
    } mouse_wheel;
    struct {
      ImGuiMouseSource source;
    } mouse_source;
    struct {
      ImGuiKey key;
      bool down;
    } key;
    struct {
      unsigned int ch;
    } char_event;
    struct {
      unsigned short ch;
    } char_utf16;
    struct {
      bool focused;
    } focus;
    struct {
      float w, h;
    } display_size;
  };

  static void ReplayTo(ImGuiIO& io, const ImGuiInputEvent* events, int count);
};

}  // namespace nyx
