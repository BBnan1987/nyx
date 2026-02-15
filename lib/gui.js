'use strict';

const binding = internalBinding('gui');

const ConfigFlags = {
  None: 0,
  NavEnableKeyboard: 1 << 0,   // Master keyboard navigation enable flag. Enable full Tabbing + directional arrows + space/enter to activate.
  NavEnableGamepad: 1 << 1,   // Master gamepad navigation enable flag. Backend also needs to set ImGuiBackendFlags_HasGamepad.
  NavEnableSetMousePos: 1 << 2,   // Instruct navigation to move the mouse cursor. May be useful on TV/console systems where moving a virtual mouse is awkward. Will update io.MousePos and set io.WantSetMousePos=true. If enabled you MUST honor io.WantSetMousePos requests in your backend, otherwise ImGui will react as if the mouse is jumping around back and forth.
  NavNoCaptureKeyboard: 1 << 3,   // Instruct navigation to not set the io.WantCaptureKeyboard flag when io.NavActive is set.
  NoMouse: 1 << 4,   // Instruct imgui to clear mouse position/buttons in NewFrame(). This allows ignoring the mouse information set by the backend.
  NoMouseCursorChange: 1 << 5,   // Instruct backend to not alter mouse cursor shape and visibility. Use if the backend cursor changes are interfering with yours and you don't want to use SetMouseCursor() to change mouse cursor. You may want to honor requests from imgui by reading GetMouseCursor() yourself instead.

  // User storage (to allow your backend/engine to communicate to code that may be shared between multiple projects. Those flags are NOT used by core Dear ImGui)
  IsSRGB: 1 << 20,  // Application is SRGB-aware.
  IsTouchScreen: 1 << 21,  // Application is using a touch screen instead of a mouse.
};

const BackendFlags = {
  None: 0,
  HasGamepad: 1 << 0,   // Backend Platform supports gamepad and currently has one connected.
  HasMouseCursors: 1 << 1,   // Backend Platform supports honoring GetMouseCursor() value to change the OS cursor shape.
  HasSetMousePos: 1 << 2,   // Backend Platform supports io.WantSetMousePos requests to reposition the OS mouse position (only used if ImGuiConfigFlags_NavEnableSetMousePos is set).
  RendererHasVtxOffset: 1 << 3,   // Backend Renderer supports ImDrawCmd::VtxOffset. This enables output of large meshes (64K+ vertices) while still using 16-bit indices.
};

const WindowFlags = {
  None: 0,
  NoTitleBar: 1 << 0,
  NoResize: 1 << 1,
  NoMove: 1 << 2,
  NoScrollbar: 1 << 3,
  NoScrollWithMouse: 1 << 4,
  NoCollapse: 1 << 5,
  AlwaysAutoResize: 1 << 6,
  NoBackground: 1 << 7,
  NoSavedSettings: 1 << 8,
  NoMouseInputs: 1 << 9,
  MenuBar: 1 << 10,
  HorizontalScrollbar: 1 << 11,
  NoFocusOnAppearing: 1 << 12,
  NoBringToFrontOnFocus: 1 << 13,
  AlwaysVerticalScrollbar: 1 << 14,
  AlwaysHorizontalScrollbar: 1 << 15,
  NoNavInputs: 1 << 16,
  NoNavFocus: 1 << 17,
  UnsavedDocument: 1 << 18,
  NoNav: (1 << 16) | (1 << 17),
  NoDecoration: (1 << 0) | (1 << 1) | (1 << 3) | (1 << 5),
  NoInputs: (1 << 9) | (1 << 16) | (1 << 17),
};

const TreeNodeFlags = {
  None: 0,
  Selected: 1 << 0,
  Framed: 1 << 1,
  AllowOverlap: 1 << 2,
  NoTreePushOnOpen: 1 << 3,
  NoAutoOpenOnLog: 1 << 4,
  DefaultOpen: 1 << 5,
  OpenOnDoubleClick: 1 << 6,
  OpenOnArrow: 1 << 7,
  Leaf: 1 << 8,
  Bullet: 1 << 9,
  FramePadding: 1 << 10,
  SpanAvailWidth: 1 << 11,
  SpanFullWidth: 1 << 12,
  SpanTextWidth: 1 << 13,
  SpanAllColumns: 1 << 14,
  NavLeftJumpsBackHere: 1 << 15,
  CollapsingHeader: (1 << 1) | (1 << 3) | (1 << 4),
};

const Dir = {
  None: -1,
  Left: 0,
  Right: 1,
  Up: 2,
  Down: 3,
};

