#pragma once

#include "nyx/base_object.h"
#include "nyx/util.h"

#include <imgui.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace nyx {

class Environment;
class WidgetManager;

class Widget : public BaseObject {
 public:
  Widget(Realm* realm, v8::Local<v8::Object> object);
  ~Widget() override;

  virtual void Render() = 0;

  virtual bool IsContainer() const { return false; }

  Widget* parent() const { return parent_; }
  const std::vector<Widget*>& children() const { return children_; }
  void AddChild(Widget* child);
  void RemoveChild(Widget* child);
  void ClearChildren();

  bool visible() const { return visible_; }
  void set_visible(bool v) { visible_ = v; }

  void On(const std::string& event, v8::Local<v8::Function> callback);
  void Off(const std::string& event);

 protected:
  void RenderChildren();
  void EmitEvent(const std::string& event);
  void EmitEvent(const std::string& event, v8::Local<v8::Value> arg);

  Widget* parent_ = nullptr;
  std::vector<Widget*> children_;
  bool visible_ = true;
  std::unordered_map<std::string, v8::Global<v8::Function>> event_handlers_;
};

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

class PanelWidget : public Widget {
 public:
  PanelWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& title, ImGuiWindowFlags flags);

  void Render() override;
  bool IsContainer() const override { return true; }

  const std::string& title() const { return title_; }
  void set_title(const std::string& t) { title_ = t; }
  bool open() const { return open_; }
  void set_open(bool o) { open_ = o; }
  bool panel_visible() const { return panel_visible_; }
  ImGuiWindowFlags flags() const { return flags_; }
  void set_flags(ImGuiWindowFlags f) { flags_ = f; }

 private:
  std::string title_;
  bool open_ = true;
  bool panel_visible_ = false;
  ImGuiWindowFlags flags_ = 0;
};

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

class SliderFloatWidget : public Widget {
 public:
  SliderFloatWidget(
      Realm* realm, v8::Local<v8::Object> object, const std::string& label, float min, float max, float value);

  void Render() override;

  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  float value() const { return value_; }
  void set_value(float v) { value_ = v; }
  float min() const { return min_; }
  void set_min(float m) { min_ = m; }
  float max() const { return max_; }
  void set_max(float m) { max_ = m; }

 private:
  std::string label_;
  float value_;
  float min_;
  float max_;
};

class SliderIntWidget : public Widget {
 public:
  SliderIntWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, int min, int max, int value);

  void Render() override;

  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  int value() const { return value_; }
  void set_value(int v) { value_ = v; }
  int min() const { return min_; }
  void set_min(int m) { min_ = m; }
  int max() const { return max_; }
  void set_max(int m) { max_ = m; }

 private:
  std::string label_;
  int value_;
  int min_;
  int max_;
};

class InputTextWidget : public Widget {
 public:
  InputTextWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, size_t max_length);

  void Render() override;

  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  const std::string& text() const { return buffer_; }
  void set_text(const std::string& t);

 private:
  std::string label_;
  std::string buffer_;
  size_t max_length_;
};

class SeparatorWidget : public Widget {
 public:
  using Widget::Widget;
  void Render() override;
};

class SpacingWidget : public Widget {
 public:
  using Widget::Widget;
  void Render() override;
};

class SameLineWidget : public Widget {
 public:
  SameLineWidget(Realm* realm, v8::Local<v8::Object> object, float offset, float spacing);

  void Render() override;

 private:
  float offset_;
  float spacing_;
};

class TreeNodeWidget : public Widget {
 public:
  TreeNodeWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label);

  void Render() override;
  bool IsContainer() const override { return true; }

  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  bool open() const { return open_; }

 private:
  std::string label_;
  bool open_ = false;
};

class CollapsingHeaderWidget : public Widget {
 public:
  CollapsingHeaderWidget(Realm* realm,
                         v8::Local<v8::Object> object,
                         const std::string& label,
                         ImGuiTreeNodeFlags flags);

  void Render() override;
  bool IsContainer() const override { return true; }

  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  bool open() const { return open_; }

 private:
  std::string label_;
  ImGuiTreeNodeFlags flags_ = 0;
  bool open_ = false;
};

class TabBarWidget : public Widget {
 public:
  TabBarWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& id);

  void Render() override;
  bool IsContainer() const override { return true; }

 private:
  std::string id_;
};

