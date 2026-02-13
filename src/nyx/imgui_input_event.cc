#include "nyx/imgui_input_event.h"

namespace nyx {

void ImGuiInputEvent::ReplayTo(ImGuiIO& io, const ImGuiInputEvent* events, int count) {
  for (int i = 0; i < count; ++i) {
    const auto& ev = events[i];
    switch (ev.type) {
      case kMousePos:
        io.AddMousePosEvent(ev.mouse_pos.x, ev.mouse_pos.y);
        break;
      case kMouseButton:
        io.AddMouseButtonEvent(ev.mouse_button.button, ev.mouse_button.down);
        break;
      case kMouseWheel:
        io.AddMouseWheelEvent(ev.mouse_wheel.x, ev.mouse_wheel.y);
        break;
      case kMouseSource:
        io.AddMouseSourceEvent(ev.mouse_source.source);
        break;
      case kKey:
        io.AddKeyEvent(ev.key.key, ev.key.down);
        break;
      case kChar:
        io.AddInputCharacter(ev.char_event.ch);
        break;
      case kCharUTF16:
        io.AddInputCharacterUTF16(ev.char_utf16.ch);
        break;
      case kFocus:
        io.AddFocusEvent(ev.focus.focused);
        break;
      case kDisplaySize:
        io.DisplaySize = ImVec2(ev.display_size.w, ev.display_size.h);
        break;
    }
  }
}

}  // namespace nyx