const TableFlags = {
  None: 0,
  Resizable: 1 << 0,
  Reorderable: 1 << 1,
  Hideable: 1 << 2,
  Sortable: 1 << 3,
  NoSavedSettings: 1 << 4,
  ContextMenuInBody: 1 << 5,
  RowBg: 1 << 6,
  BordersInnerH: 1 << 7,
  BordersOuterH: 1 << 8,
  BordersInnerV: 1 << 9,
  BordersOuterV: 1 << 10,
  BordersH: (1 << 7) | (1 << 8),
  BordersV: (1 << 9) | (1 << 10),
  Borders: (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10),
  NoBordersInBody: 1 << 11,
  NoBordersInBodyUntilResize: 1 << 12,
  SizingFixedFit: 1 << 13,
  SizingFixedSame: 2 << 13,
  SizingStretchProp: 3 << 13,
  SizingStretchSame: 4 << 13,
  NoHostExtendX: 1 << 16,
  NoHostExtendY: 1 << 17,
  NoKeepColumnsVisible: 1 << 18,
  PreciseWidths: 1 << 19,
  NoClip: 1 << 20,
  PadOuterX: 1 << 21,
  NoPadOuterX: 1 << 22,
  NoPadInnerX: 1 << 23,
  ScrollX: 1 << 24,
  ScrollY: 1 << 25,
  SortMulti: 1 << 26,
  SortTristate: 1 << 27,
};

const ChildFlags = {
  None: 0,
  Borders: 1 << 0,
  AlwaysUseWindowPadding: 1 << 1,
  ResizeX: 1 << 2,
  ResizeY: 1 << 3,
  AutoResizeX: 1 << 4,
  AutoResizeY: 1 << 5,
  AlwaysAutoResize: 1 << 6,
  FrameStyle: 1 << 7,
};

const StyleColor = {
  Text: 0,
  TextDisabled: 1,
  WindowBg: 2,
  ChildBg: 3,
  PopupBg: 4,
  Border: 5,
  BorderShadow: 6,
  FrameBg: 7,
  FrameBgHovered: 8,
  FrameBgActive: 9,
  TitleBg: 10,
  TitleBgActive: 11,
  TitleBgCollapsed: 12,
  MenuBarBg: 13,
  ScrollbarBg: 14,
  ScrollbarGrab: 15,
  ScrollbarGrabHovered: 16,
  ScrollbarGrabActive: 17,
  CheckMark: 18,
  SliderGrab: 19,
  SliderGrabActive: 20,
  Button: 21,
  ButtonHovered: 22,
  ButtonActive: 23,
  Header: 24,
  HeaderHovered: 25,
  HeaderActive: 26,
  Separator: 27,
  SeparatorHovered: 28,
  SeparatorActive: 29,
  ResizeGrip: 30,
  ResizeGripHovered: 31,
  ResizeGripActive: 32,
  Tab: 33,
  TabHovered: 34,
  TabActive: 35,
  TabUnfocused: 36,
  TabUnfocusedActive: 37,
  PlotLines: 38,
  PlotLinesHovered: 39,
  PlotHistogram: 40,
  PlotHistogramHovered: 41,
  TableHeaderBg: 42,
  TableBorderStrong: 43,
  TableBorderLight: 44,
  TableRowBg: 45,
  TableRowBgAlt: 46,
  TextSelectedBg: 47,
  DragDropTarget: 48,
  NavHighlight: 49,
  NavWindowingHighlight: 50,
  NavWindowingDimBg: 51,
  ModalWindowDimBg: 52,
};

const StyleVar = {
  Alpha: 0,                   // float     
  DisabledAlpha: 1,           // float     
  WindowPadding: 2,           // ImVec2    
  WindowRounding: 3,          // float     
  WindowBorderSize: 4,        // float     
  WindowMinSize: 5,           // ImVec2    
  WindowTitleAlign: 6,        // ImVec2    
  ChildRounding: 7,           // float     
  ChildBorderSize: 8,         // float     
  PopupRounding: 9,           // float     
  PopupBorderSize: 10,        // float     
  FramePadding: 11,           // ImVec2    
  FrameRounding: 12,          // float     
  FrameBorderSize: 13,        // float     
  ItemSpacing: 14,            // ImVec2    
  ItemInnerSpacing: 15,       // ImVec2    
  IndentSpacing: 16,          // float     
  CellPadding: 17,            // ImVec2    
  ScrollbarSize: 18,          // float     
  ScrollbarRounding: 19,      // float     
  GrabMinSize: 20,            // float     
  GrabRounding: 21,           // float     
  TabRounding: 22,            // float     
  TabBorderSize: 23,          // float     
  TabBarBorderSize: 24,       // float     
  TableAngledHeadersAngle: 25,// float  
  ButtonTextAlign: 26,        // ImVec2    
  SelectableTextAlign: 27,    // ImVec2    
  SeparatorTextBorderSize: 28,// float  
  SeparatorTextAlign: 29,     // ImVec2    
  SeparatorTextPadding: 30,   // ImVec2    
};

