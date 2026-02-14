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
  void Render() override;
  const std::string& label() const { return label_; }
  const std::vector<float>& values() const { return values_; }
  void set_values(std::vector<float> v) { values_ = std::move(v); }
  void set_overlay(const std::string& o) { overlay_ = o; }

 private:
  std::string label_;
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
  void Render() override;
  const std::string& label() const { return label_; }
  const std::vector<float>& values() const { return values_; }
  void set_values(std::vector<float> v) { values_ = std::move(v); }
  void set_overlay(const std::string& o) { overlay_ = o; }

 private:
  std::string label_;
  std::vector<float> values_;
  std::string overlay_;
  float scale_min_, scale_max_;
  float width_, height_;
};

}  // namespace nyx
