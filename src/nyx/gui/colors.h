#pragma once

#include "nyx/gui/widget.h"

/** Implements:
 * + ImGui::ColorEdit3
 * + ImGui::ColorEdit4
 * + ImGui::ColorPicker3
 * + ImGui::ColorPicker4
 * + ImGui::ColorButton
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

class ColorPicker3Widget : public Widget {
 public:
  ColorPicker3Widget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, float r, float g, float b);
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

class ColorPicker4Widget : public Widget {
 public:
  ColorPicker4Widget(Realm* realm,
                     v8::Local<v8::Object> object,
                     const std::string& label,
                     float r,
                     float g,
                     float b,
                     float a,
                     float ref_r = 0.0f,
                     float ref_g = 0.0f,
                     float ref_b = 0.0f,
                     float ref_a = 0.0f);
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
  float ref_r() const { return ref_[0]; }
  float ref_g() const { return ref_[1]; }
  float ref_b() const { return ref_[2]; }
  float ref_a() const { return ref_[3]; }
  void set_ref_color(float r, float g, float b, float a) {
    ref_[0] = r;
    ref_[1] = g;
    ref_[2] = b;
    ref_[3] = a;
  }

 private:
  std::string label_;
  float col_[4];
  float ref_[4];
};

class ColorButtonWidget : public Widget {
 public:
  ColorButtonWidget(Realm* realm,
                    v8::Local<v8::Object> object,
                    const std::string& label,
                    float r,
                    float g,
                    float b,
                    float a,
                    float size_x,
                    float size_y);
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
  float size_x() const { return size_[0]; }
  float size_y() const { return size_[1]; }
  void set_size(float x, float y) {
    size_[0] = x;
    size_[1] = y;
  }
  bool clicked() const { return clicked_; }

 private:
  std::string label_;
  float col_[4];
  float size_[2];
  bool clicked_ = false;
};

}  // namespace nyx
