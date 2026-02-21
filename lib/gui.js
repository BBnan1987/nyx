'use strict';

const binding = internalBinding('gui');

const Key = {
  Tab: 512,
  LeftArrow: 513, RightArrow: 514, UpArrow: 515, DownArrow: 516,
  PageUp: 517, PageDown: 518,
  Home: 519, End: 520,
  Insert: 521, Delete: 522,
  Backspace: 523, Space: 524, Enter: 525, Escape: 526,
  LeftCtrl: 527, LeftShift: 528, LeftAlt: 529, LeftSuper: 530,
  RightCtrl: 531, RightShift: 532, RightAlt: 533, RightSuper: 534,
  Menu: 535,
  Num0: 536, Num1: 537, Num2: 538, Num3: 539, Num4: 540,
  Num5: 541, Num6: 542, Num7: 543, Num8: 544, Num9: 545,
  A: 546, B: 547, C: 548, D: 549, E: 550, F: 551, G: 552, H: 553, I: 554, J: 555,
  K: 556, L: 557, M: 558, N: 559, O: 560, P: 561, Q: 562, R: 563, S: 564, T: 565,
  U: 566, V: 567, W: 568, X: 569, Y: 570, Z: 571,
  F1: 572, F2: 573, F3: 574, F4: 575, F5: 576, F6: 577,
  F7: 578, F8: 579, F9: 580, F10: 581, F11: 582, F12: 583,
  F13: 584, F14: 585, F15: 586, F16: 587, F17: 588, F18: 589,
  F19: 590, F20: 591, F21: 592, F22: 593, F23: 594, F24: 595,
  Apostrophe: 596, Comma: 597, Minus: 598, Period: 599, Slash: 600,
  Semicolon: 601, Equal: 602, LeftBracket: 603, Backslash: 604, RightBracket: 605, GraveAccent: 606,
  CapsLock: 607, ScrollLock: 608, NumLock: 609, PrintScreen: 610, Pause: 611,
  Keypad0: 612, Keypad1: 613, Keypad2: 614, Keypad3: 615, Keypad4: 616,
  Keypad5: 617, Keypad6: 618, Keypad7: 619, Keypad8: 620, Keypad9: 621,
  KeypadDecimal: 622, KeypadDivide: 623, KeypadMultiply: 624,
  KeypadSubtract: 625, KeypadAdd: 626, KeypadEnter: 627, KeypadEqual: 628,
  AppBack: 629, AppForward: 630,
  GamepadStart: 631, GamepadBack: 632,
  GamepadFaceLeft: 633, GamepadFaceRight: 634, GamepadFaceUp: 635, GamepadFaceDown: 636,
  GamepadDpadLeft: 637, GamepadDpadRight: 638, GamepadDpadUp: 639, GamepadDpadDown: 640,
  GamepadL1: 641, GamepadR1: 642, GamepadL2: 643, GamepadR2: 644,
  GamepadL3: 645, GamepadR3: 646,
  GamepadLStickLeft: 647, GamepadLStickRight: 648, GamepadLStickUp: 649, GamepadLStickDown: 650,
  GamepadRStickLeft: 651, GamepadRStickRight: 652, GamepadRStickUp: 653, GamepadRStickDown: 654,

  ModNone: 0, ModCtrl: 1 << 12, ModShift: 1 << 13, ModAlt: 1 << 14, ModSuper: 1 << 15, ModShortcut: 1 << 11,
};

const MouseButton = {
  Left: 0,
  Right: 1,
  Middle: 2,
  X1: 3,
  X2: 4,
};

const io = binding.io;

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

const ColorEditFlags = {
  None: 0,

  NoAlpha: 1 << 1,
  NoPicker: 1 << 2,
  NoOptions: 1 << 3,
  NoSmallPreview: 1 << 4,
  NoInputs: 1 << 5,
  NoTooltip: 1 << 6,
  NoLabel: 1 << 7,
  NoSidePreview: 1 << 8,
  NoDragDrop: 1 << 9,
  NoBorder: 1 << 10,

  AlphaBar: 1 << 16,
  AlphaPreview: 1 << 17,
  AlphaPreviewHalf: 1 << 18,
  HDR: 1 << 19,
  DisplayRGB: 1 << 20,
  DisplayHSV: 1 << 21,
  DisplayHex: 1 << 22,
  Uint8: 1 << 23,
  Float: 1 << 24,
  PickerHueBar: 1 << 25,
  PickerHueWheel: 1 << 26,
  InputRGB: 1 << 27,
  InputHSV: 1 << 28,

  DefaultOptions: (1 << 23) | (1 << 20) | (1 << 27) | (1 << 25)
}

