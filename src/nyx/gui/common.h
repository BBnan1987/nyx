#pragma once

#include "nyx/gui/widget.h"

/** Implements:
 * + ImGui::Button
 * + ImGui::SmallButton
 * - ImGui::InvisibleButton
 * + ImGui::ArrowButton
 * + ImGui::Checkbox
 * - ImGui::CheckboxFlags
 * + ImGui::RadioButton
 * + ImGui::ProgressBar
 * + ImGui::Bullet
 */

namespace nyx {

class ButtonWidget : public Widget {
 public:
  ButtonWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, float width, float height);

  void Render() override;

  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  bool clicked() const { return clicked_; }

 private:
  std::string label_;
  float width_ = 0.0f;
  float height_ = 0.0f;
  bool clicked_ = false;
};

class CheckboxWidget : public Widget {
 public:
  CheckboxWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, bool checked);

  void Render() override;

  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  bool checked() const { return checked_; }
  void set_checked(bool c) { checked_ = c; }

 private:
  std::string label_;
  bool checked_;
};

class BulletWidget : public Widget {
 public:
  using Widget::Widget;
  void Render() override;
};

class SmallButtonWidget : public Widget {
 public:
  SmallButtonWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label);
  void Render() override;
  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  bool clicked() const { return clicked_; }

 private:
  std::string label_;
  bool clicked_ = false;
};

class ArrowButtonWidget : public Widget {
 public:
  ArrowButtonWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& id, ImGuiDir dir);
  void Render() override;
  bool clicked() const { return clicked_; }

 private:
  std::string id_;
  ImGuiDir dir_;
  bool clicked_ = false;
};

class RadioButtonWidget : public Widget {
 public:
  RadioButtonWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, bool active);
  void Render() override;
  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  bool active() const { return active_; }
  void set_active(bool a) { active_ = a; }
  bool clicked() const { return clicked_; }

 private:
  std::string label_;
  bool active_;
  bool clicked_ = false;
};

class ProgressBarWidget : public Widget {
 public:
  ProgressBarWidget(Realm* realm, v8::Local<v8::Object> object, float fraction, const std::string& overlay);
  void Render() override;
  float fraction() const { return fraction_; }
  void set_fraction(float f) { fraction_ = f; }
  const std::string& overlay() const { return overlay_; }
  void set_overlay(const std::string& o) { overlay_ = o; }

 private:
  float fraction_;
  std::string overlay_;
};

}  // namespace nyx
