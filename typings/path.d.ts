declare module 'path' {
  /**
   * Platform-specific path segment separator
   */
  export const sep: string;

  /**
   * Platform-specific path delimiter
   */
  export const delimiter: string;

  /**
   * Normalize a path
   * @param path Path to normalize
   */
  export function normalize(path: string): string;

  /**
   * Join path segments
   * @param paths Path segments to join
   */
  export function join(...paths: string[]): string;

  /**
   * Resolve path segments to an absolute path
   * @param paths Path segments to resolve
   */
  export function resolve(...paths: string[]): string;

  /**
   * Check if a path is absolute
   * @param path Path to check
   */
  export function isAbsolute(path: string): boolean;

  /**
   * Get the directory name of a path
   * @param path Path to process
   */
  export function dirname(path: string): string;

  /**
   * Get the last portion of a path
   * @param path Path to process
   * @param ext Optional file extension to remove
   */
  export function basename(path: string, ext?: string): string;

  /**
   * Get the extension of a path
   * @param path Path to process
   */
  export function extname(path: string): string;

  /**
   * Get the relative path from one path to another
   * @param from Source path
   * @param to Destination path
   */
  export function relative(from: string, to: string): string;

  /**
   * Parse a path into an object
   * @param path Path to parse
   */
  export function parse(path: string): {
    root: string;
    dir: string;
    base: string;
    ext: string;
    name: string;
  };

  /**
   * Format a path object into a path string
   * @param pathObject Path object to format
   */
  export function format(pathObject: {
    root?: string;
    dir?: string;
    base?: string;
    ext?: string;
    name?: string;
  }): string;
}

declare module 'nyx:path' {
  export * from 'path';
}
