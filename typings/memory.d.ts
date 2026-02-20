declare module 'memory' {
  // DataTypes enum-like object
  export const DataTypes: {
    readonly Int8: 'int8';
    readonly Uint8: 'uint8';
    readonly Int16: 'int16';
    readonly Uint16: 'uint16';
    readonly Int32: 'int32';
    readonly Uint32: 'uint32';
    readonly Int64: 'int64';
    readonly Uint64: 'uint64';
    readonly Float32: 'float32';
    readonly Float64: 'float64';
    readonly Pointer: 'pointer';
    readonly Bool: 'bool';
    readonly Padding: 'padding';
  };

  type DataType = typeof DataTypes[keyof typeof DataTypes];

  // Field definition for MemoryModel
  export interface FieldDefinition {
    name?: string;
    type?: DataType;
    model?: MemoryModel;
    length?: number;
  }

  // Cursor interface - represents a view into memory
  export interface Cursor<T = any> {
    readonly $index: number;
    readonly $count: number;
    readonly $valid: boolean;
    readonly $address: bigint;

    $next(): boolean;
    $reset(): void;
    $moveTo(index: number): boolean;
    $flushCurrent(): void;
    $flushAll(): void;
    $toObject(target?: T): T & { _address: bigint };
    $load?(address: bigint | number, count: number): void;
  }

  // MemoryModel options
  export interface MemoryModelOptions {
    packed?: boolean;
    maxAlign?: number;
    structAlign?: number;
  }

  // MemoryModel class
  export class MemoryModel {
    readonly name: string;
    readonly size: number;
    readonly parent: MemoryModel | null;
    readonly fieldInfos: ReadonlyArray<any>;
    readonly CursorClass: new (...args: any[]) => Cursor;

    constructor(
      name: string,
      fields: FieldDefinition[],
      parent?: MemoryModel | null,
      options?: MemoryModelOptions
    );

    /**
     * Read a single object at the given address
     */
    read<T = any>(address: bigint | number): Cursor<T>;

    /**
     * Create a one-time cursor for count objects
     */
    cursor<T = any>(address: bigint | number, count: number): Cursor<T>;

    /**
     * Create a reusable cursor with preallocated buffer
     */
    createCursor<T = any>(maxCount: number): Cursor<T> & {
      $load(address: bigint | number, count: number): void;
    };

    /**
     * Read an array of objects (returns array of cursor instances)
     */
    readArray<T = any>(address: bigint | number, count: number): Cursor<T>[];

    /**
     * Initialize snapshot object with zero values
     */
    initSnapshot(target: any): void;

    /**
     * Copy cursor data into a snapshot object
     */
    snapshot<T = any>(source: Cursor, target?: T): T;

    /**
     * Extend this model with additional fields
     */
    extend(name: string, fields: FieldDefinition[]): MemoryModel;

    /**
     * Define a new MemoryModel
     */
    static define(
      name: string,
      fields: FieldDefinition[],
      parent?: MemoryModel | null,
      options?: MemoryModelOptions
    ): MemoryModel;
  }

  // Snapshot registry for class associations
  export const snapshotRegistry: {
    /**
     * Register a class constructor for a model name
     * @param cls Constructor function
     * @param modelName Name of the MemoryModel
     */
    add(cls: new (...args: any[]) => any, modelName: string): void;
  };

  // Raw memory functions
  export function readMemory(address: bigint | number, size: number): Uint8Array;
  export function readMemoryFast(address: bigint | number, size: number): Uint8Array;
  export function readMemoryInto(address: bigint | number, buffer: Uint8Array): void;
  export function readMemoryIntoIfChanged(address: bigint | number, buffer: Uint8Array): boolean;
  export function writeMemory(address: bigint | number, data: Uint8Array): void;

  // Test memory allocation functions
  export function allocateTestMemory(size: number): bigint;
  export function freeTestMemory(address: bigint): void;
  export function freeAllTestMemory(): void;

  // High resolution time
  export function highResolutionTime(): bigint;

  // Game lock functions
  /**
   * Acquire the game lock (waits for game thread to open its window)
   * @param timeout Timeout in milliseconds (default: 100)
   * @returns True if lock was acquired
   */
  export function acquireGameLock(timeout?: number): boolean;

  /**
   * Release the game lock
   */
  export function releaseGameLock(): void;

  /**
   * Check if the game lock is currently held
   */
  export function isGameLockHeld(): boolean;

  /**
   * Check if the game lock window is currently open
   */
  export function isGameLockOpen(): boolean;

  /**
   * Execute a function while holding the game lock.
   * Ensures the lock is released even if an error occurs.
   * @param fn Function to execute while holding the lock
   * @param timeout Timeout in milliseconds to acquire lock (default: 100)
   * @returns Return value of fn
   * @throws Error if lock cannot be acquired, or if fn throws
   */
  export function withGameLock<T>(fn: () => T, timeout?: number): T;

  /**
   * Try to execute a function while holding the game lock.
   * Returns undefined if lock cannot be acquired (does not throw).
   * @param fn Function to execute while holding the lock
   * @param timeout Timeout in milliseconds to acquire lock (default: 100)
   * @returns Return value of fn, or undefined if lock not acquired
   */
  export function tryWithGameLock<T>(fn: () => T, timeout?: number): T | undefined;
}

// Support nyx: prefix
declare module 'nyx:memory' {
  export * from 'memory';
}
