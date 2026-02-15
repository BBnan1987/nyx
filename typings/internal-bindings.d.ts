// Ambient declarations for internalBinding() function
// Provides type-safe access to nyx C++ bindings

/**
 * Access nyx memory internal C++ bindings
 */
declare function internalBinding(module: 'memory'): {
  readMemory(address: bigint, size: number): Uint8Array;
  readMemoryFast(address: bigint, size: number): Uint8Array;
  readMemoryInto(address: bigint, buffer: Uint8Array): void;
  writeMemory(address: bigint, data: Uint8Array): void;
  allocateTestMemory(size: number): bigint;
  freeTestMemory(address: bigint): void;
  freeAllTestMemory(): void;
  highResolutionTime(): bigint;
  acquireGameLock(timeout?: number): boolean;
  releaseGameLock(): void;
  isGameLockHeld(): boolean;
  isGameLockOpen(): boolean;
};

declare function internalBinding(module: 'console'): {
  write(fd: number, message: string): void;
  getStackTrace(): string;
};

declare function internalBinding(module: 'fs'): {
  // Sync methods
  readFileSync(path: string, encoding?: string): Uint8Array | string;
  writeFileSync(path: string, data: string | Uint8Array, encoding?: string): void;
  existsSync(path: string): boolean;
  statSync(path: string): {
    size: number;
    mode: number;
    mtime: number;
    atime: number;
    ctime: number;
    isFile: boolean;
    isDirectory: boolean;
  };
  readdirSync(path: string): string[];
  mkdirSync(path: string, options?: { mode?: number; recursive?: boolean }): void;
  unlinkSync(path: string): void;
  rmdirSync(path: string): void;
  renameSync(oldPath: string, newPath: string): void;
  realpathSync(path: string): string;

  // Async methods
  readFile(path: string, encoding?: string): Promise<Uint8Array | string>;
  writeFile(path: string, data: string | Uint8Array): Promise<void>;
  stat(path: string): Promise<{
    size: number;
    mode: number;
    mtime: number;
    atime: number;
    ctime: number;
    isFile: boolean;
    isDirectory: boolean;
  }>;
  readdir(path: string): Promise<string[]>;
  mkdir(path: string, options?: { mode?: number }): Promise<void>;
  unlink(path: string): Promise<void>;
  rmdir(path: string): Promise<void>;
  rename(oldPath: string, newPath: string): Promise<void>;
};

declare function internalBinding(module: 'process'): {
  cwd(): string;
  chdir(path: string): void;
  scriptsRoot(): string | undefined;
  setScriptsRoot(path: string): void;
};

declare function internalBinding(module: 'timers'): {
  setTimeout(callback: (...args: any[]) => void, ms?: number): number;
  setInterval(callback: (...args: any[]) => void, ms?: number): number;
  clearTimeout(id: number): void;
  clearInterval(id: number): void;
  setImmediate(callback: (...args: any[]) => void): number;
  clearImmediate(id: number): void;
};

declare function internalBinding(module: 'events'): any;

declare function internalBinding(module: 'gui'): {
  Panel: any;
  Text: any;
  TextColored: any;
  Button: any;
  Checkbox: any;
  SliderFloat: any;
  SliderInt: any;
  InputText: any;
  Separator: any;
  Spacing: any;
  SameLine: any;
  TreeNode: any;
  CollapsingHeader: any;
  TabBar: any;
  TabItem: any;
  DemoWindow: any;
  BulletText: any;
  TextWrapped: any;
  TextDisabled: any;
  LabelText: any;
  SeparatorText: any;
  Bullet: any;
  NewLine: any;
  SmallButton: any;
  ArrowButton: any;
  RadioButton: any;
  DragFloat: any;
  DragInt: any;
  InputFloat: any;
  InputInt: any;
  InputTextMultiline: any;
  ColorEdit3: any;
  ColorEdit4: any;
  Combo: any;
  ListBox: any;
  ProgressBar: any;
  Selectable: any;
  Child: any;
  Group: any;
  Disabled: any;
  MenuBar: any;
  Menu: any;
  MenuItem: any;
  Popup: any;
  Modal: any;
  Tooltip: any;
  Table: any;
  TableRow: any;
  PlotLines: any;
  PlotHistogram: any;
  Indent: any;
  Unindent: any;
  Dummy: any;
};

declare function internalBinding(module: 'module_wrap'): any;
declare function internalBinding(module: 'builtins'): any;
