#pragma once

#include "nyx/gui/widget.h"

/** Implements:
 * + ImGui::Text
 * + ImGui::TextColored
 * + ImGui::TextDisabled
 * + ImGui::TextWrapped
 * + ImGui::LabelText
 * + ImGui::BulletText
 * + ImGui::SeparatorText
 */

namespace nyx {

class TextWidget : public Widget {
 public:
  TextWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& text);

  void Render() override;

  const std::string& text() const { return text_; }
  void set_text(const std::string& t) { text_ = t; }

 private:
  std::string text_;
};

class TextColoredWidget : public Widget {
 public:
  TextColoredWidget(
      Realm* realm, v8::Local<v8::Object> object, const std::string& text, float r, float g, float b, float a);

  void Render() override;

  const std::string& text() const { return text_; }
  void set_text(const std::string& t) { text_ = t; }
  float r() const { return r_; }
  float g() const { return g_; }
  float b() const { return b_; }
  float a() const { return a_; }
  void set_color(float r, float g, float b, float a) {
    r_ = r;
    g_ = g;
    b_ = b;
    a_ = a;
  }

 private:
  std::string text_;
  float r_, g_, b_, a_;
};

class TextWrappedWidget : public Widget {
 public:
  TextWrappedWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& text);
  void Render() override;
  const std::string& text() const { return text_; }
  void set_text(const std::string& t) { text_ = t; }

 private:
  std::string text_;
};

class TextDisabledWidget : public Widget {
 public:
  TextDisabledWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& text);
  void Render() override;
  const std::string& text() const { return text_; }
  void set_text(const std::string& t) { text_ = t; }

 private:
  std::string text_;
};

class LabelTextWidget : public Widget {
 public:
  LabelTextWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, const std::string& text);
  void Render() override;
  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  const std::string& text() const { return text_; }
  void set_text(const std::string& t) { text_ = t; }

 private:
  std::string label_;
  std::string text_;
};

class BulletTextWidget : public Widget {
 public:
  BulletTextWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& text);
  void Render() override;
  const std::string& text() const { return text_; }
  void set_text(const std::string& t) { text_ = t; }

 private:
  std::string text_;
};

class SeparatorTextWidget : public Widget {
 public:
  SeparatorTextWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& text);
  void Render() override;
  const std::string& text() const { return text_; }
  void set_text(const std::string& t) { text_ = t; }

 private:
  std::string text_;
};

}  // namespace nyx
