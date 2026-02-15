declare module 'console' {
  interface Console {
    /**
     * Prints to stdout with newline
     */
    log(...args: any[]): void;

    /**
     * Prints to stdout with newline (alias for log)
     */
    info(...args: any[]): void;

    /**
     * Prints to stdout with newline (alias for log)
     */
    debug(...args: any[]): void;

    /**
     * Prints to stderr with newline
     */
    warn(...args: any[]): void;

    /**
     * Prints to stderr with newline
     */
    error(...args: any[]): void;

    /**
     * Prints assertion failure to stderr if condition is false
     */
    assert(condition: boolean, ...args: any[]): void;

    /**
     * Prints stack trace to stderr
     */
    trace(...args: any[]): void;

    /**
     * Start a timer
     * @param label Timer label (default: 'default')
     */
    time(label?: string): void;

    /**
     * Log the current value of a timer
     * @param label Timer label (default: 'default')
     */
    timeLog(label?: string, ...args: any[]): void;

    /**
     * Stop a timer and log the result
     * @param label Timer label (default: 'default')
     */
    timeEnd(label?: string): void;

    /**
     * Increment and log a counter
     * @param label Counter label (default: 'default')
     */
    count(label?: string): void;

    /**
     * Reset a counter
     * @param label Counter label (default: 'default')
     */
    countReset(label?: string): void;

    /**
     * Start a group with optional label
     */
    group(...args: any[]): void;

    /**
     * End the current group
     */
    groupEnd(): void;

    /**
     * Clear the console
     */
    clear(): void;

    /**
     * Print an object representation
     */
    dir(obj: any): void;

    /**
     * Display data as a table
     */
    table(data: any): void;
  }

  const console: Console;
  export = console;
}

declare module 'nyx:console' {
  export * from 'console';
}
