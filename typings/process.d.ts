declare module 'process' {
  /**
   * Get the current working directory
   */
  export function cwd(): string;

  /**
   * Change the current working directory
   * @param path New working directory path
   */
  export function chdir(path: string): void;

  /**
   * Get the current scripts directory
   */
  export function scriptsRoot(): string;

  /**
   * Change the current scripts root directory
   * @param path
   */
  export function setScriptsRoot(path: string): void;
}

declare module 'nyx:process' {
  export * from 'process';
}
