#include "nyx/widget.h"

#include "nyx/env.h"

#include <algorithm>

namespace nyx {

using v8::Context;
using v8::Function;
using v8::Global;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::TryCatch;
using v8::Value;

Widget::Widget(Realm* realm, v8::Local<v8::Object> object) : BaseObject(realm, object) {
  ClearWeak();  // prevent GC by default; destroy() makes it weak again
}

Widget::~Widget() {
  ClearChildren();
}

void Widget::AddChild(Widget* child) {
  if (child->parent_) {
    child->parent_->RemoveChild(child);
  }
  child->parent_ = this;
  child->ClearWeak();
  children_.push_back(child);
}

void Widget::RemoveChild(Widget* child) {
  auto it = std::find(children_.begin(), children_.end(), child);
  if (it != children_.end()) {
    children_.erase(it);
    child->parent_ = nullptr;
    child->MakeWeak();
  }
}

void Widget::ClearChildren() {
  for (Widget* child : children_) {
    child->parent_ = nullptr;
    child->MakeWeak();
  }
  children_.clear();
}

void Widget::On(const std::string& event, Local<Function> callback) {
  event_handlers_[event].Reset(isolate(), callback);
}

void Widget::Off(const std::string& event) {
  auto it = event_handlers_.find(event);
  if (it != event_handlers_.end()) {
    it->second.Reset();
    event_handlers_.erase(it);
  }
}

void Widget::RenderChildren() {
  // Copy in case event handlers mutate the children vector
  auto snapshot = children_;
  for (Widget* child : snapshot) {
    if (child->visible_) {
      child->Render();
    }
  }
}

void Widget::EmitEvent(const std::string& event) {
  auto it = event_handlers_.find(event);
  if (it == event_handlers_.end()) return;

  Isolate* iso = isolate();
  HandleScope scope(iso);
  Local<Context> ctx = env()->context();
  Local<Function> fn = it->second.Get(iso);
  if (fn.IsEmpty()) return;

  TryCatchScope try_catch(iso);
  fn->Call(ctx, object(), 0, nullptr).IsEmpty();
}

void Widget::EmitEvent(const std::string& event, Local<Value> arg) {
  auto it = event_handlers_.find(event);
  if (it == event_handlers_.end()) return;

  Isolate* iso = isolate();
  HandleScope scope(iso);
  Local<Context> ctx = env()->context();
  Local<Function> fn = it->second.Get(iso);
  if (fn.IsEmpty()) return;

  TryCatchScope try_catch(iso);
  Local<Value> args[] = {arg};
  fn->Call(ctx, object(), 1, args).IsEmpty();
}

void WidgetManager::AddRoot(Widget* widget) {
  roots_.push_back(widget);
}

void WidgetManager::RemoveRoot(Widget* widget) {
  auto it = std::find(roots_.begin(), roots_.end(), widget);
  if (it != roots_.end()) {
    roots_.erase(it);
  }
}

void WidgetManager::RenderAll() {
  // Snapshot in case event handlers add/remove roots
  auto snapshot = roots_;
  for (Widget* root : snapshot) {
    if (root->visible()) {
      root->Render();
    }
  }
}

PanelWidget::PanelWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& title, ImGuiWindowFlags flags)
    : Widget(realm, object), title_(title), flags_(flags) {}

void PanelWidget::Render() {
  if (!open_) return;

  bool was_open = open_;
  panel_visible_ = ImGui::Begin(title_.c_str(), &open_, flags_);
  if (panel_visible_) {
    RenderChildren();
  }
  ImGui::End();

  if (was_open && !open_) {
    EmitEvent("close");
  }
}

TextWidget::TextWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& text)
    : Widget(realm, object), text_(text) {}

void TextWidget::Render() {
  ImGui::TextUnformatted(text_.c_str());
}

TextColoredWidget::TextColoredWidget(
    Realm* realm, v8::Local<v8::Object> object, const std::string& text, float r, float g, float b, float a)
    : Widget(realm, object), text_(text), r_(r), g_(g), b_(b), a_(a) {}

void TextColoredWidget::Render() {
  ImGui::TextColored(ImVec4(r_, g_, b_, a_), "%s", text_.c_str());
}

ButtonWidget::ButtonWidget(
    Realm* realm, v8::Local<v8::Object> object, const std::string& label, float width, float height)
    : Widget(realm, object), label_(label), width_(width), height_(height) {}

void ButtonWidget::Render() {
  clicked_ = ImGui::Button(label_.c_str(), ImVec2(width_, height_));
  if (clicked_) {
    EmitEvent("click");
  }
}

