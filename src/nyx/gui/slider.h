#pragma once

#include "nyx/gui/widget.h"

/** Implements:
 * + ImGui::DragFloat
 * - ImGui::DragFloatRange2
 * + ImGui::DragInt
 * - ImGui::DragIntRange2
 * + ImGui::SliderFloat
 * + ImGui::SliderInt
 * - ImGui::SliderAngle
 */

namespace nyx {

class SliderFloatWidget : public Widget {
 public:
  SliderFloatWidget(
      Realm* realm, v8::Local<v8::Object> object, const std::string& label, float min, float max, float value);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void GetValue(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetValue(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;

  float value() const { return value_; }
  void set_value(float v) { value_ = v; }
  float min() const { return min_; }
  void set_min(float m) { min_ = m; }
  float max() const { return max_; }
  void set_max(float m) { max_ = m; }

 private:
  float value_;
  float min_;
  float max_;
};

class SliderIntWidget : public Widget {
 public:
  SliderIntWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, int min, int max, int value);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void GetValue(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetValue(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;

  int value() const { return value_; }
  void set_value(int v) { value_ = v; }
  int min() const { return min_; }
  void set_min(int m) { min_ = m; }
  int max() const { return max_; }
  void set_max(int m) { max_ = m; }

 private:
  int value_;
  int min_;
  int max_;
};

class DragFloatWidget : public Widget {
 public:
  DragFloatWidget(Realm* realm,
                  v8::Local<v8::Object> object,
                  const std::string& label,
                  float value,
                  float speed,
                  float min,
                  float max);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void GetValue(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetValue(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;
  float value() const { return value_; }
  void set_value(float v) { value_ = v; }

 private:
  float value_, speed_, min_, max_;
};

class DragIntWidget : public Widget {
 public:
  DragIntWidget(
      Realm* realm, v8::Local<v8::Object> object, const std::string& label, int value, float speed, int min, int max);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void GetValue(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void SetValue(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;
  int value() const { return value_; }
  void set_value(int v) { value_ = v; }

 private:
  int value_;
  float speed_;
  int min_, max_;
};

}  // namespace nyx
