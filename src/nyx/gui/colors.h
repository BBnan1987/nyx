#pragma once

#include "nyx/gui/widget.h"

/** Implements:
 * + ImGui::ColorEdit3
 * + ImGui::ColorEdit4
 * - ImGui::ColorPicker3
 * - ImGui::ColorPicker4
 * - ImGui::ColorButton
 * - ImGui::SetColorEditOptions should this be global or per-widget?
 */

namespace nyx {

class ColorEdit3Widget : public Widget {
 public:
  ColorEdit3Widget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, float r, float g, float b);
  void Render() override;
  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  float r() const { return col_[0]; }
  float g() const { return col_[1]; }
  float b() const { return col_[2]; }
  void set_color(float r, float g, float b) {
    col_[0] = r;
    col_[1] = g;
    col_[2] = b;
  }

 private:
  std::string label_;
  float col_[3];
};

class ColorEdit4Widget : public Widget {
 public:
  ColorEdit4Widget(
      Realm* realm, v8::Local<v8::Object> object, const std::string& label, float r, float g, float b, float a);
  void Render() override;
  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  float r() const { return col_[0]; }
  float g() const { return col_[1]; }
  float b() const { return col_[2]; }
  float a() const { return col_[3]; }
  void set_color(float r, float g, float b, float a) {
    col_[0] = r;
    col_[1] = g;
    col_[2] = b;
    col_[3] = a;
  }

 private:
  std::string label_;
  float col_[4];
};

}  // namespace nyx
