#include "nyx/gui/stack.h"

namespace nyx {

using v8::HandleScope;
using v8::Isolate;

void StackWidget::Render() {
  if (id_ != -1) {
    ImGui::PushID(id_);
  }
  if (clip_rect_data_.min.x != -1 && clip_rect_data_.min.y != -1 && clip_rect_data_.max.x != -1 &&
      clip_rect_data_.max.y != -1) {
    ImGui::PushClipRect(clip_rect_data_.min, clip_rect_data_.max, clip_rect_data_.intersect_with_current_clip_rect);
  }
  if (!colors_.empty()) {
    for (const auto& [idx, color] : colors_) {
      ImGui::PushStyleColor(idx, color);
    }
  }
  if (!vars_.empty()) {
    for (const auto& [idx, value] : vars_) {
      ImGui::PushStyleVar(idx, value);
    }
  }
  ImGui::PushTabStop(tab_stop_);
  ImGui::PushButtonRepeat(button_repeat_);
  if (item_width_ != 0.0f) {
    ImGui::PushItemWidth(item_width_);
  }
  if (text_wrap_ != 0.0f) {
    ImGui::PushTextWrapPos(text_wrap_);
  }
  RenderChildren();
  if (text_wrap_ != 0.0f) {
    ImGui::PopTextWrapPos();
  }
  if (item_width_ != 0.0f) {
    ImGui::PopItemWidth();
  }
  ImGui::PopButtonRepeat();
  ImGui::PopTabStop();
  if (!vars_.empty()) {
    ImGui::PopStyleVar(static_cast<int>(vars_.size()));
  }
  if (!colors_.empty()) {
    ImGui::PopStyleColor(static_cast<int>(colors_.size()));
  }
  if (clip_rect_data_.min.x != -1 && clip_rect_data_.min.y != -1 && clip_rect_data_.max.x != -1 &&
      clip_rect_data_.max.y != -1) {
    ImGui::PopClipRect();
  }
  if (id_ != -1) {
    ImGui::PopID();
  }
}

}  // namespace nyx
