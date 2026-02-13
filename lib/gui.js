'use strict';

const binding = internalBinding('gui');

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

module.exports = {
  Panel: binding.Panel,
  Text: binding.Text,
  TextColored: binding.TextColored,
  Button: binding.Button,
  Checkbox: binding.Checkbox,
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
  RadioButton: binding.RadioButton,

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

  WindowFlags,
  TreeNodeFlags,
  TableFlags,
  ChildFlags,
  Dir,
};
