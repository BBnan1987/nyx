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

ColorPicker3Widget::ColorPicker3Widget(Realm* realm,
                                       v8::Local<v8::Object> object,
                                       const std::string& label,
                                       float r,
                                       float g,
                                       float b)
    : Widget(realm, object), label_(label) {
  col_[0] = r;
  col_[1] = g;
  col_[2] = b;
}

void ColorPicker3Widget::Render() {
  float old[3] = {col_[0], col_[1], col_[2]};
  ImGui::ColorPicker3(label_.c_str(), col_);
  if (col_[0] != old[0] || col_[1] != old[1] || col_[2] != old[2]) {
    EmitEvent("change");
  }
}

ColorPicker4Widget::ColorPicker4Widget(Realm* realm,
                                       v8::Local<v8::Object> object,
                                       const std::string& label,
                                       float r,
                                       float g,
                                       float b,
                                       float a,
                                       float ref_r,
                                       float ref_g,
                                       float ref_b,
                                       float ref_a)
    : Widget(realm, object), label_(label) {
  col_[0] = r;
  col_[1] = g;
  col_[2] = b;
  col_[3] = a;
  ref_[0] = ref_r;
  ref_[1] = ref_g;
  ref_[2] = ref_b;
  ref_[3] = ref_a;
}

void ColorPicker4Widget::Render() {
  float old[4] = {col_[0], col_[1], col_[2], col_[3]};
  ImGui::ColorPicker4(label_.c_str(), col_, 0, ref_);
  if (col_[0] != old[0] || col_[1] != old[1] || col_[2] != old[2] || col_[3] != old[3]) {
    EmitEvent("change");
  }
}

}  // namespace nyx