CheckboxWidget::CheckboxWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, bool checked)
    : Widget(realm, object), label_(label), checked_(checked) {}

void CheckboxWidget::Render() {
  bool old = checked_;
  ImGui::Checkbox(label_.c_str(), &checked_);
  if (checked_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", v8::Boolean::New(iso, checked_));
  }
}

SliderFloatWidget::SliderFloatWidget(
    Realm* realm, v8::Local<v8::Object> object, const std::string& label, float min, float max, float value)
    : Widget(realm, object), label_(label), value_(value), min_(min), max_(max) {}

void SliderFloatWidget::Render() {
  float old = value_;
  ImGui::SliderFloat(label_.c_str(), &value_, min_, max_);
  if (value_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", v8::Number::New(iso, value_));
  }
}

SliderIntWidget::SliderIntWidget(
    Realm* realm, v8::Local<v8::Object> object, const std::string& label, int min, int max, int value)
    : Widget(realm, object), label_(label), value_(value), min_(min), max_(max) {}

void SliderIntWidget::Render() {
  int old = value_;
  ImGui::SliderInt(label_.c_str(), &value_, min_, max_);
  if (value_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", v8::Integer::New(iso, value_));
  }
}

InputTextWidget::InputTextWidget(Realm* realm,
                                 v8::Local<v8::Object> object,
                                 const std::string& label,
                                 size_t max_length)
    : Widget(realm, object), label_(label), max_length_(max_length) {
  buffer_.reserve(max_length_);
}

void InputTextWidget::set_text(const std::string& t) {
  buffer_ = t;
  if (buffer_.size() > max_length_) {
    buffer_.resize(max_length_);
  }
}

void InputTextWidget::Render() {
  // ImGui::InputText needs a mutable char buffer with fixed capacity
  std::vector<char> buf(max_length_ + 1, '\0');
  std::copy(buffer_.begin(), buffer_.end(), buf.begin());

  if (ImGui::InputText(label_.c_str(), buf.data(), max_length_ + 1)) {
    std::string new_text(buf.data());
    if (new_text != buffer_) {
      buffer_ = std::move(new_text);
      Isolate* iso = isolate();
      HandleScope scope(iso);
      EmitEvent("change", v8::String::NewFromUtf8(iso, buffer_.c_str()).ToLocalChecked());
    }
  }
}

void SeparatorWidget::Render() {
  ImGui::Separator();
}

void SpacingWidget::Render() {
  ImGui::Spacing();
}

SameLineWidget::SameLineWidget(Realm* realm, v8::Local<v8::Object> object, float offset, float spacing)
    : Widget(realm, object), offset_(offset), spacing_(spacing) {}

void SameLineWidget::Render() {
  ImGui::SameLine(offset_, spacing_);
}

TreeNodeWidget::TreeNodeWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label)
    : Widget(realm, object), label_(label) {}

void TreeNodeWidget::Render() {
  open_ = ImGui::TreeNode(label_.c_str());
  if (open_) {
    RenderChildren();
    ImGui::TreePop();
  }
}

CollapsingHeaderWidget::CollapsingHeaderWidget(Realm* realm,
                                               v8::Local<v8::Object> object,
                                               const std::string& label,
                                               ImGuiTreeNodeFlags flags)
    : Widget(realm, object), label_(label), flags_(flags) {}

void CollapsingHeaderWidget::Render() {
  open_ = ImGui::CollapsingHeader(label_.c_str(), flags_);
  if (open_) {
    RenderChildren();
  }
}

TabBarWidget::TabBarWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& id)
    : Widget(realm, object), id_(id) {}

void TabBarWidget::Render() {
  if (ImGui::BeginTabBar(id_.c_str())) {
    RenderChildren();
    ImGui::EndTabBar();
  }
}

TabItemWidget::TabItemWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label)
    : Widget(realm, object), label_(label) {}

void TabItemWidget::Render() {
  selected_ = ImGui::BeginTabItem(label_.c_str());
  if (selected_) {
    RenderChildren();
    ImGui::EndTabItem();
  }
}

void DemoWindowWidget::Render() {
  bool show = true;
  ImGui::ShowDemoWindow(&show);
}

BulletTextWidget::BulletTextWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& text)
    : Widget(realm, object), text_(text) {}

void BulletTextWidget::Render() {
  ImGui::BulletText("%s", text_.c_str());
}

TextWrappedWidget::TextWrappedWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& text)
    : Widget(realm, object), text_(text) {}

void TextWrappedWidget::Render() {
  ImGui::TextWrapped("%s", text_.c_str());
}

TextDisabledWidget::TextDisabledWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& text)
    : Widget(realm, object), text_(text) {}

