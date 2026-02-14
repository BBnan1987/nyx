#pragma once

#include "nyx/gui/widget.h"

/** Implements:
 * + ImGui::BeginTooltip
 * + ImGui::EndTooltip
 * - ImGui::SetTooltip
 * - ImGui::BeginItemTooltip
 * - ImGui::EndItemTooltip
 */

namespace nyx {

class TooltipWidget : public Widget {
 public:
  TooltipWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& text);
  void Render() override;
  bool IsContainer() const override { return true; }
  const std::string& text() const { return text_; }
  void set_text(const std::string& t) { text_ = t; }

 private:
  std::string text_;
};

}  // namespace nyx
