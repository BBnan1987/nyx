#include "nyx/gui/tooltip.h"

namespace nyx {

TooltipWidget::TooltipWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& text)
    : Widget(realm, object), text_(text) {}

void TooltipWidget::Render() {
  if (ImGui::IsItemHovered()) {
    ImGui::BeginTooltip();
    if (!text_.empty()) {
      ImGui::TextUnformatted(text_.c_str());
    }
    RenderChildren();
    ImGui::EndTooltip();
  }
}

}  // namespace nyx
