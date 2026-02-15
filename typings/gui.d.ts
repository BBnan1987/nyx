declare module 'gui' {
  export const ConfigFlags: {
    readonly None: number;
    readonly NavEnableKeyboard: number;
    readonly NavEnableGamepad: number;
    readonly NavEnableSetMousePos: number;
    readonly NavNoCaptureKeyboard: number;
    readonly NoMouse: number;
    readonly NoMouseCursorChange: number;

    readonly IsSRGB: number;
    readonly IsTouchScreen: number;
  };

  export const BackendFlags: {
    readonly None: number;
    readonly HasGamepad: number;   // Backend Platform supports gamepad and currently has one connected.
    readonly HasMouseCursors: number;   // Backend Platform supports honoring GetMouseCursor() value to change the OS cursor shape.
    readonly HasSetMousePos: number;   // Backend Platform supports io.WantSetMousePos requests to reposition the OS mouse position (only used if ImGuiConfigFlags_NavEnableSetMousePos is set).
    readonly RendererHasVtxOffset: number;   // Backend Renderer supports ImDrawCmd::VtxOffset. This enables output of large meshes (64K+ vertices) while still using 16-bit indices.
  };

  // Window flags
  export const WindowFlags: {
    readonly None: 0;
    readonly NoTitleBar: number;
    readonly NoResize: number;
    readonly NoMove: number;
    readonly NoScrollbar: number;
    readonly NoScrollWithMouse: number;
    readonly NoCollapse: number;
    readonly AlwaysAutoResize: number;
    readonly NoBackground: number;
    readonly NoSavedSettings: number;
    readonly NoMouseInputs: number;
    readonly MenuBar: number;
    readonly HorizontalScrollbar: number;
    readonly NoFocusOnAppearing: number;
    readonly NoBringToFrontOnFocus: number;
    readonly AlwaysVerticalScrollbar: number;
    readonly AlwaysHorizontalScrollbar: number;
    readonly NoNavInputs: number;
    readonly NoNavFocus: number;
    readonly UnsavedDocument: number;
    readonly NoNav: number;
    readonly NoDecoration: number;
    readonly NoInputs: number;
  };

  // Tree node flags
  export const TreeNodeFlags: {
    readonly None: 0;
    readonly Selected: number;
    readonly Framed: number;
    readonly AllowOverlap: number;
    readonly NoTreePushOnOpen: number;
    readonly NoAutoOpenOnLog: number;
    readonly DefaultOpen: number;
    readonly OpenOnDoubleClick: number;
    readonly OpenOnArrow: number;
    readonly Leaf: number;
    readonly Bullet: number;
    readonly FramePadding: number;
    readonly SpanAvailWidth: number;
    readonly SpanFullWidth: number;
    readonly SpanTextWidth: number;
    readonly SpanAllColumns: number;
    readonly NavLeftJumpsBackHere: number;
    readonly CollapsingHeader: number;
  };

  // Table flags
  export const TableFlags: {
    readonly None: 0;
    readonly Resizable: number;
    readonly Reorderable: number;
    readonly Hideable: number;
    readonly Sortable: number;
    readonly NoSavedSettings: number;
    readonly ContextMenuInBody: number;
    readonly RowBg: number;
    readonly BordersInnerH: number;
    readonly BordersOuterH: number;
    readonly BordersInnerV: number;
    readonly BordersOuterV: number;
    readonly BordersH: number;
    readonly BordersV: number;
    readonly Borders: number;
    readonly NoBordersInBody: number;
    readonly NoBordersInBodyUntilResize: number;
    readonly SizingFixedFit: number;
    readonly SizingFixedSame: number;
    readonly SizingStretchProp: number;
    readonly SizingStretchSame: number;
    readonly NoHostExtendX: number;
    readonly NoHostExtendY: number;
    readonly NoKeepColumnsVisible: number;
    readonly PreciseWidths: number;
    readonly NoClip: number;
    readonly PadOuterX: number;
    readonly NoPadOuterX: number;
    readonly NoPadInnerX: number;
    readonly ScrollX: number;
    readonly ScrollY: number;
    readonly SortMulti: number;
    readonly SortTristate: number;
  };

  // Child flags
  export const ChildFlags: {
    readonly None: 0;
    readonly Borders: number;
    readonly AlwaysUseWindowPadding: number;
    readonly ResizeX: number;
    readonly ResizeY: number;
    readonly AutoResizeX: number;
    readonly AutoResizeY: number;
    readonly AlwaysAutoResize: number;
    readonly FrameStyle: number;
  };

  // Direction enum
  export const Dir: {
    readonly None: -1;
    readonly Left: 0;
    readonly Right: 1;
    readonly Up: 2;
    readonly Down: 3;
  };

  export const StyleColor: {
    readonly Text: number;
    readonly TextDisabled: number;
    readonly WindowBg: number;
    readonly ChildBg: number;
    readonly PopupBg: number;
    readonly Border: number;
    readonly BorderShadow: number;
    readonly FrameBg: number;
    readonly FrameBgHovered: number;
    readonly FrameBgActive: number;
    readonly TitleBg: number;
    readonly TitleBgActive: number;
    readonly TitleBgCollapsed: number;
    readonly MenuBarBg: number;
    readonly ScrollbarBg: number;
    readonly ScrollbarGrab: number;
    readonly ScrollbarGrabHovered: number;
    readonly ScrollbarGrabActive: number;
    readonly CheckMark: number;
    readonly SliderGrab: number;
    readonly SliderGrabActive: number;
    readonly Button: number;
    readonly ButtonHovered: number;
    readonly ButtonActive: number;
    readonly Header: number;
    readonly HeaderHovered: number;
    readonly HeaderActive: number;
    readonly Separator: number;
    readonly SeparatorHovered: number;
    readonly SeparatorActive: number;
    readonly ResizeGrip: number;
    readonly ResizeGripHovered: number;
    readonly ResizeGripActive: number;
    readonly Tab: number;
    readonly TabHovered: number;
    readonly TabActive: number;
    readonly TabUnfocused: number;
    readonly TabUnfocusedActive: number;
    readonly PlotLines: number;
    readonly PlotLinesHovered: number;
    readonly PlotHistogram: number;
    readonly PlotHistogramHovered: number;
    readonly TableHeaderBg: number;
    readonly TableBorderStrong: number;
    readonly TableBorderLight: number;
    readonly TableRowBg: number;
    readonly TableRowBgAlt: number;
    readonly TextSelectedBg: number;
    readonly DragDropTarget: number;
    readonly NavHighlight: number;
    readonly NavWindowingHighlight: number;
    readonly NavWindowingDimBg: number;
    readonly ModalWindowDimBg: number;
  };

  export const StyleVar: {
    readonly Alpha: number;
    readonly DisabledAlpha: number;
    readonly WindowPadding: number;
    readonly WindowRounding: number;
    readonly WindowBorderSize: number;
    readonly WindowMinSize: number;
    readonly WindowTitleAlign: number;
    readonly ChildRounding: number;
    readonly ChildBorderSize: number;
    readonly PopupRounding: number;
    readonly PopupBorderSize: number;
    readonly FramePadding: number;
    readonly FrameRounding: number;
    readonly FrameBorderSize: number;
    readonly ItemSpacing: number;
    readonly ItemInnerSpacing: number;
    readonly IndentSpacing: number;
    readonly CellPadding: number;
    readonly ScrollbarSize: number;
    readonly ScrollbarRounding: number;
    readonly GrabMinSize: number;
    readonly GrabRounding: number;
    readonly TabRounding: number;
    readonly TabBorderSize: number;
    readonly TabBarBorderSize: number;
    readonly TableAngledHeadersAngle: number;
    readonly ButtonTextAlign: number;
    readonly SelectableTextAlign: number;
    readonly SeparatorTextBorderSize: number;
    readonly SeparatorTextAlign: number;
    readonly SeparatorTextPadding: number;
  };

  export interface IO {
    // Main display size
    readonly displaySize: { x: number; y: number };
    readonly displayFramebufferScale: { x: number; y: number };

    // Time
    readonly deltaTime: number;

    // Mouse state
    readonly mousePos: { x: number; y: number };
    readonly mouseDown: readonly boolean[];
    readonly mouseWheel: number;
    readonly mouseWheelH: number;

    // Keyboard modifiers
    readonly keyCtrl: boolean;
    readonly keyShift: boolean;
    readonly keyAlt: boolean;
    readonly keySuper: boolean;

    // Input capture flags
    readonly wantCaptureMouse: boolean;
    readonly wantCaptureKeyboard: boolean;
    readonly wantTextInput: boolean;
    readonly wantSetMousePos: boolean;
    readonly wantSaveIniSettings: boolean;

    // Navigation flags
    readonly navActive: boolean;
    readonly navVisible: boolean;

    // Framerate
    readonly framerate: number;

    // Performance metrics
    readonly metricsRenderVertices: number;
    readonly metricsRenderIndices: number;
    readonly metricsRenderWindows: number;
    readonly metricsActiveWindows: number;

    // Font configuration
    fontGlobalScale: number;
    fontAllowUserScaling: boolean;

    // Settings
    mouseDoubleClickTime: number;
    mouseDoubleClickMaxDist: number;
    keyRepeatDelay: number;
    keyRepeatRate: number;

    // Config flags
    configFlags: number;
    backendFlags: number;

    backendFlags: boolean;
    mouseDrawCursor: boolean;
    configInputTextCursorBlink: boolean;
    configInputTextEnterKeepActive: boolean;
    configDragClickToInputText: boolean;
    configWindowsResizeFromEdges: boolean;
    configWindowsMoveFromTitleBarOnly: boolean;
    configMacOSXBehaviors: boolean;
    configDebugIsDebuggerPresent: boolean;
    configDebugBeginReturnValueOnce: boolean;
    configDebugBeginReturnValueLoop: boolean;
    configDebugIgnoreFocusLoss: boolean;
    configDebugIniSettings: boolean;
  }

  // ImGui IO singleton
  export const io: IO;

  // Base Widget interface
  export interface Widget {
    /**
     * Add a child widget
     */
    add(child: Widget): Widget;

    /**
     * Remove a child widget
     */
    remove(child: Widget): void;

    /**
     * Register an event handler
     */
    on(event: string, handler: (...args: any[]) => void): void;

    /**
     * Remove an event handler
     */
    off(event: string): void;

    /**
     * Destroy the widget
     */
    destroy(): void;

    /**
     * Widget visibility
     */
    visible: boolean;
  }

  // Widget constructor types
  export const Panel: new (title?: string, flags?: number) => Widget & {
    open: boolean;
    title: string;
    flags: number;
  };

  export const Text: new (text?: string) => Widget & {
    text: string;
  };

  export const TextColored: new (text?: string, r?: number, g?: number, b?: number, a?: number) => Widget & {
    text: string;
  };

  export const Button: new (label?: string) => Widget & {
    readonly clicked: boolean;
    label: string;
  };

  export const InvisibleButton: new (label?: string) => Widget & {
    readonly clicked: boolean;
    label: string;
  };

  export const Checkbox: new (label?: string, obj: object, property: string) => Widget & {
    checked: boolean;
    label: string;
  };

  export const CheckboxFlags: new (label?: string, obj: object, property: string, flags: number) => Widget & {
    checked: boolean;
    label: string;
    flags: number;
  };

  export const SliderFloat: new (label?: string, value?: number, min?: number, max?: number) => Widget & {
    value: number;
    label: string;
  };

  export const SliderInt: new (label?: string, value?: number, min?: number, max?: number) => Widget & {
    value: number;
    label: string;
  };

  export const Slider: typeof SliderFloat;

  export const InputText: new (label?: string, text?: string) => Widget & {
    text: string;
    label: string;
  };

  export const InputTextWithHint: new (label?: string, text?: string) => Widget & {
    text: string;
    hint: string;
    label: string;
  };

  export const Separator: new () => Widget;
  export const Spacing: new () => Widget;
  export const SameLine: new () => Widget;

  export const TreeNode: new (label?: string, flags?: number) => Widget & {
    label: string;
    flags: number;
  };

  export const CollapsingHeader: new (label?: string, flags?: number) => Widget & {
    label: string;
    flags: number;
  };

  export const TabBar: new (label?: string) => Widget & {
    label: string;
  };

  export const TabItem: new (label?: string) => Widget & {
    label: string;
  };

  export const DemoWindow: new () => Widget;

  export const BulletText: new (text?: string) => Widget & {
    text: string;
  };

  export const TextWrapped: new (text?: string) => Widget & {
    text: string;
  };

  export const TextDisabled: new (text?: string) => Widget & {
    text: string;
  };

  export const LabelText: new (label?: string, text?: string) => Widget & {
    label: string;
    text: string;
  };

  export const SeparatorText: new (text?: string) => Widget & {
    text: string;
  };

  export const Bullet: new () => Widget;
  export const NewLine: new () => Widget;

  export const SmallButton: new (label?: string) => Widget & {
    readonly clicked: boolean;
    label: string;
  };

  export const ArrowButton: new (label?: string, dir?: number) => Widget & {
    readonly clicked: boolean;
    label: string;
  };

  export const RadioButton: new (label?: string, obj: object, property: string, value: any) => Widget & {
    active: boolean;
    label: string;
  };

  export const DragFloat: new (label?: string, value?: number, speed?: number, min?: number, max?: number) => Widget & {
    value: number;
    label: string;
  };

  export const DragInt: new (label?: string, value?: number, speed?: number, min?: number, max?: number) => Widget & {
    value: number;
    label: string;
  };

  export const InputFloat: new (label?: string, value?: number) => Widget & {
    value: number;
    label: string;
  };

  export const InputInt: new (label?: string, value?: number) => Widget & {
    value: number;
    label: string;
  };

  export const InputTextMultiline: new (label?: string, text?: string) => Widget & {
    text: string;
    label: string;
  };

  export const ColorEdit3: new (label?: string, r?: number, g?: number, b?: number) => Widget & {
    label: string;
  };

  export const ColorEdit4: new (label?: string, r?: number, g?: number, b?: number, a?: number) => Widget & {
    label: string;
  };

  export const ColorPicker3: new (label?: string, r?: number, g?: number, b?: number) => Widget & {
    label: string;
  };

  export const ColorPicker4: new (label?: string, r?: number, g?: number, b?: number, a?: number, refR?: number, refG?: number, refB?: number, refA?: number) => Widget & {
    label: string;
  };

  export const ColorButton: new (label?: string, r?: number, g?: number, b?: number, a?: number, width?: number, height?: number) => Widget & {
    label: string;
  };

  export const Combo: new (label?: string, items?: string[]) => Widget & {
    label: string;
  };

  export const ListBox: new (label?: string, items?: string[]) => Widget & {
    label: string;
  };

  export const ProgressBar: new (fraction?: number, sizeX?: number, sizeY?: number, overlay?: string) => Widget & {
    fraction: number;
  };

  export const Selectable: new (label?: string, selected?: boolean) => Widget & {
    label: string;
    selected: boolean;
  };

  export const Child: new (label?: string, sizeX?: number, sizeY?: number, flags?: number) => Widget & {
    label: string;
  };

  export const Group: new () => Widget;
  export const Disabled: new (disabled?: boolean) => Widget;

  export const MainMenuBar: new () => Widget;
  export const MenuBar: new () => Widget;
  export const Menu: new (label?: string) => Widget & {
    label: string;
  };

  export const MenuItem: new (label?: string, shortcut?: string, selected?: boolean, enabled?: boolean) => Widget & {
    label: string;
  };

  export const Popup: new (label?: string) => Widget & {
    label: string;
  };

  export const Modal: new (label?: string, flags?: number) => Widget & {
    label: string;
    open: boolean;
  };

  export const Tooltip: new () => Widget;

  export const Table: new (label?: string, columns?: number, flags?: number) => Widget & {
    label: string;
  };

  export const TableRow: new () => Widget;

  export const PlotLines: new (label?: string, values?: number[]) => Widget & {
    label: string;
  };

  export const PlotHistogram: new (label?: string, values?: number[]) => Widget & {
    label: string;
  };

  export const Indent: new (width?: number) => Widget;
  export const Unindent: new (width?: number) => Widget;
  export const Dummy: new (width?: number, height?: number) => Widget;

  export const Stack: new () => Widget & {
    id: number;
    readonly clipRect: { min: { x: number, y: number }, max: { x: number, y: number }, intersectWithCurrentClipRect: boolean };
    setClipRect(min_x: number, min_y: number, max_x: number, max_y: number, intersectWithCurrentClipRect: boolean): void;
    readonly colors: Array<{ idx: number, r: number, g: number, b: number, a: number }>;
    setColor(idx: number, r: number, g: number, b: number, a: number);
    readonly vars: Array<{ idx: number, x: number, y: number }>;
    setVar(idx: number, x: number, y: number);
    tabStop: boolean;
    buttonRepeat: boolean;
    itemWidth: number;
    textWrap: number;
  }
}

declare module 'nyx:gui' {
  export * from 'gui';
}
