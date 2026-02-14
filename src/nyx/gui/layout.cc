#include "nyx/gui/layout.h"

namespace nyx {

void SeparatorWidget::Render() {
  ImGui::Separator();
}

void SpacingWidget::Render() {
  ImGui::Spacing();
}

SameLineWidget::SameLineWidget(Realm* realm, v8::Local<v8::Object> object, float offset, float spacing)
    : Widget(realm, object), offset_(offset), spacing_(spacing) {}

void SameLineWidget::Render() {
  ImGui::SameLine(offset_, spacing_);
}

void NewLineWidget::Render() {
  ImGui::NewLine();
}

IndentWidget::IndentWidget(Realm* realm, v8::Local<v8::Object> object, float width)
    : Widget(realm, object), width_(width) {}

void IndentWidget::Render() {
  ImGui::Indent(width_);
  RenderChildren();
  ImGui::Unindent(width_);
}

UnindentWidget::UnindentWidget(Realm* realm, v8::Local<v8::Object> object, float width)
    : Widget(realm, object), width_(width) {}

void UnindentWidget::Render() {
  ImGui::Unindent(width_);
  RenderChildren();
  ImGui::Indent(width_);
}

DummyWidget::DummyWidget(Realm* realm, v8::Local<v8::Object> object, float width, float height)
    : Widget(realm, object), width_(width), height_(height) {}

void DummyWidget::Render() {
  ImGui::Dummy(ImVec2(width_, height_));
}

void GroupWidget::Render() {
  ImGui::BeginGroup();
  RenderChildren();
  ImGui::EndGroup();
}

DisabledWidget::DisabledWidget(Realm* realm, v8::Local<v8::Object> object, bool disabled)
    : Widget(realm, object), disabled_(disabled) {}

void DisabledWidget::Render() {
  ImGui::BeginDisabled(disabled_);
  RenderChildren();
  ImGui::EndDisabled();
}

}  // namespace nyx
