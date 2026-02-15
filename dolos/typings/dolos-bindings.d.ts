// Ambient declarations for dolos internalBinding() function
// Provides type-safe access to dolos C++ bindings

/**
 * Access dolos internal C++ bindings
 * Dynamically registered offsets accessible as properties
 */
declare function internalBinding(module: 'dolos'): {
  readonly [offsetName: string]: bigint;
};
