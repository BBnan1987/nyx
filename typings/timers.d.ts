declare module 'timers' {
  /**
   * Execute a function after a delay
   * @param callback Function to execute
   * @param ms Delay in milliseconds (default: 0)
   * @param args Arguments to pass to callback
   * @returns Timer ID
   */
  export function setTimeout(
    callback: (...args: any[]) => void,
    ms?: number,
    ...args: any[]
  ): number;

  /**
   * Clear a timeout
   * @param id Timer ID returned by setTimeout
   */
  export function clearTimeout(id: number): void;

  /**
   * Execute a function repeatedly at an interval
   * @param callback Function to execute
   * @param ms Interval in milliseconds (default: 0)
   * @param args Arguments to pass to callback
   * @returns Timer ID
   */
  export function setInterval(
    callback: (...args: any[]) => void,
    ms?: number,
    ...args: any[]
  ): number;

  /**
   * Clear an interval
   * @param id Timer ID returned by setInterval
   */
  export function clearInterval(id: number): void;

  /**
   * Execute a function immediately (queued, not synchronous)
   * @param callback Function to execute
   * @param args Arguments to pass to callback
   * @returns Timer ID
   */
  export function setImmediate(
    callback: (...args: any[]) => void,
    ...args: any[]
  ): number;

  /**
   * Clear an immediate
   * @param id Timer ID returned by setImmediate
   */
  export function clearImmediate(id: number): void;
}

declare module 'nyx:timers' {
  export * from 'timers';
}
