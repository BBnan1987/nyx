declare namespace nyx {
  namespace fs {
    /** Read a file as UTF-8 string. Throws on error. */
    function readFile(path: string): string;

    /** Write a UTF-8 string to file. */
    function writeFile(path: string, data: string): void;

    /** Check if path exists. */
    function exists(path: string): boolean;
  }
}