void TextDisabledWidget::Render() {
  ImGui::TextDisabled("%s", text_.c_str());
}

LabelTextWidget::LabelTextWidget(Realm* realm,
                                 v8::Local<v8::Object> object,
                                 const std::string& label,
                                 const std::string& text)
    : Widget(realm, object), label_(label), text_(text) {}

void LabelTextWidget::Render() {
  ImGui::LabelText(label_.c_str(), "%s", text_.c_str());
}

SeparatorTextWidget::SeparatorTextWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& text)
    : Widget(realm, object), text_(text) {}

void SeparatorTextWidget::Render() {
  ImGui::SeparatorText(text_.c_str());
}

void BulletWidget::Render() {
  ImGui::Bullet();
}

void NewLineWidget::Render() {
  ImGui::NewLine();
}

SmallButtonWidget::SmallButtonWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label)
    : Widget(realm, object), label_(label) {}

void SmallButtonWidget::Render() {
  clicked_ = ImGui::SmallButton(label_.c_str());
  if (clicked_) {
    EmitEvent("click");
  }
}

ArrowButtonWidget::ArrowButtonWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& id, ImGuiDir dir)
    : Widget(realm, object), id_(id), dir_(dir) {}

void ArrowButtonWidget::Render() {
  clicked_ = ImGui::ArrowButton(id_.c_str(), dir_);
  if (clicked_) {
    EmitEvent("click");
  }
}

RadioButtonWidget::RadioButtonWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, bool active)
    : Widget(realm, object), label_(label), active_(active) {}

void RadioButtonWidget::Render() {
  clicked_ = ImGui::RadioButton(label_.c_str(), active_);
  if (clicked_) {
    EmitEvent("click");
  }
}

DragFloatWidget::DragFloatWidget(Realm* realm,
                                 v8::Local<v8::Object> object,
                                 const std::string& label,
                                 float value,
                                 float speed,
                                 float min,
                                 float max)
    : Widget(realm, object), label_(label), value_(value), speed_(speed), min_(min), max_(max) {}

void DragFloatWidget::Render() {
  float old = value_;
  ImGui::DragFloat(label_.c_str(), &value_, speed_, min_, max_);
  if (value_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", v8::Number::New(iso, value_));
  }
}

DragIntWidget::DragIntWidget(
    Realm* realm, v8::Local<v8::Object> object, const std::string& label, int value, float speed, int min, int max)
    : Widget(realm, object), label_(label), value_(value), speed_(speed), min_(min), max_(max) {}

void DragIntWidget::Render() {
  int old = value_;
  ImGui::DragInt(label_.c_str(), &value_, speed_, min_, max_);
  if (value_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", v8::Integer::New(iso, value_));
  }
}

InputFloatWidget::InputFloatWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, float value)
    : Widget(realm, object), label_(label), value_(value) {}

void InputFloatWidget::Render() {
  float old = value_;
  ImGui::InputFloat(label_.c_str(), &value_);
  if (value_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", v8::Number::New(iso, value_));
  }
}

InputIntWidget::InputIntWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, int value)
    : Widget(realm, object), label_(label), value_(value) {}

void InputIntWidget::Render() {
  int old = value_;
  ImGui::InputInt(label_.c_str(), &value_);
  if (value_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", v8::Integer::New(iso, value_));
  }
}

InputTextMultilineWidget::InputTextMultilineWidget(
    Realm* realm, v8::Local<v8::Object> object, const std::string& label, size_t max_length, float width, float height)
    : Widget(realm, object), label_(label), max_length_(max_length), width_(width), height_(height) {
  buffer_.reserve(max_length_);
}

void InputTextMultilineWidget::set_text(const std::string& t) {
  buffer_ = t;
  if (buffer_.size() > max_length_) {
    buffer_.resize(max_length_);
  }
}

void InputTextMultilineWidget::Render() {
  std::vector<char> buf(max_length_ + 1, '\0');
  std::copy(buffer_.begin(), buffer_.end(), buf.begin());

  if (ImGui::InputTextMultiline(label_.c_str(), buf.data(), max_length_ + 1, ImVec2(width_, height_))) {
    std::string new_text(buf.data());
    if (new_text != buffer_) {
      buffer_ = std::move(new_text);
      Isolate* iso = isolate();
      HandleScope scope(iso);
      EmitEvent("change", v8::String::NewFromUtf8(iso, buffer_.c_str()).ToLocalChecked());
    }
  }
}

ColorEdit3Widget::ColorEdit3Widget(
    Realm* realm, v8::Local<v8::Object> object, const std::string& label, float r, float g, float b)
    : Widget(realm, object), label_(label) {
  col_[0] = r;
  col_[1] = g;
  col_[2] = b;
}

