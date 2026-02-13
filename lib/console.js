'use strict';

const { write, getStackTrace } = internalBinding('console');

const kStdout = 1;
const kStderr = 2;

const timers = new Map();
const counters = new Map();
let groupDepth = 0;

function indent() {
  if (groupDepth === 0) return '';
  return '  '.repeat(groupDepth);
}

function formatValue(v) {
  if (v === null) return 'null';
  if (v === undefined) return 'undefined';
  if (typeof v === 'string') return v;
  if (typeof v === 'symbol') return v.toString();
  if (typeof v === 'function') return `[Function: ${v.name || 'anonymous'}]`;
  if (typeof v === 'object') {
    if (Array.isArray(v)) {
      try {
        return '[ ' + v.map(formatValue).join(', ') + ' ]';
      } catch {
        return String(v);
      }
    }
    if (v instanceof Error) {
      return v.stack || v.message || String(v);
    }
    try {
      return JSON.stringify(v, null, 2);
    } catch {
      return String(v);
    }
  }
  return String(v);
}

function formatArgs(args) {
  if (args.length === 0) return '';

  const first = args[0];

  // simple printf-style substitution when first arg is a string
  if (typeof first === 'string' && args.length > 1) {
    let i = 1;
    let result = first.replace(/%[sdifjoO%]/g, (match) => {
      if (match === '%%') return '%';
      if (i >= args.length) return match;
      const arg = args[i++];
      switch (match) {
        case '%s': return String(arg);
        case '%d':
        case '%i': return parseInt(arg, 10).toString();
        case '%f': return parseFloat(arg).toString();
        case '%j':
          try { return JSON.stringify(arg); }
          catch { return '[Circular]'; }
        case '%o':
        case '%O': return formatValue(arg);
        default: return match;
      }
    });
    // append remaining args
    for (; i < args.length; i++) {
      result += ' ' + formatValue(args[i]);
    }
    return result;
  }

  // fallback to joining all args with spaces
  return args.map(formatValue).join(' ');
}

const console = {
  log(...args) {
    write(kStdout, indent() + formatArgs(args));
  },

  info(...args) {
    write(kStdout, indent() + formatArgs(args));
  },

  debug(...args) {
    write(kStdout, indent() + formatArgs(args));
  },

  warn(...args) {
    write(kStderr, indent() + formatArgs(args));
  },

  error(...args) {
    write(kStderr, indent() + formatArgs(args));
  },

  assert(condition, ...args) {
    if (!condition) {
      const message = args.length > 0
        ? 'Assertion failed: ' + formatArgs(args)
        : 'Assertion failed';
      write(kStderr, indent() + message);
    }
  },

  trace(...args) {
    const label = args.length > 0 ? 'Trace: ' + formatArgs(args) : 'Trace';
    const stack = getStackTrace();
    write(kStderr, indent() + label + '\n' + stack);
  },

  time(label = 'default') {
    if (timers.has(label)) {
      write(kStderr, `Timer '${label}' already exists`);
      return;
    }
    timers.set(label, Date.now());
  },

  timeLog(label = 'default', ...args) {
    const start = timers.get(label);
    if (start === undefined) {
      write(kStderr, `Timer '${label}' does not exist`);
      return;
    }
    const elapsed = Date.now() - start;
    const extra = args.length > 0 ? ' ' + formatArgs(args) : '';
    write(kStdout, indent() + `${label}: ${elapsed.toFixed(3)}ms${extra}`);
  },

  timeEnd(label = 'default') {
    const start = timers.get(label);
    if (start === undefined) {
      write(kStderr, `Timer '${label}' does not exist`);
      return;
    }
    const elapsed = Date.now() - start;
    timers.delete(label);
    write(kStdout, indent() + `${label}: ${elapsed.toFixed(3)}ms`);
  },

  count(label = 'default') {
    const count = (counters.get(label) || 0) + 1;
    counters.set(label, count);
    write(kStdout, indent() + `${label}: ${count}`);
  },

  countReset(label = 'default') {
    if (!counters.has(label)) {
      write(kStderr, `Count for '${label}' does not exist`);
      return;
    }
    counters.delete(label);
  },

  group(...args) {
    if (args.length > 0) {
      write(kStdout, indent() + formatArgs(args));
    }
    groupDepth++;
  },

  groupEnd() {
    if (groupDepth > 0) groupDepth--;
  },

  clear() {
    write(kStdout, '\x1b[2J\x1b[H');
    groupDepth = 0;
  },

  dir(obj) {
    write(kStdout, indent() + formatValue(obj));
  },

  table(data) {
    // simplified table, just log the formatted object
    write(kStdout, indent() + formatValue(data));
  },
};

globalThis.console = console;

module.exports = console;
