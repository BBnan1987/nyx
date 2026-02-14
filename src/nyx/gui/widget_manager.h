#pragma once

#include "nyx/gui/widget.h"

#include <vector>

namespace nyx {

class WidgetManager {
 public:
  WidgetManager() = default;
  ~WidgetManager() = default;

  void AddRoot(Widget* widget);
  void RemoveRoot(Widget* widget);
  void RenderAll();

 private:
  std::vector<Widget*> roots_;
};

}  // namespace nyx
