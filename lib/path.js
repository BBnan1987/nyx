'use strict';

// Platform-specific path separator
const isWindows = true; // TODO: detect from C++
const sep = isWindows ? '\\' : '/';
const delimiter = isWindows ? ';' : ':';

function normalizeArray(parts, allowAboveRoot) {
  const result = [];
  for (let i = 0; i < parts.length; i++) {
    const part = parts[i];
    if (!part || part === '.') {
      continue;
    }
    if (part === '..') {
      if (result.length > 0 && result[result.length - 1] !== '..') {
        result.pop();
      } else if (allowAboveRoot) {
        result.push('..');
      }
    } else {
      result.push(part);
    }
  }
  return result;
}

function splitPath(path) {
  const driveMatch = /^([a-zA-Z]:)/.exec(path);
  let device = '';
  if (driveMatch) {
    device = driveMatch[1];
    path = path.slice(2);
  }

  // normalize separators
  path = path.replace(/\\/g, '/');

  const isAbsolutePath = path.charAt(0) === '/';
  const trailingSlash = path.slice(-1) === '/';

  const parts = path.split('/').filter(p => p.length > 0);

  return { device, isAbsolute: isAbsolutePath, parts, trailingSlash };
}

function normalize(path) {
  if (typeof path !== 'string') {
    throw new TypeError('Path must be a string');
  }

  if (path.length === 0) {
    return '.';
  }

  const { device, isAbsolute: isAbs, parts, trailingSlash } = splitPath(path);
  const normalized = normalizeArray(parts, !isAbs);

  let result = normalized.join(sep);
  if (!result && !isAbs) {
    result = '.';
  }
  if (result && trailingSlash) {
    result += sep;
  }

  if (isAbs) {
    result = sep + result;
  }

  return device + result;
}

function join(...paths) {
  if (paths.length === 0) {
    return '.';
  }

  let joined = '';
  for (let i = 0; i < paths.length; i++) {
    const arg = paths[i];
    if (typeof arg !== 'string') {
      throw new TypeError('Arguments must be strings');
    }
    if (arg.length > 0) {
      if (joined.length === 0) {
        joined = arg;
      } else {
        joined += sep + arg;
      }
    }
  }

  if (joined.length === 0) {
    return '.';
  }

  return normalize(joined);
}

function isAbsolute(path) {
  if (typeof path !== 'string') {
    throw new TypeError('Path must be a string');
  }

  if (path.length === 0) {
    return false;
  }

  // check for Windows drive letter
  if (/^[a-zA-Z]:/.test(path)) {
    return path.charAt(2) === '/' || path.charAt(2) === '\\';
  }

  // unix absolute path
  return path.charAt(0) === '/' || path.charAt(0) === '\\';
}

function resolve(...paths) {
  let resolvedPath = '';
  let resolvedAbsolute = false;

  for (let i = paths.length - 1; i >= -1 && !resolvedAbsolute; i--) {
    let path;
    if (i >= 0) {
      path = paths[i];
    } else {
      // Use current working directory
      // TODO: get from C++ binding
      path = '.';
    }

    if (typeof path !== 'string') {
      throw new TypeError('Arguments must be strings');
    }

    if (path.length === 0) {
      continue;
    }

    resolvedPath = path + sep + resolvedPath;
    resolvedAbsolute = isAbsolute(path);
  }

  // normalize
  resolvedPath = normalizeArray(
    resolvedPath.split(/[/\\]/).filter(p => p.length > 0),
    !resolvedAbsolute
  ).join(sep);

  // handle drive letter on Windows
  const driveMatch = /^([a-zA-Z]:)/.exec(paths[0] || '');
  const device = driveMatch ? driveMatch[1] : '';

  return (resolvedAbsolute ? device + sep : '') + resolvedPath || '.';
}

