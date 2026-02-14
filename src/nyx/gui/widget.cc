#include "nyx/gui/widget.h"

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

void DemoWindowWidget::Render() {
  bool show = true;
  ImGui::ShowDemoWindow(&show);
}

}  // namespace nyx