// must match colors.h
const ColorWidgetType = {
  ColorEdit3: 0,
  ColorEdit4: 1,
  ColorPicker3: 2,
  ColorPicker4: 3,
  ColorButton: 4,
};

class ColorEdit3 extends binding.ColorWidget {
  constructor(label, color, flags) {
    if (label !== undefined && typeof label !== 'string') { throw Error("expected string label"); }
    if (color !== undefined && typeof color !== 'object' && !(color instanceof Array)) { throw Error("expected object or array color"); }
    if (flags !== undefined && typeof flags !== 'number') { throw Error("expected number flags"); }
    super(label ?? '', ColorWidgetType.ColorEdit3, flags ?? 0, color ?? [0, 0, 0, 1], [0, 0, 0, 1], [0, 0]);
  }
}

class ColorEdit4 extends binding.ColorWidget {
  constructor(label, color, flags) {
    if (label !== undefined && typeof label !== 'string') { throw Error("expected string label"); }
    if (color !== undefined && typeof color !== 'object' && !(color instanceof Array)) { throw Error("expected object or array color"); }
    if (flags !== undefined && typeof flags !== 'number') { throw Error("expected number flags"); }
    super(label ?? '', ColorWidgetType.ColorEdit4, flags ?? 0, color ?? [0, 0, 0, 1], [0, 0, 0, 1], [0, 0]);
  }
}

class ColorPicker3 extends binding.ColorWidget {
  constructor(label, color, flags) {
    if (label !== undefined && typeof label !== 'string') { throw Error("expected string label"); }
    if (color !== undefined && typeof color !== 'object' && !(color instanceof Array)) { throw Error("expected object or array color"); }
    if (flags !== undefined && typeof flags !== 'number') { throw Error("expected number flags"); }
    super(label, ColorWidgetType.ColorPicker3, flags ?? 0, color ?? [0, 0, 0, 1], [0, 0, 0, 1], [0, 0]);
  }
}

class ColorPicker4 extends binding.ColorWidget {
  constructor(label, color, flags, refColor) {
    if (label !== undefined && typeof label !== 'string') { throw Error("expected string label"); }
    if (color !== undefined && typeof color !== 'object' && !(color instanceof Array)) { throw Error("expected object or array color"); }
    if (flags !== undefined && typeof flags !== 'number') { throw Error("expected number flags"); }
    if (refColor !== undefined && typeof refColor !== 'object' && !(refColor instanceof Array)) { throw Error("expected object or array refColor"); }
    super(label, ColorWidgetType.ColorPicker4, flags ?? 0, color ?? [0, 0, 0, 1], refColor ?? [0, 0, 0, 1], [0, 0]);
  }
}

class ColorButton extends binding.ColorWidget {
  constructor(label, color, flags, size) {
    if (label !== undefined && typeof label !== 'string') { throw Error("expected string label"); }
    if (color !== undefined && typeof color !== 'object' && !(color instanceof Array)) { throw Error("expected object or array color"); }
    if (flags !== undefined && typeof flags !== 'number') { throw Error("expected number flags"); }
    if (size !== undefined && typeof size !== 'object' && !(size instanceof Array)) { throw Error("expected object or array size"); }
    super(label, ColorWidgetType.ColorPicker4, flags ?? 0, color ?? [0, 0, 0, 1], [0, 0, 0, 1], size ?? [0, 0]);
    on('click', clicked => { this.clicked = clicked; });
  }
}

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

class InputText extends binding.InputTextWidget {
  constructor(label, maxLength, text, flags) {
    super(label, text, maxLength, flags);
  }
}

class InputTextMultiline extends binding.InputTextWidget {
  constructor(label, maxLength, text, flags) {
    super(label, text, maxLength, flags);
    this.multiLine = true;
  }
}

class InputTextWithHint extends binding.InputTextWidget {
  constructor(label, maxLength, text, hint, flags) {
    text ??= '';
    maxLength ??= 256;
    hint ??= '';
    flags ??= 0;
    super(label, text, maxLength, flags);
    this.hint = hint;
  }
}

