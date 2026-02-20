#pragma once

#include "nyx/gui/widget.h"

#include <array>
#include <string>

/** Implements:
 * + ImGui::InputText
 * + ImGui::InputTextMultiline
 * - ImGui::InputTextWithHint
 *
 * + ImGui::InputFloat
 * - ImGui::InputFloat2
 * - ImGui::InputFloat3
 * - ImGui::InputFloat4
 * + ImGui::InputInt
 * - ImGui::InputInt2
 * - ImGui::InputInt3
 * - ImGui::InputInt4
 * - ImGui::InputDouble
 * + ImGui::InputScalar (used internally)
 */

namespace nyx {

class InputTextWidget : public Widget {
 public:
  InputTextWidget(Realm* realm,
                  v8::Local<v8::Object> object,
                  const std::string& label,
                  const std::string& text = "",
                  size_t max_length = 256,
                  ImGuiInputTextFlags flags = ImGuiInputTextFlags_None);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void TextGetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void TextSetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void MaxLengthGetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void MaxLengthSetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void FlagsGetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void FlagsSetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void HintGetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void HintSetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void MultilineGetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void MultilineSetter(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;

 private:
  std::string text_;
  size_t max_length_;
  ImGuiInputTextFlags flags_;
  std::string hint_;
  bool multiline_;
};

constexpr float kDefaultFloatValue = 0.0f;
constexpr float kDefaultFloatStep = 0.0f;
constexpr float kDefaultFloatStepFast = 0.0f;
constexpr int32_t kDefaultIntValue = 0;
constexpr int32_t kDefaultIntStep = 1;
constexpr int32_t kDefaultIntStepFast = 100;
constexpr double kDefaultDoubleValue = 0.0;
constexpr double kDefaultDoubleStep = 0.0;
constexpr double kDefaultDoubleStepFast = 0.0;

class InputNumberWidget : public Widget {
 public:
  enum InputNumberValueType { Float, Int, Double };

  InputNumberWidget(Realm* realm,
                    v8::Local<v8::Object> object,
                    const std::string& label,
                    InputNumberValueType type,
                    int components,
                    const std::array<double, 4>& values,
                    double step,
                    double step_fast,
                    const std::string& format = "",
                    ImGuiInputTextFlags flags = ImGuiInputTextFlags_None);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void FormatGetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void FormatSetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void FlagsGetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void FlagsSetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void ValueGetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void ValueSetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void StepGetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void StepSetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void StepFastGetter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void StepFastSetter(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;

 private:
  ImGuiInputTextFlags flags_;
  std::string format_;
  InputNumberValueType type_;
  int components_;
  std::array<double, 4> values_;
  double step_;
  double step_fast_;
};

}  // namespace nyx