class TabItemWidget : public Widget {
 public:
  TabItemWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label);

  void Render() override;
  bool IsContainer() const override { return true; }

  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  bool selected() const { return selected_; }

 private:
  std::string label_;
  bool selected_ = false;
};

class DemoWindowWidget : public Widget {
 public:
  using Widget::Widget;
  void Render() override;
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

class SeparatorTextWidget : public Widget {
 public:
  SeparatorTextWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& text);
  void Render() override;
  const std::string& text() const { return text_; }
  void set_text(const std::string& t) { text_ = t; }

 private:
  std::string text_;
};

class BulletWidget : public Widget {
 public:
  using Widget::Widget;
  void Render() override;
};

class NewLineWidget : public Widget {
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

class DragFloatWidget : public Widget {
 public:
  DragFloatWidget(Realm* realm,
                  v8::Local<v8::Object> object,
                  const std::string& label,
                  float value,
                  float speed,
                  float min,
                  float max);
  void Render() override;
  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  float value() const { return value_; }
  void set_value(float v) { value_ = v; }

 private:
  std::string label_;
  float value_, speed_, min_, max_;
};

class DragIntWidget : public Widget {
 public:
  DragIntWidget(
      Realm* realm, v8::Local<v8::Object> object, const std::string& label, int value, float speed, int min, int max);
  void Render() override;
  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  int value() const { return value_; }
  void set_value(int v) { value_ = v; }

 private:
  std::string label_;
  int value_;
  float speed_;
  int min_, max_;
};

class InputFloatWidget : public Widget {
 public:
  InputFloatWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, float value);
  void Render() override;
  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  float value() const { return value_; }
  void set_value(float v) { value_ = v; }

 private:
  std::string label_;
  float value_;
};

class InputIntWidget : public Widget {
 public:
  InputIntWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, int value);
  void Render() override;
  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  int value() const { return value_; }
  void set_value(int v) { value_ = v; }

 private:
  std::string label_;
  int value_;
};

class InputTextMultilineWidget : public Widget {
 public:
  InputTextMultilineWidget(Realm* realm,
                           v8::Local<v8::Object> object,
                           const std::string& label,
                           size_t max_length,
                           float width,
                           float height);
  void Render() override;
  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  const std::string& text() const { return buffer_; }
  void set_text(const std::string& t);

 private:
  std::string label_;
  std::string buffer_;
  size_t max_length_;
  float width_, height_;
};

class ColorEdit3Widget : public Widget {
 public:
  ColorEdit3Widget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, float r, float g, float b);
  void Render() override;
  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  float r() const { return col_[0]; }
  float g() const { return col_[1]; }
  float b() const { return col_[2]; }
  void set_color(float r, float g, float b) {
    col_[0] = r;
    col_[1] = g;
    col_[2] = b;
  }

 private:
  std::string label_;
  float col_[3];
};

class ColorEdit4Widget : public Widget {
 public:
  ColorEdit4Widget(
      Realm* realm, v8::Local<v8::Object> object, const std::string& label, float r, float g, float b, float a);
  void Render() override;
  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  float r() const { return col_[0]; }
  float g() const { return col_[1]; }
  float b() const { return col_[2]; }
  float a() const { return col_[3]; }
  void set_color(float r, float g, float b, float a) {
    col_[0] = r;
    col_[1] = g;
    col_[2] = b;
    col_[3] = a;
  }

 private:
  std::string label_;
  float col_[4];
};

class ComboWidget : public Widget {
 public:
  ComboWidget(Realm* realm,
              v8::Local<v8::Object> object,
              const std::string& label,
              std::vector<std::string> items,
              int selected);
  void Render() override;
  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  int selected() const { return selected_; }
  void set_selected(int s) { selected_ = s; }
  const std::vector<std::string>& items() const { return items_; }
  void set_items(std::vector<std::string> items) { items_ = std::move(items); }

 private:
  std::string label_;
  std::vector<std::string> items_;
  int selected_;
};

class ListBoxWidget : public Widget {
 public:
  ListBoxWidget(Realm* realm,
                v8::Local<v8::Object> object,
                const std::string& label,
                std::vector<std::string> items,
                int selected,
                int height_items);
  void Render() override;
  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  int selected() const { return selected_; }
  void set_selected(int s) { selected_ = s; }
  const std::vector<std::string>& items() const { return items_; }
  void set_items(std::vector<std::string> items) { items_ = std::move(items); }

