#pragma once

#include "nyx/gui/widget.h"

/** Implements:
 * + ImGui::PlotLines
 * + ImGui::PlotHistogram
 */

namespace nyx {

class PlotLinesWidget : public Widget {
 public:
  PlotLinesWidget(Realm* realm,
                  v8::Local<v8::Object> object,
                  const std::string& label,
                  std::vector<float> values,
                  const std::string& overlay,
                  float scale_min,
                  float scale_max,
                  float width,
                  float height);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void SetValues(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;
  const std::vector<float>& values() const { return values_; }
  void set_values(std::vector<float> v) { values_ = std::move(v); }
  void set_overlay(const std::string& o) { overlay_ = o; }

 private:
  std::vector<float> values_;
  std::string overlay_;
  float scale_min_, scale_max_;
  float width_, height_;
};

class PlotHistogramWidget : public Widget {
 public:
  PlotHistogramWidget(Realm* realm,
                      v8::Local<v8::Object> object,
                      const std::string& label,
                      std::vector<float> values,
                      const std::string& overlay,
                      float scale_min,
                      float scale_max,
                      float width,
                      float height);

  static void Initialize(IsolateData* isolate_data, v8::Local<v8::ObjectTemplate> target);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void SetValues(const v8::FunctionCallbackInfo<v8::Value>& args);

  void Render() override;
  const std::vector<float>& values() const { return values_; }
  void set_values(std::vector<float> v) { values_ = std::move(v); }
  void set_overlay(const std::string& o) { overlay_ = o; }

 private:
  std::vector<float> values_;
  std::string overlay_;
  float scale_min_, scale_max_;
  float width_, height_;
};

}  // namespace nyx