class InputFloat extends binding.InputNumberWidget {
  constructor(label, value, step, stepFast, format, flags) {
    value ??= 0.0; step ??= 0.0; stepFast ??= 0.0; format ??= "%.3f"; flags ??= 0;
    super(label, 0 /* Float */, 1, [value], step, stepFast, format, flags);
  }
}

class InputFloat2 extends binding.InputNumberWidget {
  constructor(label, value, step, stepFast, format, flags) {
    value ??= [0.0, 0.0]; step ??= 0.0; stepFast ??= 0.0; format ??= "%.3f"; flags ??= 0;
    super(label, 0 /* Float */, 2, value, step, stepFast, format, flags);
  }
}

class InputFloat3 extends binding.InputNumberWidget {
  constructor(label, value, step, stepFast, format, flags) {
    value ??= [0.0, 0.0, 0.0]; step ??= 0.0; stepFast ??= 0.0; format ??= "%.3f"; flags ??= 0;
    super(label, 0 /* Float */, 3, value, step, stepFast, format, flags);
  }
}

class InputFloat4 extends binding.InputNumberWidget {
  constructor(label, value, step, stepFast, format, flags) {
    value ??= [0.0, 0.0, 0.0, 0.0]; step ??= 0.0; stepFast ??= 0.0; format ??= "%.3f"; flags ??= 0;
    super(label, 0 /* Float */, 4, value, step, stepFast, format, flags);
  }
}

class InputInt extends binding.InputNumberWidget {
  constructor(label, value, step, stepFast, format, flags) {
    value ??= 0; step ??= 1; stepFast ??= 100; format ??= "%d"; flags ??= 0;
    super(label, 1 /* Int */, 1, [value], step, stepFast, format, flags);
  }
}

class InputInt2 extends binding.InputNumberWidget {
  constructor(label, value, step, stepFast, format, flags) {
    value ??= [0, 0]; step ??= 1; stepFast ??= 100; format ??= "%d"; flags ??= 0;
    super(label, 1 /* Int */, 2, value, step, stepFast, format, flags);
  }
}

class InputInt3 extends binding.InputNumberWidget {
  constructor(label, value, step, stepFast, format, flags) {
    value ??= [0, 0, 0]; step ??= 1; stepFast ??= 100; format ??= "%d"; flags ??= 0;
    super(label, 1 /* Int */, 3, value, step, stepFast, format, flags);
  }
}

class InputInt4 extends binding.InputNumberWidget {
  constructor(label, value, step, stepFast, format, flags) {
    value ??= [0, 0, 0, 0]; step ??= 1; stepFast ??= 100; format ??= "%d"; flags ??= 0;
    super(label, 1 /* Int */, 4, value, step, stepFast, format, flags);
  }
}

class InputDouble extends binding.InputNumberWidget {
  constructor(label, value, step, stepFast, format, flags) {
    value ??= 0.0; step ??= 0.0; stepFast ??= 0.0; format ??= "%.6f"; flags ??= 0;
    super(label, 2 /* Double */, 1, [value], step, stepFast, format, flags);
  }
}

module.exports = {
  ColorEditFlags,
  ColorEdit3,
  ColorEdit4,
  ColorPicker3,
  ColorPicker4,
  ColorButton,

  InputText: InputText,
  InputTextMultiline: InputTextMultiline,
  InputTextWithHint: InputTextWithHint,
  InputFloat: InputFloat,
  InputFloat2: InputFloat2,
  InputFloat3: InputFloat3,
  InputFloat4: InputFloat4,
  InputInt: InputInt,
  InputInt2: InputInt2,
  InputInt3: InputInt3,
  InputInt4: InputInt4,
  InputDouble: InputDouble,

  Panel: binding.Panel,
  Text: binding.Text,
  TextColored: binding.TextColored,
  Button: binding.Button,
  Checkbox: Checkbox,
  CheckboxFlags: CheckboxFlags,
  SliderFloat: binding.SliderFloat,
  SliderInt: binding.SliderInt,
  Slider: binding.SliderFloat,
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

  Key,
  MouseButton,

  ConfigFlags,
  BackendFlags,
  WindowFlags,
  TreeNodeFlags,
  TableFlags,
  ChildFlags,
  Dir,
  StyleColor,
  StyleVar,

  io,
  fontSize: binding.fontSize,

  background: binding.background,
  foreground: binding.foreground,
};