function dirname(path) {
  if (typeof path !== 'string') {
    throw new TypeError('Path must be a string');
  }

  if (path.length === 0) {
    return '.';
  }

  // handle Windows drive letter
  const driveMatch = /^([a-zA-Z]:)/.exec(path);
  let device = '';
  if (driveMatch) {
    device = driveMatch[1];
    path = path.slice(2);
  }

  // normalize separators
  path = path.replace(/\\/g, '/');

  // remove trailing slashes
  let end = path.length - 1;
  while (end > 0 && path.charAt(end) === '/') {
    end--;
  }

  // find last separator
  const lastSlash = path.lastIndexOf('/', end);

  if (lastSlash === -1) {
    return device || '.';
  }

  if (lastSlash === 0) {
    return device + sep;
  }

  return device + path.slice(0, lastSlash).replace(/\//g, sep);
}

function basename(path, ext) {
  if (typeof path !== 'string') {
    throw new TypeError('Path must be a string');
  }

  // normalize separators
  path = path.replace(/\\/g, '/');

  // remove trailing slashes
  let end = path.length - 1;
  while (end > 0 && path.charAt(end) === '/') {
    end--;
  }

  // find last separator
  let start = path.lastIndexOf('/', end);
  start = start === -1 ? 0 : start + 1;

  let base = path.slice(start, end + 1);

  // remove extension if provided
  if (ext && base.endsWith(ext)) {
    base = base.slice(0, base.length - ext.length);
  }

  return base;
}

function extname(path) {
  if (typeof path !== 'string') {
    throw new TypeError('Path must be a string');
  }

  // normalize separators
  path = path.replace(/\\/g, '/');

  // get basename
  const base = basename(path);

  // find last dot
  const dotIndex = base.lastIndexOf('.');

  if (dotIndex <= 0) {
    return '';
  }

  return base.slice(dotIndex);
}

function relative(from, to) {
  if (typeof from !== 'string' || typeof to !== 'string') {
    throw new TypeError('Arguments must be strings');
  }

  from = resolve(from);
  to = resolve(to);

  if (from === to) {
    return '';
  }

  // normalize
  from = from.toLowerCase().replace(/\\/g, '/');
  to = to.toLowerCase().replace(/\\/g, '/');

  const fromParts = from.split('/').filter(p => p.length > 0);
  const toParts = to.split('/').filter(p => p.length > 0);

  // find common prefix
  let commonLength = 0;
  const minLength = Math.min(fromParts.length, toParts.length);
  for (let i = 0; i < minLength; i++) {
    if (fromParts[i] === toParts[i]) {
      commonLength++;
    } else {
      break;
    }
  }

  // build relative path
  const upCount = fromParts.length - commonLength;
  const result = [];

  for (let i = 0; i < upCount; i++) {
    result.push('..');
  }

  for (let i = commonLength; i < toParts.length; i++) {
    result.push(toParts[i]);
  }

  return result.join(sep);
}

function parse(path) {
  if (typeof path !== 'string') {
    throw new TypeError('Path must be a string');
  }

  const result = {
    root: '',
    dir: '',
    base: '',
    ext: '',
    name: ''
  };

  if (path.length === 0) {
    return result;
  }

  // handle Windows drive letter
  const driveMatch = /^([a-zA-Z]:)/.exec(path);
  if (driveMatch) {
    result.root = driveMatch[1] + sep;
  } else if (path.charAt(0) === '/' || path.charAt(0) === '\\') {
    result.root = sep;
  }

  result.dir = dirname(path);
  result.base = basename(path);
  result.ext = extname(path);
  result.name = result.base.slice(0, result.base.length - result.ext.length);

  return result;
}

function format(pathObject) {
  if (typeof pathObject !== 'object' || pathObject === null) {
    throw new TypeError('Argument must be an object');
  }

  const dir = pathObject.dir || pathObject.root || '';
  const base = pathObject.base || (pathObject.name || '') + (pathObject.ext || '');

  if (!dir) {
    return base;
  }

  if (dir === pathObject.root) {
    return dir + base;
  }

  return dir + sep + base;
}

module.exports = {
  sep,
  delimiter,
  normalize,
  join,
  resolve,
  isAbsolute,
  dirname,
  basename,
  extname,
  relative,
  parse,
  format
};
