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
    for (const StyleColorData& color_data : colors_) {
      ImGui::PushStyleColor(color_data.idx, color_data.color);
    }
  }
  if (!vars_.empty()) {
    for (const StyleVarData& var_data : vars_) {
      ImGui::PushStyleVar(var_data.idx, var_data.val);
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
