#pragma once

#include "nyx/gui/widget.h"

/** Implements:
 * + ImGui::Button
 * + ImGui::SmallButton
 * + ImGui::InvisibleButton
 * + ImGui::ArrowButton
 * + ImGui::Checkbox real implementation in JS-land
 * + ImGui::CheckboxFlags implemented in JS-land
 * + ImGui::RadioButton
 * + ImGui::ProgressBar
 * + ImGui::Bullet
 */

namespace nyx {

class ButtonWidget : public Widget {
 public:
  ButtonWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, float width, float height);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void GetClicked(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;

  bool clicked() const { return clicked_; }

 private:
  float width_ = 0.0f;
  float height_ = 0.0f;
  bool clicked_ = false;
};

class InvisibleButtonWidget : public Widget {
 public:
  InvisibleButtonWidget(
      Realm* realm, v8::Local<v8::Object> object, const std::string& label, float width, float height);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void GetClicked(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;

  bool clicked() const { return clicked_; }

 private:
  float width_ = 0.0f;
  float height_ = 0.0f;
  bool clicked_ = false;
};

class CheckboxWidget : public Widget {
 public:
  CheckboxWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, bool checked);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void GetChecked(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetChecked(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;

  bool checked() const { return checked_; }
  void set_checked(bool c) { checked_ = c; }

 private:
  bool checked_;
};

class BulletWidget : public Widget {
 public:
  using Widget::Widget;

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;
};

class SmallButtonWidget : public Widget {
 public:
  SmallButtonWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void GetClicked(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;
  bool clicked() const { return clicked_; }

 private:
  bool clicked_ = false;
};

class ArrowButtonWidget : public Widget {
 public:
  ArrowButtonWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& id, ImGuiDir dir);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void GetClicked(const v8::FunctionCallbackInfo<v8::Value>& args);

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

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void GetActive(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetActive(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void GetClicked(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;
  bool active() const { return active_; }
  void set_active(bool a) { active_ = a; }
  bool clicked() const { return clicked_; }

 private:
  bool active_;
  bool clicked_ = false;
};

class ProgressBarWidget : public Widget {
 public:
  ProgressBarWidget(Realm* realm, v8::Local<v8::Object> object, float fraction, const std::string& overlay);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void GetFraction(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetFraction(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void GetOverlay(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetOverlay(const v8::FunctionCallbackInfo<v8::Value>& args);

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