void ColorEdit3Widget::Render() {
  float old[3] = {col_[0], col_[1], col_[2]};
  ImGui::ColorEdit3(label_.c_str(), col_);
  if (col_[0] != old[0] || col_[1] != old[1] || col_[2] != old[2]) {
    EmitEvent("change");
  }
}

ColorEdit4Widget::ColorEdit4Widget(
    Realm* realm, v8::Local<v8::Object> object, const std::string& label, float r, float g, float b, float a)
    : Widget(realm, object), label_(label) {
  col_[0] = r;
  col_[1] = g;
  col_[2] = b;
  col_[3] = a;
}

void ColorEdit4Widget::Render() {
  float old[4] = {col_[0], col_[1], col_[2], col_[3]};
  ImGui::ColorEdit4(label_.c_str(), col_);
  if (col_[0] != old[0] || col_[1] != old[1] || col_[2] != old[2] || col_[3] != old[3]) {
    EmitEvent("change");
  }
}

ComboWidget::ComboWidget(
    Realm* realm, v8::Local<v8::Object> object, const std::string& label, std::vector<std::string> items, int selected)
    : Widget(realm, object), label_(label), items_(std::move(items)), selected_(selected) {}

void ComboWidget::Render() {
  const char* preview =
      (selected_ >= 0 && selected_ < static_cast<int>(items_.size())) ? items_[selected_].c_str() : "";
  if (ImGui::BeginCombo(label_.c_str(), preview)) {
    for (int i = 0; i < static_cast<int>(items_.size()); i++) {
      bool is_selected = (i == selected_);
      if (ImGui::Selectable(items_[i].c_str(), is_selected)) {
        if (selected_ != i) {
          selected_ = i;
          Isolate* iso = isolate();
          HandleScope scope(iso);
          EmitEvent("change", v8::Integer::New(iso, selected_));
        }
      }
      if (is_selected) ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
}

ListBoxWidget::ListBoxWidget(Realm* realm,
                             v8::Local<v8::Object> object,
                             const std::string& label,
                             std::vector<std::string> items,
                             int selected,
                             int height_items)
    : Widget(realm, object),
      label_(label),
      items_(std::move(items)),
      selected_(selected),
      height_items_(height_items) {}

void ListBoxWidget::Render() {
  // Build const char* array for ImGui
  std::vector<const char*> c_items;
  c_items.reserve(items_.size());
  for (const auto& s : items_) c_items.push_back(s.c_str());

  int old = selected_;
  ImGui::ListBox(label_.c_str(), &selected_, c_items.data(), static_cast<int>(c_items.size()), height_items_);
  if (selected_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", v8::Integer::New(iso, selected_));
  }
}

ProgressBarWidget::ProgressBarWidget(Realm* realm,
                                     v8::Local<v8::Object> object,
                                     float fraction,
                                     const std::string& overlay)
    : Widget(realm, object), fraction_(fraction), overlay_(overlay) {}

void ProgressBarWidget::Render() {
  ImGui::ProgressBar(fraction_, ImVec2(-FLT_MIN, 0), overlay_.empty() ? nullptr : overlay_.c_str());
}

SelectableWidget::SelectableWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label, bool selected)
    : Widget(realm, object), label_(label), selected_(selected) {}

void SelectableWidget::Render() {
  bool old = selected_;
  ImGui::Selectable(label_.c_str(), &selected_);
  if (selected_ != old) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", v8::Boolean::New(iso, selected_));
  }
}

ChildWidget::ChildWidget(Realm* realm,
                         v8::Local<v8::Object> object,
                         const std::string& id,
                         float width,
                         float height,
                         ImGuiChildFlags child_flags,
                         ImGuiWindowFlags window_flags)
    : Widget(realm, object),
      id_(id),
      width_(width),
      height_(height),
      child_flags_(child_flags),
      window_flags_(window_flags) {}

void ChildWidget::Render() {
  if (ImGui::BeginChild(id_.c_str(), ImVec2(width_, height_), child_flags_, window_flags_)) {
    RenderChildren();
  }
  ImGui::EndChild();
}

void GroupWidget::Render() {
  ImGui::BeginGroup();
  RenderChildren();
  ImGui::EndGroup();
}

DisabledWidget::DisabledWidget(Realm* realm, v8::Local<v8::Object> object, bool disabled)
    : Widget(realm, object), disabled_(disabled) {}

void DisabledWidget::Render() {
  ImGui::BeginDisabled(disabled_);
  RenderChildren();
  ImGui::EndDisabled();
}

