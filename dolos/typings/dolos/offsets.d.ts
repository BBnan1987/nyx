declare module 'dolos/offsets' {
  /**
   * Dolos offsets registry
   * Offsets are registered dynamically at runtime from C++
   * Access offset values as properties, e.g., offsets.someOffset
   */
  const offsets: {
    readonly [offsetName: string]: bigint;
  };

  export = offsets;
}

declare module 'nyx:dolos/offsets' {
  export * from 'dolos/offsets';
}
