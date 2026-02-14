#include "nyx/gui/plotting.h"

namespace nyx {

PlotLinesWidget::PlotLinesWidget(Realm* realm,
                                 v8::Local<v8::Object> object,
                                 const std::string& label,
                                 std::vector<float> values,
                                 const std::string& overlay,
                                 float scale_min,
                                 float scale_max,
                                 float width,
                                 float height)
    : Widget(realm, object),
      label_(label),
      values_(std::move(values)),
      overlay_(overlay),
      scale_min_(scale_min),
      scale_max_(scale_max),
      width_(width),
      height_(height) {}

void PlotLinesWidget::Render() {
  ImGui::PlotLines(label_.c_str(),
                   values_.data(),
                   static_cast<int>(values_.size()),
                   0,
                   overlay_.empty() ? nullptr : overlay_.c_str(),
                   scale_min_,
                   scale_max_,
                   ImVec2(width_, height_));
}

PlotHistogramWidget::PlotHistogramWidget(Realm* realm,
                                         v8::Local<v8::Object> object,
                                         const std::string& label,
                                         std::vector<float> values,
                                         const std::string& overlay,
                                         float scale_min,
                                         float scale_max,
                                         float width,
                                         float height)
    : Widget(realm, object),
      label_(label),
      values_(std::move(values)),
      overlay_(overlay),
      scale_min_(scale_min),
      scale_max_(scale_max),
      width_(width),
      height_(height) {}

void PlotHistogramWidget::Render() {
  ImGui::PlotHistogram(label_.c_str(),
                       values_.data(),
                       static_cast<int>(values_.size()),
                       0,
                       overlay_.empty() ? nullptr : overlay_.c_str(),
                       scale_min_,
                       scale_max_,
                       ImVec2(width_, height_));
}

}  // namespace nyx