void MenuBarWidget::Render() {
  if (ImGui::BeginMenuBar()) {
    RenderChildren();
    ImGui::EndMenuBar();
  }
}

MenuWidget::MenuWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& label)
    : Widget(realm, object), label_(label) {}

void MenuWidget::Render() {
  if (ImGui::BeginMenu(label_.c_str())) {
    RenderChildren();
    ImGui::EndMenu();
  }
}

MenuItemWidget::MenuItemWidget(
    Realm* realm, v8::Local<v8::Object> object, const std::string& label, const std::string& shortcut, bool selected)
    : Widget(realm, object), label_(label), shortcut_(shortcut), selected_(selected) {}

void MenuItemWidget::Render() {
  bool old_selected = selected_;
  clicked_ = ImGui::MenuItem(label_.c_str(), shortcut_.empty() ? nullptr : shortcut_.c_str(), &selected_);
  if (clicked_) {
    EmitEvent("click");
  }
  if (selected_ != old_selected) {
    Isolate* iso = isolate();
    HandleScope scope(iso);
    EmitEvent("change", v8::Boolean::New(iso, selected_));
  }
}

PopupWidget::PopupWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& id)
    : Widget(realm, object), id_(id) {}

void PopupWidget::Render() {
  if (should_open_) {
    ImGui::OpenPopup(id_.c_str());
    should_open_ = false;
  }
  if (ImGui::BeginPopup(id_.c_str())) {
    RenderChildren();
    if (should_close_) {
      ImGui::CloseCurrentPopup();
      should_close_ = false;
    }
    ImGui::EndPopup();
  }
}

ModalWidget::ModalWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& title)
    : Widget(realm, object), title_(title) {}

void ModalWidget::Render() {
  if (should_open_) {
    ImGui::OpenPopup(title_.c_str());
    should_open_ = false;
  }
  is_open_ = ImGui::BeginPopupModal(title_.c_str(), nullptr);
  if (is_open_) {
    RenderChildren();
    if (should_close_) {
      ImGui::CloseCurrentPopup();
      should_close_ = false;
    }
    ImGui::EndPopup();
  }
}

TooltipWidget::TooltipWidget(Realm* realm, v8::Local<v8::Object> object, const std::string& text)
    : Widget(realm, object), text_(text) {}

void TooltipWidget::Render() {
  if (ImGui::IsItemHovered()) {
    ImGui::BeginTooltip();
    if (!text_.empty()) {
      ImGui::TextUnformatted(text_.c_str());
    }
    RenderChildren();
    ImGui::EndTooltip();
  }
}

TableWidget::TableWidget(
    Realm* realm, v8::Local<v8::Object> object, const std::string& id, int columns, ImGuiTableFlags flags)
    : Widget(realm, object), id_(id), columns_(columns), flags_(flags) {}

void TableWidget::AddColumn(const std::string& name, ImGuiTableColumnFlags flags, float width) {
  column_defs_.push_back({name, flags, width});
}

void TableWidget::Render() {
  if (ImGui::BeginTable(id_.c_str(), columns_, flags_)) {
    // Setup columns
    for (const auto& col : column_defs_) {
      ImGui::TableSetupColumn(col.name.c_str(), col.flags, col.width);
    }
    if (!column_defs_.empty()) {
      ImGui::TableHeadersRow();
    }
    RenderChildren();
    ImGui::EndTable();
  }
}

void TableRowWidget::Render() {
  ImGui::TableNextRow();
  // Each child occupies the next column
  auto snapshot = children_;
  for (size_t i = 0; i < snapshot.size(); i++) {
    ImGui::TableSetColumnIndex(static_cast<int>(i));
    if (snapshot[i]->visible()) {
      snapshot[i]->Render();
    }
  }
}

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

IndentWidget::IndentWidget(Realm* realm, v8::Local<v8::Object> object, float width)
    : Widget(realm, object), width_(width) {}

void IndentWidget::Render() {
  ImGui::Indent(width_);
  RenderChildren();
  ImGui::Unindent(width_);
}

UnindentWidget::UnindentWidget(Realm* realm, v8::Local<v8::Object> object, float width)
    : Widget(realm, object), width_(width) {}

void UnindentWidget::Render() {
  ImGui::Unindent(width_);
  RenderChildren();
  ImGui::Indent(width_);
}

DummyWidget::DummyWidget(Realm* realm, v8::Local<v8::Object> object, float width, float height)
    : Widget(realm, object), width_(width), height_(height) {}

void DummyWidget::Render() {
  ImGui::Dummy(ImVec2(width_, height_));
}

}  // namespace nyx