class Checkbox extends binding.Checkbox {
  constructor(label, obj, property) {
    if (typeof obj !== 'object' || typeof obj[property] !== 'boolean') {
      throw new Error(`Checkbox requires an object and boolean property`);
    }
    super(label, obj[property]);
    this._obj = obj;
    this._property = property;
    this.on('change', checked => {
      this._obj[this._property] = checked;
    });
  }
}

class CheckboxFlags extends binding.Checkbox {
  constructor(label, obj, property, flags) {
    super(label, (obj[property] & flags) !== 0);
    this._obj = obj;
    this._property = property;
    this.flags = flags;
    this.on('change', checked => {
      if (checked) {
        this._obj[this._property] |= this.flags;
      } else {
        this._obj[this._property] &= ~this.flags;
      }
    });
  }
}

class RadioButton extends binding.RadioButton {
  constructor(label, obj, property, value) {
    if (typeof obj !== 'object' || typeof obj[property] !== typeof value) {
      throw new Error(`Checkbox requires an object and boolean property`);
    }
    super(label, obj[property] === value);
    this._obj = obj;
    this._property = property;
    this._value = value;
    this.on('click', _ => {
      this._obj[this._property] = this._value;
    });
    this.on('update', _ => {
      this.active = this._obj[this._property] === value;
    });
  }
}

module.exports = {
  Panel: binding.Panel,
  Text: binding.Text,
  TextColored: binding.TextColored,
  Button: binding.Button,
  Checkbox: Checkbox,
  CheckboxFlags: CheckboxFlags,
  SliderFloat: binding.SliderFloat,
  SliderInt: binding.SliderInt,
  Slider: binding.SliderFloat,
  InputText: binding.InputText,
  Separator: binding.Separator,
  Spacing: binding.Spacing,
  SameLine: binding.SameLine,
  TreeNode: binding.TreeNode,
  CollapsingHeader: binding.CollapsingHeader,
  TabBar: binding.TabBar,
  TabItem: binding.TabItem,
  DemoWindow: binding.DemoWindow,

  BulletText: binding.BulletText,
  TextWrapped: binding.TextWrapped,
  TextDisabled: binding.TextDisabled,
  LabelText: binding.LabelText,
  SeparatorText: binding.SeparatorText,
  Bullet: binding.Bullet,
  NewLine: binding.NewLine,

  SmallButton: binding.SmallButton,
  ArrowButton: binding.ArrowButton,
  RadioButton: RadioButton,

  DragFloat: binding.DragFloat,
  DragInt: binding.DragInt,

  InputFloat: binding.InputFloat,
  InputInt: binding.InputInt,
  InputTextMultiline: binding.InputTextMultiline,

  ColorEdit3: binding.ColorEdit3,
  ColorEdit4: binding.ColorEdit4,

  Combo: binding.Combo,
  ListBox: binding.ListBox,

  ProgressBar: binding.ProgressBar,
  Selectable: binding.Selectable,

  Child: binding.Child,
  Group: binding.Group,
  Disabled: binding.Disabled,

  MainMenuBar: binding.MainMenuBar,
  MenuBar: binding.MenuBar,
  Menu: binding.Menu,
  MenuItem: binding.MenuItem,

  Popup: binding.Popup,
  Modal: binding.Modal,

  Tooltip: binding.Tooltip,

  Table: binding.Table,
  TableRow: binding.TableRow,

  PlotLines: binding.PlotLines,
  PlotHistogram: binding.PlotHistogram,

  Indent: binding.Indent,
  Unindent: binding.Unindent,
  Dummy: binding.Dummy,

  Stack: binding.Stack,

  ConfigFlags,
  BackendFlags,
  WindowFlags,
  TreeNodeFlags,
  TableFlags,
  ChildFlags,
  Dir,
  StyleColor,
  StyleVar,

  io: binding.io,
  fontSize: binding.fontSize,
};
