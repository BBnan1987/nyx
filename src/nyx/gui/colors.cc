#include "nyx/gui/colors.h"

namespace nyx {

ColorEdit3Widget::ColorEdit3Widget(
    Realm* realm, v8::Local<v8::Object> object, const std::string& label, float r, float g, float b)
    : Widget(realm, object), label_(label) {
  col_[0] = r;
  col_[1] = g;
  col_[2] = b;
}

void ColorEdit3Widget::Render() {
  float old[3] = {col_[0], col_[1], col_[2]};
  ImGui::ColorEdit3(label_.c_str(), col_);
  if (col_[0] != old[0] || col_[1] != old[1] || col_[2] != old[2]) {
    EmitEvent("change");
  }
}

ColorEdit4Widget::ColorEdit4Widget(
    Realm* realm, v8::Local<v8::Object> object, const std::string& label, float r, float g, float b, float a)
    : Widget(realm, object), label_(label) {
  col_[0] = r;
  col_[1] = g;
  col_[2] = b;
  col_[3] = a;
}

void ColorEdit4Widget::Render() {
  float old[4] = {col_[0], col_[1], col_[2], col_[3]};
  ImGui::ColorEdit4(label_.c_str(), col_);
  if (col_[0] != old[0] || col_[1] != old[1] || col_[2] != old[2] || col_[3] != old[3]) {
    EmitEvent("change");
  }
}

}  // namespace nyx
