'use strict';

const binding = internalBinding('timers');

const {
  setTimeout: _setTimeout,
  setInterval: _setInterval,
  clearTimeout: _clearTimeout,
  clearInterval: _clearInterval,
  setImmediate: _setImmediate,
  clearImmediate: _clearImmediate,
} = binding;

// expose on globalThis so they're available everywhere without import
// file is required in internal/bootstrap/nyx
globalThis.setTimeout = _setTimeout;
globalThis.setInterval = _setInterval;
globalThis.clearTimeout = _clearTimeout;
globalThis.clearInterval = _clearInterval;
globalThis.setImmediate = _setImmediate;
globalThis.clearImmediate = _clearImmediate;

// also export for explicit imports
module.exports = {
  setTimeout: _setTimeout,
  setInterval: _setInterval,
  clearTimeout: _clearTimeout,
  clearInterval: _clearInterval,
  setImmediate: _setImmediate,
  clearImmediate: _clearImmediate,
};
