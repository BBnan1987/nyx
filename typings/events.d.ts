declare namespace nyx {
  namespace events {
    /** Register an event handler. */
    function on(event: string, handler: (...args: any[]) => void): void;

    /** Remove an event handler. */
    function off(event: string, handler?: (...args: any[]) => void): void;

    /** Emit an event. */
    function emit(event: string, ...args: any[]): void;
  }
}