 private:
  std::string label_;
  std::vector<std::string> items_;
  int selected_;
  int height_items_;
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

class SelectableWidget : public Widget {
 public:
  SelectableWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, bool selected);
  void Render() override;
  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  bool selected() const { return selected_; }
  void set_selected(bool s) { selected_ = s; }

 private:
  std::string label_;
  bool selected_;
};

class ChildWidget : public Widget {
 public:
  ChildWidget(Realm* realm,
              v8::Local<v8::Object> object,
              const std::string& id,
              float width,
              float height,
              ImGuiChildFlags child_flags,
              ImGuiWindowFlags window_flags);
  void Render() override;
  bool IsContainer() const override { return true; }

 private:
  std::string id_;
  float width_, height_;
  ImGuiChildFlags child_flags_;
  ImGuiWindowFlags window_flags_;
};

class GroupWidget : public Widget {
 public:
  using Widget::Widget;
  void Render() override;
  bool IsContainer() const override { return true; }
};

class DisabledWidget : public Widget {
 public:
  DisabledWidget(Realm* realm, v8::Local<v8::Object> object, bool disabled);
  void Render() override;
  bool IsContainer() const override { return true; }
  bool disabled() const { return disabled_; }
  void set_disabled(bool d) { disabled_ = d; }

 private:
  bool disabled_;
};

class MenuBarWidget : public Widget {
 public:
  using Widget::Widget;
  void Render() override;
  bool IsContainer() const override { return true; }
};

class MenuWidget : public Widget {
 public:
  MenuWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label);
  void Render() override;
  bool IsContainer() const override { return true; }
  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }

 private:
  std::string label_;
};

class MenuItemWidget : public Widget {
 public:
  MenuItemWidget(
      Realm* realm, v8::Local<v8::Object> object, const std::string& label, const std::string& shortcut, bool selected);
  void Render() override;
  const std::string& label() const { return label_; }
  void set_label(const std::string& l) { label_ = l; }
  bool selected() const { return selected_; }
  void set_selected(bool s) { selected_ = s; }
  bool clicked() const { return clicked_; }

 private:
  std::string label_;
  std::string shortcut_;
  bool selected_;
  bool clicked_ = false;
};

class PopupWidget : public Widget {
 public:
  PopupWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& id);
  void Render() override;
  bool IsContainer() const override { return true; }
  void DoOpen() { should_open_ = true; }
  void DoClose() { should_close_ = true; }

 private:
  std::string id_;
  bool should_open_ = false;
  bool should_close_ = false;
};

class ModalWidget : public Widget {
 public:
  ModalWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& title);
  void Render() override;
  bool IsContainer() const override { return true; }
  void DoOpen() { should_open_ = true; }
  void DoClose() { should_close_ = true; }
  bool is_open() const { return is_open_; }

 private:
  std::string title_;
  bool should_open_ = false;
  bool should_close_ = false;
  bool is_open_ = false;
};

class TooltipWidget : public Widget {
 public:
  TooltipWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& text);
  void Render() override;
  bool IsContainer() const override { return true; }
  const std::string& text() const { return text_; }
  void set_text(const std::string& t) { text_ = t; }

 private:
  std::string text_;
};

class TableWidget : public Widget {
 public:
  TableWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& id, int columns, ImGuiTableFlags flags);
  void Render() override;
  bool IsContainer() const override { return true; }
  void AddColumn(const std::string& name, ImGuiTableColumnFlags flags, float width);

 private:
  std::string id_;
  int columns_;
  ImGuiTableFlags flags_;
  struct ColumnDef {
    std::string name;
    ImGuiTableColumnFlags flags;
    float width;
  };
  std::vector<ColumnDef> column_defs_;
};

class TableRowWidget : public Widget {
 public:
  using Widget::Widget;
  void Render() override;
  bool IsContainer() const override { return true; }
};

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

class IndentWidget : public Widget {
 public:
  IndentWidget(Realm* realm, v8::Local<v8::Object> object, float width);
  void Render() override;
  bool IsContainer() const override { return true; }

 private:
  float width_;
};

class UnindentWidget : public Widget {
 public:
  UnindentWidget(Realm* realm, v8::Local<v8::Object> object, float width);
  void Render() override;
  bool IsContainer() const override { return true; }

 private:
  float width_;
};

class DummyWidget : public Widget {
 public:
  DummyWidget(Realm* realm, v8::Local<v8::Object> object, float width, float height);
  void Render() override;

 private:
  float width_, height_;
};

}  // namespace nyx
