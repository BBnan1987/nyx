#include "nyx/gui/text.h"

namespace nyx {

TextWidget::TextWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& text)
    : Widget(realm, object), text_(text) {}

void TextWidget::Render() {
  ImGui::TextUnformatted(text_.c_str());
}

TextColoredWidget::TextColoredWidget(
    Realm* realm, v8::Local<v8::Object> object, const std::string& text, float r, float g, float b, float a)
    : Widget(realm, object), text_(text), r_(r), g_(g), b_(b), a_(a) {}

void TextColoredWidget::Render() {
  ImGui::TextColored(ImVec4(r_, g_, b_, a_), "%s", text_.c_str());
}

TextWrappedWidget::TextWrappedWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& text)
    : Widget(realm, object), text_(text) {}

void TextWrappedWidget::Render() {
  ImGui::TextWrapped("%s", text_.c_str());
}

TextDisabledWidget::TextDisabledWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& text)
    : Widget(realm, object), text_(text) {}

void TextDisabledWidget::Render() {
  ImGui::TextDisabled("%s", text_.c_str());
}

LabelTextWidget::LabelTextWidget(Realm* realm,
                                 v8::Local<v8::Object> object,
                                 const std::string& label,
                                 const std::string& text)
    : Widget(realm, object), label_(label), text_(text) {}

void LabelTextWidget::Render() {
  EmitEvent("update");
  ImGui::LabelText(label_.c_str(), "%s", text_.c_str());
}

BulletTextWidget::BulletTextWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& text)
    : Widget(realm, object), text_(text) {}

void BulletTextWidget::Render() {
  ImGui::BulletText("%s", text_.c_str());
}

SeparatorTextWidget::SeparatorTextWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& text)
    : Widget(realm, object), text_(text) {}

void SeparatorTextWidget::Render() {
  ImGui::SeparatorText(text_.c_str());
}

}  // namespace nyx
