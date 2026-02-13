'use strict';

class EventEmitter {
  constructor() {
    this._listeners = {};
  }

  on(event, callback) {
    if (!this._listeners[event]) this._listeners[event] = [];
    this._listeners[event].push(callback);
    return this;
  }

  off(event, callback) {
    const list = this._listeners[event];
    if (!list) return this;
    if (callback) {
      const idx = list.indexOf(callback);
      if (idx !== -1) list.splice(idx, 1);
    } else {
      delete this._listeners[event];
    }
    return this;
  }

  emit(event, ...args) {
    const list = this._listeners[event];
    if (list) for (const fn of list) fn(...args);
    return this;
  }
}

module.exports = { EventEmitter };
