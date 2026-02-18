#include "nyx/env.h"
#include "nyx/gui/colors.h"
#include "nyx/gui/combo.h"
#include "nyx/gui/common.h"
#include "nyx/gui/image.h"
#include "nyx/gui/input.h"
#include "nyx/gui/layout.h"
#include "nyx/gui/listbox.h"
#include "nyx/gui/menus.h"
#include "nyx/gui/panel.h"
#include "nyx/gui/plotting.h"
#include "nyx/gui/popups.h"
#include "nyx/gui/selectable.h"
#include "nyx/gui/slider.h"
#include "nyx/gui/stack.h"
#include "nyx/gui/tables.h"
#include "nyx/gui/tabs.h"
#include "nyx/gui/text.h"
#include "nyx/gui/tooltip.h"
#include "nyx/gui/trees.h"
#include "nyx/gui/widget.h"
#include "nyx/gui/widget_manager.h"
#include "nyx/nyx_binding.h"
#include "nyx/util.h"

namespace nyx {

using v8::Boolean;
using v8::Context;
using v8::EscapableHandleScope;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::HandleScope;
using v8::Integer;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::ObjectTemplate;
using v8::String;
using v8::Value;

// forward declerations
void CreatePerIsolatePropertiesIO(IsolateData* isolate_data, Local<ObjectTemplate> target);

static void DemoWindowNew(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args.GetIsolate());
  DemoWindowWidget* demo = new DemoWindowWidget(env->principal_realm(), args.This());
  if (env->widget_manager()) {
    env->widget_manager()->AddRoot(demo);
  }
}

static void CreatePerIsolateProperties(IsolateData* isolate_data, Local<ObjectTemplate> target) {
  Isolate* isolate = isolate_data->isolate();
  HandleScope scope(isolate);

  // target->Set(FixedOneByteString(isolate, "Widget"), Widget::GetConstructorTemplate(isolate_data));

#define INIT_WIDGET_CONSTRUCTORS(V)                                                                                    \
  V(ChildWidget)                                                                                                       \
  V(ColorWidget)                                                                                                       \
  V(ComboWidget)                                                                                                       \
  V(ButtonWidget)                                                                                                      \
  V(CheckboxWidget)                                                                                                    \
  V(BulletWidget)                                                                                                      \
  V(SmallButtonWidget)                                                                                                 \
  V(ArrowButtonWidget)                                                                                                 \
  V(RadioButtonWidget)                                                                                                 \
  V(ProgressBarWidget)                                                                                                 \
  V(InputTextWidget)                                                                                                   \
  V(InputFloatWidget)                                                                                                  \
  V(InputIntWidget)                                                                                                    \
  V(InputTextMultilineWidget)                                                                                          \
  V(InputTextWithHintWidget)                                                                                           \
  V(SeparatorWidget)                                                                                                   \
  V(SpacingWidget)                                                                                                     \
  V(SameLineWidget)                                                                                                    \
  V(NewLineWidget)                                                                                                     \
  V(IndentWidget)                                                                                                      \
  V(UnindentWidget)                                                                                                    \
  V(DummyWidget)                                                                                                       \
  V(GroupWidget)                                                                                                       \
  V(DisabledWidget)                                                                                                    \
  V(ListBoxWidget)                                                                                                     \
  V(MainMenuBarWidget)                                                                                                 \
  V(MenuBarWidget)                                                                                                     \
  V(MenuWidget)                                                                                                        \
  V(MenuItemWidget)                                                                                                    \
  V(PanelWidget)                                                                                                       \
  V(PlotLinesWidget)                                                                                                   \
  V(PlotHistogramWidget)                                                                                               \
  V(PopupWidget)                                                                                                       \
  V(ModalWidget)                                                                                                       \
  V(SelectableWidget)                                                                                                  \
  V(SliderFloatWidget)                                                                                                 \
  V(SliderIntWidget)                                                                                                   \
  V(DragFloatWidget)                                                                                                   \
  V(DragIntWidget)                                                                                                     \
  V(StackWidget)                                                                                                       \
  V(TableWidget)                                                                                                       \
  V(TableRowWidget)                                                                                                    \
  V(TabBarWidget)                                                                                                      \
  V(TabItemWidget)                                                                                                     \
  V(TextWidget)                                                                                                        \
  V(TextColoredWidget)                                                                                                 \
  V(TextWrappedWidget)                                                                                                 \
  V(TextDisabledWidget)                                                                                                \
  V(LabelTextWidget)                                                                                                   \
  V(BulletTextWidget)                                                                                                  \
  V(SeparatorTextWidget)                                                                                               \
  V(TooltipWidget)                                                                                                     \
  V(TreeNodeWidget)                                                                                                    \
  V(CollapsingHeaderWidget)

#define V(name) name::Initialize(isolate_data, target);
  INIT_WIDGET_CONSTRUCTORS(V)
#undef V

  //
  //{
  //  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, DemoWindowNew);
  //  tmpl->SetClassName(OneByteString(isolate, "DemoWindow"));
  //  tmpl->InstanceTemplate()->SetInternalFieldCount(BaseObject::kInternalFieldCount);
  //  InstallWidgetMethods(isolate, tmpl->PrototypeTemplate());
  //  target->Set(OneByteString(isolate, "DemoWindow"), tmpl);
  //}

  CreatePerIsolatePropertiesIO(isolate_data, target);

  SetProperty(isolate, target, "fontSize", [](const FunctionCallbackInfo<Value>& args) {
    args.GetReturnValue().Set(ImGui::GetFontSize());
  });
}

static void CreatePerContextProperties(Local<Object> target, Local<Context> context) {}

NYX_BINDING_PER_ISOLATE_INIT(gui, CreatePerIsolateProperties)
NYX_BINDING_CONTEXT_AWARE(gui, CreatePerContextProperties)

}  // namespace nyx
