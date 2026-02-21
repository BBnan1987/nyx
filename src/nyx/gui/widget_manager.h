#pragma once

#include "nyx/gui/canvas.h"
#include "nyx/gui/widget.h"

#include <vector>

namespace nyx {

class WidgetManager {
 public:
  WidgetManager() = default;
  ~WidgetManager();

  void AddRoot(Widget* widget);
  void RemoveRoot(Widget* widget);
  void UpdateAll() const;
  void RenderAll() const;

  Canvas* background_canvas() const { return background_canvas_.get(); }
  Canvas* foreground_canvas() const { return foreground_canvas_.get(); }
  void set_background_canvas(Canvas* c) { background_canvas_.reset(c); }
  void set_foreground_canvas(Canvas* c) { foreground_canvas_.reset(c); }

 private:
  std::vector<Widget*> roots_;
  std::unique_ptr<Canvas> background_canvas_;
  std::unique_ptr<Canvas> foreground_canvas_;
};

}  // namespace nyx
