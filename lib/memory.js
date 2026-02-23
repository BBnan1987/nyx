'use strict';

const binding = internalBinding('memory');

// fixme: use this once we have 'os'
//const {
//  endianess
//} = require('os');
//const IS_LITTLE_ENDIAN = os.endianness() === 'LE';

const IS_LITTLE_ENDIAN = true;

function _readString(buffer, offset, byteSize, encoding) {
  if (encoding === 'wide') {
    let end = 0;
    while (end + 1 < byteSize) {
      if (buffer[offset + end] === 0 && buffer[offset + end + 1] === 0) break;
      end += 2;
    }
    let s = '';
    for (let i = 0; i < end; i += 2)
      s += String.fromCharCode(buffer[offset + i] | (buffer[offset + i + 1] << 8));
    return s;
  } else {
    let end = 0;
    while (end < byteSize && buffer[offset + end] !== 0) end++;
    if (encoding === 'ansi') {
      let s = '';
      for (let i = 0; i < end; i++) s += String.fromCharCode(buffer[offset + i]);
      return s;
    } else {
      if (typeof TextDecoder !== 'undefined')
        return new TextDecoder('utf-8').decode(buffer.subarray(offset, offset + end));
      let s = '';
      for (let i = 0; i < end; i++) s += String.fromCharCode(buffer[offset + i]);
      return s;
    }
  }
}

function _writeString(str, buffer, offset, byteSize, encoding) {
  buffer.fill(0, offset, offset + byteSize);
  if (!str) return;
  if (encoding === 'wide') {
    const maxChars = (byteSize >> 1) - 1; // reserve null terminator word
    const len = Math.min(str.length, maxChars);
    for (let i = 0; i < len; i++) {
      const code = str.charCodeAt(i);
      buffer[offset + i * 2]     = code & 0xFF;
      buffer[offset + i * 2 + 1] = (code >> 8) & 0xFF;
    }
  } else if (encoding === 'ansi') {
    const maxBytes = byteSize - 1;
    const len = Math.min(str.length, maxBytes);
    for (let i = 0; i < len; i++) buffer[offset + i] = str.charCodeAt(i) & 0xFF;
  } else {
    if (typeof TextEncoder !== 'undefined') {
      const encoded = new TextEncoder().encode(str);
      const toCopy = Math.min(encoded.length, byteSize - 1);
      buffer.set(encoded.subarray(0, toCopy), offset);
    } else {
      const maxBytes = byteSize - 1;
      const len = Math.min(str.length, maxBytes);
      for (let i = 0; i < len; i++) buffer[offset + i] = str.charCodeAt(i) & 0xFF;
    }
  }
}

const DataTypes = {
  Int8: 'int8',
  Uint8: 'uint8',
  Int16: 'int16',
  Uint16: 'uint16',
  Int32: 'int32',
  Uint32: 'uint32',
  Int64: 'int64',
  Uint64: 'uint64',
  Float32: 'float32',
  Float64: 'float64',
  Pointer: 'pointer',
  Bool: 'bool',
  Padding: 'padding',
  Union: 'union',
  String: 'string',
};

const typeInfo = {
  int8: { size: 1, read: 'getInt8', write: 'setInt8' },
  uint8: { size: 1, read: 'getUint8', write: 'setUint8' },
  int16: { size: 2, read: 'getInt16', write: 'setInt16' },
  uint16: { size: 2, read: 'getUint16', write: 'setUint16' },
  int32: { size: 4, read: 'getInt32', write: 'setInt32' },
  uint32: { size: 4, read: 'getUint32', write: 'setUint32' },
  int64: { size: 8, read: 'getBigInt64', write: 'setBigInt64' },
  uint64: { size: 8, read: 'getBigUint64', write: 'setBigUint64' },
  float32: { size: 4, read: 'getFloat32', write: 'setFloat32' },
  float64: { size: 8, read: 'getFloat64', write: 'setFloat64' },
  pointer: { size: 8, read: 'getBigUint64', write: 'setBigUint64' },
  bool: { size: 1, read: 'getUint8', write: 'setUint8' },
};

// TypedArray shift amounts for computing index from byte offset
const typeShift = {
  int8: 0, uint8: 0, bool: 0,
  int16: 1, uint16: 1,
  int32: 2, uint32: 2, float32: 2,
  float64: 3,
  // BigInt types use DataView (no TypedArray for 64-bit integers)
  int64: null, uint64: null, pointer: null,
};

// TypedArray property names and constructors
const typedArrayInfo = {
  int8: { prop: '_i8', ctor: 'Int8Array' },
  uint8: { prop: '_u8', ctor: 'Uint8Array' },
  bool: { prop: '_u8', ctor: 'Uint8Array' },
  int16: { prop: '_i16', ctor: 'Int16Array' },
  uint16: { prop: '_u16', ctor: 'Uint16Array' },
  int32: { prop: '_i32', ctor: 'Int32Array' },
  uint32: { prop: '_u32', ctor: 'Uint32Array' },
  float32: { prop: '_f32', ctor: 'Float32Array' },
  float64: { prop: '_f64', ctor: 'Float64Array' },
};

// Resolve a model field that may be a thunk (() => model) for self-referential types.
function resolveModel(m) {
  return typeof m === 'function' ? m() : m;
}

class MemoryModel {
  constructor(name, fields, parent, options = {}) {
    this.name = name;
    this.parent = parent || null;
    this.size = 0;
    this._cache = new Map();

    this.packed = options.packed ?? false;
    this.maxAlign = options.maxAlign ?? 8;
    this.structAlign = this.packed ? (options.structAlign ?? 1) : (options.structAlign ?? 8);

    // Inherit parent fields if extending
    const fieldInfos = parent ? parent.fieldInfos.slice() : [];
    let offset = parent ? parent.size : 0;
    for (const f of fields) {
      // Handle padding - skip field but advance offset
      if (f.type === 'padding') {
        if (!f.length) throw new Error('Padding type requires length property');
        offset += f.length;
        continue;
      }

      // Handle C-style anonymous union: { type: 'union', fields: [...] }
      if (f.type === 'union') {
        if (!Array.isArray(f.fields) || f.fields.length === 0)
          throw new Error('Union requires a non-empty fields array');
        let maxMemberSize = 0;
        let maxMemberAlign = 1;
        if (!this.packed) {
          for (const uf of f.fields) {
            if (uf.type === 'padding' || uf.type === 'union') continue;
            if (uf.model && uf.type !== 'pointer') {
              const r = resolveModel(uf.model);
              maxMemberAlign = Math.max(maxMemberAlign, Math.min(r.maxAlign ?? 8, this.maxAlign));
            } else if (uf.type === 'pointer' || uf.type === 'int64' || uf.type === 'uint64' || uf.type === 'float64') {
              maxMemberAlign = Math.max(maxMemberAlign, Math.min(8, this.maxAlign));
            } else {
              const mInfo = typeInfo[uf.type];
              if (mInfo) maxMemberAlign = Math.max(maxMemberAlign, Math.min(mInfo.size, this.maxAlign));
            }
          }
        }
        const unionAlign = this.packed ? 1 : maxMemberAlign;
        if (!this.packed) {
          offset = (offset + unionAlign - 1) & ~(unionAlign - 1);
        }
        const alignedStart = offset;
        for (const uf of f.fields) {
          if (uf.type === 'union') throw new Error('Nested unions are not supported');
          if (uf.type === 'padding') {
            if (!uf.length) throw new Error('Padding type requires length property');
            maxMemberSize = Math.max(maxMemberSize, uf.length);
            continue;
          }
          if (uf.model && uf.type !== 'pointer') {
            const resolved = resolveModel(uf.model);
            fieldInfos.push({ name: uf.name, type: 'model_inline', model: uf.model, offset: alignedStart, size: resolved.size });
            maxMemberSize = Math.max(maxMemberSize, resolved.size);
          } else if (uf.type === 'pointer' && uf.model !== undefined) {
            fieldInfos.push({ name: uf.name, type: 'model_pointer', model: uf.model, offset: alignedStart, size: 8, lazy: true });
            maxMemberSize = Math.max(maxMemberSize, 8);
          } else {
            const mInfo = typeInfo[uf.type];
            if (!mInfo) throw new Error(`Unknown type in union: ${uf.type}`);
            fieldInfos.push({ name: uf.name, type: uf.type, offset: alignedStart, size: mInfo.size });
            maxMemberSize = Math.max(maxMemberSize, mInfo.size);
          }
        }
        offset = alignedStart + maxMemberSize;
        continue;
      }

      // Handle fixed-size arrays: any field with count > 1
      if (f.count !== undefined && f.count > 1) {
        if (!f.name) throw new Error('Array field requires name property');
        const count = f.count;

        // Array of inline model
        if (f.model && f.type !== 'pointer') {
          const resolved = resolveModel(f.model);
          const itemSize = resolved.size;
          const childAlign = this.packed ? 1 : (resolved.maxAlign ?? 8);
          if (!this.packed) offset = (offset + childAlign - 1) & ~(childAlign - 1);
          fieldInfos.push({ name: f.name, type: 'array_model', model: f.model, offset, size: itemSize * count, count, itemSize });
          offset += itemSize * count;
          continue;
        }

        // Array of pointer-to-model
        if (f.type === 'pointer' && f.model !== undefined) {
          const ptrAlign = this.packed ? 1 : 8;
          if (!this.packed) offset = (offset + ptrAlign - 1) & ~(ptrAlign - 1);
          fieldInfos.push({ name: f.name, type: 'array_pointer', model: f.model, offset, size: 8 * count, count, itemSize: 8 });
          offset += 8 * count;
          continue;
        }

        // Array of primitives
        const aInfo = typeInfo[f.type];
        if (!aInfo) throw new Error(`Unknown type in array: ${f.type}`);
        const aAlign = this.packed ? 1 : (aInfo.size > this.maxAlign ? this.maxAlign : aInfo.size);
        if (!this.packed) offset = (offset + aAlign - 1) & ~(aAlign - 1);
        fieldInfos.push({ name: f.name, type: 'array_primitive', itemType: f.type, offset, size: aInfo.size * count, count, itemSize: aInfo.size });
        offset += aInfo.size * count;
        continue;
      }

      // Handle inline model (no type or type is not pointer)
      if (f.model && f.type !== 'pointer') {
        if (!f.name) throw new Error('Model field requires name property');
        const resolvedInline = resolveModel(f.model);
        let childAlign = this.packed ? 1 : resolvedInline.maxAlign ?? 8;
        if (!this.packed) {
          offset = (offset + childAlign - 1) & ~(childAlign - 1);
        }
        fieldInfos.push({
          name: f.name,
          type: 'model_inline',
          model: f.model,
          offset,
          size: resolvedInline.size,
        });
        offset += resolvedInline.size;
        continue;
      }

      // Handle pointer to model
      if (f.type === 'pointer' && f.model) {
        if (!f.name) throw new Error('Model field requires name property');
        const ptrAlign = this.packed ? 1 : 8;
        if (!this.packed) {
          offset = (offset + ptrAlign - 1) & ~(ptrAlign - 1);
        }
        fieldInfos.push({
          name: f.name,
          type: 'model_pointer',
          model: f.model,
          offset,
          size: 8,
          lazy: f.lazy ?? false,
        });
        offset += 8;
        continue;
      }

      if (f.type === 'string') {
        if (!f.name) throw new Error('String field requires name property');
        const encoding = f.encoding ?? 'utf8';
        const charSize = encoding === 'wide' ? 2 : 1;
        const length = f.length ?? 128;
        const byteSize = length * charSize;
        const strAlign = this.packed ? 1 : Math.min(charSize, this.maxAlign);
        if (!this.packed) offset = (offset + strAlign - 1) & ~(strAlign - 1);
        fieldInfos.push({ name: f.name, type: 'string', encoding, length, offset, size: byteSize });
        offset += byteSize;
        continue;
      }

      const info = typeInfo[f.type];
      if (!info) throw new Error(`Unknown type: ${f.type}`);
      const fieldAlign = this.packed ? 1 : (info.size > this.maxAlign ? this.maxAlign : info.size);
      if (!this.packed) {
        offset = (offset + fieldAlign - 1) & ~(fieldAlign - 1);
      }
      fieldInfos.push({ name: f.name, type: f.type, offset, size: info.size });
      offset += info.size;
    }
    const finalAlign = this.packed ? this.structAlign : Math.max(8, this.structAlign);
    this.size = (offset + finalAlign - 1) & ~(finalAlign - 1);
    this.fieldInfos = fieldInfos;
    this._pointerModelFields = fieldInfos.filter(f => f.type === 'model_pointer' && !f.lazy);

    const alignCap = options.maxAlign ?? 8;
    let naturalAlign = 1;
    for (const fi of fieldInfos) {
      if (fi.type === 'model_inline' || fi.type === 'array_model') {
        naturalAlign = Math.max(naturalAlign, resolveModel(fi.model).maxAlign ?? 1);
      } else if (fi.type === 'model_pointer' || fi.type === 'array_pointer' ||
                 fi.type === 'pointer' || fi.type === 'int64' || fi.type === 'uint64' || fi.type === 'float64') {
        naturalAlign = Math.max(naturalAlign, 8);
      } else if (fi.type === 'array_primitive') {
        const aInfo = typeInfo[fi.itemType];
        if (aInfo) naturalAlign = Math.max(naturalAlign, aInfo.size);
      } else if (fi.type === 'string') {
        naturalAlign = Math.max(naturalAlign, fi.encoding === 'wide' ? 2 : 1);
      } else {
        const fInfo = typeInfo[fi.type];
        if (fInfo) naturalAlign = Math.max(naturalAlign, fInfo.size);
      }
    }
    this.maxAlign = this.packed ? 1 : Math.min(naturalAlign, alignCap);

    if (options.expectedSize !== undefined && this.size !== options.expectedSize) {
      throw new Error(
        `MemoryModel '${name}' size mismatch: expected 0x${options.expectedSize.toString(16).toUpperCase()} (${options.expectedSize}), got 0x${this.size.toString(16).toUpperCase()} (${this.size})`
      );
    }

    // Generate cursor classes
    this._generateCursorClass();

    if (MemoryModel._registry[name]) {
      console.warn(`MemoryModel: duplicate registration for '${name}', overwriting previous definition`);
    }
    MemoryModel._registry[name] = this;
  }

  // Generate TypedArray-based cursor class (faster field access)
  _generateCursorClass() {
    const { fieldInfos } = this;

    const forceDataView = this.packed;

    // Determine which TypedArray views we need
    const neededViews = new Map();
    let needsDataView = forceDataView;

    for (const f of fieldInfos) {
      if (f.type === 'model_inline') continue;
      if (f.type === 'model_pointer') {
        if (f.lazy) needsDataView = true;
        continue;
      }
      // array_model sub-cursor manages its own views; array_pointer needs DataView
      if (f.type === 'array_model') continue;
      if (f.type === 'array_pointer') { needsDataView = true; continue; }
      if (f.type === 'string') continue;
      if (f.type === 'array_primitive') {
        const shift = forceDataView ? null : typeShift[f.itemType];
        if (shift === null) { needsDataView = true; }
        else {
          const info = typedArrayInfo[f.itemType];
          if (!neededViews.has(info.prop)) neededViews.set(info.prop, { ctor: info.ctor, shift });
        }
        continue;
      }

      const shift = forceDataView ? null : typeShift[f.type];
      if (shift !== null) {
        const info = typedArrayInfo[f.type];
        if (!neededViews.has(info.prop)) {
          neededViews.set(info.prop, { ctor: info.ctor, shift });
        }
      } else {
        needsDataView = true;
      }
    }

    // Build view setup code
    const viewSetup = [];
    for (const [prop, { ctor, shift }] of neededViews) {
      viewSetup.push(`this.${prop} = new ${ctor}(buffer.buffer, buffer.byteOffset, buffer.byteLength >> ${shift});`);
    }
    if (needsDataView) {
      viewSetup.push('this._dv = new DataView(buffer.buffer, buffer.byteOffset, buffer.byteLength);');
    }

    // Generate getters using TypedArray indexing where possible
    const getterLines = fieldInfos.map(f => {
      // Inline model
      if (f.type === 'model_inline') {
        const modelName = resolveModel(f.model).name;
        return `    get ${f.name}() {
      const slice = this._b.subarray(this._off + ${f.offset}, this._off + ${f.offset} + ${f.size});
      return new this._models.${modelName}.CursorClass(this.$address + ${f.offset}n, slice, 1, ${f.size}, this._models);
    }`;
      }
      // Pointer to model
      if (f.type === 'model_pointer') {
        if (f.lazy) {
          const modelName = resolveModel(f.model).name;
          return `    get ${f.name}() {
      const ptr = this._dv.getBigUint64(this._off + ${f.offset}, this._isLittleEndian);
      if (ptr === 0n) return null;
      return this._models[${JSON.stringify(modelName)}].read(ptr);
    }`;
        }
        return `    get ${f.name}() {
      if (!this._pointerModelCache) return null;
      const ec = this._pointerModelCache[this._idx];
      return ec ? ec['${f.name}'] : null;
    }`;
      }
      // Array of inline models
      if (f.type === 'array_model') {
        const modelName = resolveModel(f.model).name;
        return `    get ${f.name}() {
      const off = this._off + ${f.offset};
      const slice = this._b.subarray(off, off + ${f.size});
      return new this._models.${modelName}.CursorClass(this.$address + ${f.offset}n, slice, ${f.count}, ${f.itemSize}, this._models);
    }`;
      }
      // Array of pointer-to-model
      if (f.type === 'array_pointer') {
        return `    get ${f.name}() {
      const arr = new Array(${f.count});
      for (let i = 0; i < ${f.count}; i++) arr[i] = this._dv.getBigUint64(this._off + ${f.offset} + i * 8, this._isLittleEndian);
      return arr;
    }`;
      }

      if (f.type === 'string') {
        return `    get ${f.name}() { return _readString(this._b, this._off + ${f.offset}, ${f.size}, '${f.encoding}'); }`;
      }

      // Array of primitives
      if (f.type === 'array_primitive') {
        const shift = forceDataView ? null : typeShift[f.itemType];
        if (shift !== null) {
          const { ctor } = typedArrayInfo[f.itemType];
          return `    get ${f.name}() { return new ${ctor}(this._b.buffer, this._b.byteOffset + this._off + ${f.offset}, ${f.count}); }`;
        } else {
          const method = typeInfo[f.itemType].read;
          return `    get ${f.name}() {
      const arr = new Array(${f.count});
      for (let i = 0; i < ${f.count}; i++) arr[i] = this._dv.${method}(this._off + ${f.offset} + i * ${f.itemSize}, this._isLittleEndian);
      return arr;
    }`;
        }
      }

      const shift = forceDataView ? null : typeShift[f.type];
      if (shift !== null) {
        const { prop } = typedArrayInfo[f.type];
        const fieldIndex = f.offset >> shift;
        return shift === 0
          ? `    get ${f.name}() { return this.${prop}[this._off + ${fieldIndex}]; }`
          : `    get ${f.name}() { return this.${prop}[(this._off >> ${shift}) + ${fieldIndex}]; }`;
      } else {
        const method = typeInfo[f.type].read;
        return `    get ${f.name}() { return this._dv.${method}(this._off + ${f.offset}, this._isLittleEndian); }`;
      }
    });

    // Generate setters
    const setterLines = fieldInfos.map(f => {
      // Models and arrays don't have setters
      if (f.type === 'model_inline' || f.type === 'model_pointer' ||
          f.type === 'array_model' || f.type === 'array_pointer' || f.type === 'array_primitive') {
        return `    set ${f.name}(v) { throw new Error('Cannot directly set array/model field ${f.name}'); }`;
      }

      if (f.type === 'string') {
        return `    set ${f.name}(v) { _writeString(v, this._b, this._off + ${f.offset}, ${f.size}, '${f.encoding}'); }`;
      }

      const shift = forceDataView ? null : typeShift[f.type];
      if (shift !== null) {
        const { prop } = typedArrayInfo[f.type];
        const fieldIndex = f.offset >> shift;
        return shift === 0
          ? `    set ${f.name}(v) { this.${prop}[this._off + ${fieldIndex}] = v; }`
          : `    set ${f.name}(v) { this.${prop}[(this._off >> ${shift}) + ${fieldIndex}] = v; }`;
      } else {
        const method = typeInfo[f.type].write;
        return `    set ${f.name}(v) { this._dv.${method}(this._off + ${f.offset}, v, this._isLittleEndian); }`;
      }
    });

    const cursorCode = `
      return class Cursor_${this.name} {
        constructor(baseAddress, buffer, count, modelSize, models) {
          this._baseAddr = baseAddress;
          this._b = buffer;
          ${viewSetup.join('\n          ')}
          this._count = count;
          this._size = modelSize;
          this._idx = 0;
          this._off = 0;
          this._models = models;
          this._isLittleEndian = ${IS_LITTLE_ENDIAN};
        }

        get $index() { return this._idx; }
        get $count() { return this._count; }
        get $valid() { return this._idx < this._count; }
        get $address() { return this._baseAddr + BigInt(this._off); }

        $next() {
          this._idx++;
          this._off += this._size;
          return this._idx < this._count;
        }

        $reset() {
          this._idx = 0;
          this._off = 0;
        }

        $moveTo(index) {
          if (index < 0 || index >= this._count) return false;
          this._idx = index;
          this._off = index * this._size;
          return true;
        }

        ${getterLines.join('\n')}
        ${setterLines.join('\n')}

        $flushCurrent() {
          const slice = this._b.subarray(this._off, this._off + this._size);
          binding.writeMemory(this.$address, slice);
        }

        $flushAll() {
          binding.writeMemory(this._baseAddr, this._b);
        }
      }
    `;

    this.CursorClass = new Function(
      'binding', 'DataView', 'BigInt',
      'Int8Array', 'Uint8Array', 'Int16Array', 'Uint16Array',
      'Int32Array', 'Uint32Array', 'Float32Array', 'Float64Array',
      '_readString', '_writeString',
      cursorCode
    )(binding, DataView, BigInt, Int8Array, Uint8Array, Int16Array, Uint16Array,
      Int32Array, Uint32Array, Float32Array, Float64Array,
      _readString, _writeString);

    // Add $toObject to cursor prototype, snapshot current element into target or new object
    const model = this;
    this.CursorClass.prototype.$toObject = function (target) {
      target = model.snapshot(this, target);
      target._address = this.$address;
      return target;
    };
  }

  // resolves model thunks once and caches {name, offset, resolvedModel}[]
  get _resolvedPointerFields() {
    const resolved = this._pointerModelFields.map(f => ({
      name: f.name,
      offset: f.offset,
      resolvedModel: resolveModel(f.model),
    }));
    Object.defineProperty(this, '_resolvedPointerFields', { value: resolved, configurable: true, writable: false });
    return resolved;
  }

  // read a single object at address, guarding against circular pointer chains.
  // visited is a Set<BigInt> of addresses already seen on the current traversal path.
  _readWithVisited(address, visited) {
    const bigAddr = BigInt(address);

    // Return cached object if it's from the current generation (same tick).
    const cached = this._cache.get(bigAddr);
    if (cached !== undefined && cached.gen === MemoryModel._generation) {
      return cached.obj;
    }

    const raw = binding.readMemoryIfChanged(bigAddr, this.size >>> 0);
    if (raw === undefined && cached !== undefined) {
      cached.gen = MemoryModel._generation;
      return cached.obj;
    }
    const buffer = raw ?? binding.readMemoryFast(bigAddr, this.size >>> 0);

    const obj = new this.CursorClass(bigAddr, buffer, 1, this.size, MemoryModel._registry);

    this._cache.set(bigAddr, { obj, gen: MemoryModel._generation });

    const pointerModelFields = this._resolvedPointerFields;
    if (pointerModelFields.length > 0) {
      const view = new DataView(buffer.buffer, buffer.byteOffset, buffer.byteLength);
      const elemCache = {};
      for (const field of pointerModelFields) {
        const ptr = view.getBigUint64(field.offset, IS_LITTLE_ENDIAN);
        if (ptr === 0n || visited.has(ptr)) {
          elemCache[field.name] = null;
        } else {
          visited.add(ptr);
          elemCache[field.name] = field.resolvedModel._readWithVisited(ptr, visited);
        }
      }
      obj._pointerModelCache = [elemCache];
    }

    return obj;
  }

  // Read a single object at address
  read(address) {
    const bigAddr = BigInt(address);
    // fast path
    const cached = this._cache.get(bigAddr);
    if (cached !== undefined && cached.gen === MemoryModel._generation) {
      return cached.obj;
    }
    return this._readWithVisited(bigAddr, new Set([bigAddr]));
  }

  // Create a one-time cursor for count objects
  cursor(address, count) {
    const totalSize = this.size * count;
    const buffer = binding.readMemoryFast(BigInt(address), totalSize >>> 0);
    const cursor = new this.CursorClass(BigInt(address), buffer, count, this.size, MemoryModel._registry);

    const pointerModelFields = this._resolvedPointerFields;
    if (pointerModelFields.length > 0) {
      const view = new DataView(buffer.buffer, buffer.byteOffset, buffer.byteLength);
      const cache = new Array(count);
      const visited = new Set();
      for (let i = 0; i < count; i++) {
        const elemCache = {};
        for (const field of pointerModelFields) {
          const ptr = view.getBigUint64(i * this.size + field.offset, IS_LITTLE_ENDIAN);
          if (ptr === 0n) {
            elemCache[field.name] = null;
          } else {
            visited.clear();
            visited.add(ptr);
            elemCache[field.name] = field.resolvedModel._readWithVisited(ptr, visited);
          }
        }
        cache[i] = elemCache;
      }
      cursor._pointerModelCache = cache;
    }

    return cursor;
  }

  // Create a reusable cursor
  createCursor(maxCount) {
    const bufferSize = this.size * maxCount;
    const buffer = new Uint8Array(bufferSize);
    const cursor = new this.CursorClass(0n, buffer, 0, this.size, MemoryModel._registry);

    // Find pointer model fields that need eager loading (thunks resolved once, lazily)
    const pointerModelFields = this._resolvedPointerFields;

    cursor.$load = (address, count) => {
      if (count > maxCount) {
        throw new Error(`count ${count} exceeds maxCount ${maxCount}`);
      }
      if (count === 0) {
        cursor._baseAddr = BigInt(address);
        cursor._count = 0;
        cursor._idx = 0;
        cursor._off = 0;
        cursor._pointerModelCache = undefined;
        return;
      }
      const bigAddr = BigInt(address);
      const slice = buffer.subarray(0, this.size * count);
      let changed;
      if (binding.readMemoryIntoIfChanged) {
        changed = binding.readMemoryIntoIfChanged(bigAddr, slice);
      } else if (binding.readMemoryInto) {
        binding.readMemoryInto(bigAddr, slice);
        changed = true;
      } else {
        const tmp = binding.readMemoryFast(bigAddr, (this.size * count) >>> 0);
        buffer.set(tmp.subarray(0, this.size * count));
        changed = true;
      }
      cursor._baseAddr = bigAddr;
      cursor._count = count;
      cursor._idx = 0;
      cursor._off = 0;

      if (pointerModelFields.length > 0) {
        const view = new DataView(buffer.buffer, buffer.byteOffset, buffer.byteLength);
        const cache = new Array(count);
        const visited = new Set();
        for (let i = 0; i < count; i++) {
          const elemCache = {};
          for (const field of pointerModelFields) {
            const ptr = view.getBigUint64(i * this.size + field.offset, IS_LITTLE_ENDIAN);
            if (ptr === 0n) {
              elemCache[field.name] = null;
            } else {
              visited.clear();
              visited.add(ptr);
              elemCache[field.name] = field.resolvedModel._readWithVisited(ptr, visited);
            }
          }
          cache[i] = elemCache;
        }
        cursor._pointerModelCache = cache;
      }

      return changed;
    };

    return cursor;
  }

  // Read array of objects (returns array of MemoryObject instances)
  readArray(address, count) {
    const totalSize = this.size * count;
    const buffer = binding.readMemoryFast(BigInt(address), totalSize >>> 0);
    const result = new Array(count);
    const pointerModelFields = this._resolvedPointerFields;
    // Hoist DataView and visited Set outside the element loop
    const view = pointerModelFields.length > 0
      ? new DataView(buffer.buffer, buffer.byteOffset, buffer.byteLength)
      : null;
    const visited = pointerModelFields.length > 0 ? new Set() : null;

    for (let i = 0; i < count; i++) {
      const slice = buffer.subarray(i * this.size, (i + 1) * this.size);
      const cursor = new this.CursorClass(BigInt(address) + BigInt(i * this.size), slice, 1, this.size, MemoryModel._registry);

      if (view !== null) {
        const elemCache = {};
        for (const field of pointerModelFields) {
          const ptr = view.getBigUint64(i * this.size + field.offset, IS_LITTLE_ENDIAN);
          if (ptr === 0n) {
            elemCache[field.name] = null;
          } else {
            visited.clear();
            visited.add(ptr);
            elemCache[field.name] = field.resolvedModel._readWithVisited(ptr, visited);
          }
        }
        cursor._pointerModelCache = [elemCache];
      }

      result[i] = cursor;
    }

    return result;
  }

  // Initialize all fields on a target object with zero values based on fieldInfos.
  // Sets 0 for numbers, 0n for bigints/pointers, null for model fields.
  initSnapshot(target) {
    for (const field of this.fieldInfos) {
      if (field.type === 'model_inline') {
        const resolvedModel = resolveModel(field.model);
        const Cls = snapshotRegistry._models[resolvedModel.name];
        target[field.name] = Cls ? new Cls() : {};
        resolvedModel.initSnapshot(target[field.name]);
      } else if (field.type === 'model_pointer') {
        target[field.name] = null;
      } else if (field.type === 'array_model') {
        const resolvedModel = resolveModel(field.model);
        const Cls = snapshotRegistry._models[resolvedModel.name];
        target[field.name] = Array.from({ length: field.count }, () => {
          const elem = Cls ? new Cls() : {};
          resolvedModel.initSnapshot(elem);
          return elem;
        });
      } else if (field.type === 'array_pointer') {
        target[field.name] = new Array(field.count).fill(0n);
      } else if (field.type === 'array_primitive') {
        const isBI = field.itemType === 'int64' || field.itemType === 'uint64' || field.itemType === 'pointer';
        target[field.name] = new Array(field.count).fill(isBI ? 0n : 0);
      } else if (field.type === 'string') {
        target[field.name] = '';
      } else if (field.type === 'int64' || field.type === 'uint64' || field.type === 'pointer') {
        target[field.name] = 0n;
      } else {
        target[field.name] = 0;
      }
    }
    target['_address'] = 0n;
  }

  // Build a per-model snapshot function via new Function() so field names, types, and
  // model references are baked in as literals
  _compileSnapshotFn() {
    const captureKeys = [];
    const captureVals = [];
    const stmts = [];

    const cap = (val) => {
      const k = `_c${captureKeys.length}`;
      captureKeys.push(k);
      captureVals.push(val);
      return k;
    };

    const cReg = cap(snapshotRegistry);

    for (const field of this.fieldInfos) {
      const n = JSON.stringify(field.name);
      if (field.type === 'model_inline' || field.type === 'model_pointer') {
        if (field.type === 'model_pointer' && field.lazy) {
          stmts.push(`tgt[${n}]=null`);
        } else {
          const rm = resolveModel(field.model);
          const cModel = cap(rm);
          const clsKey = JSON.stringify(rm.name);
          stmts.push(
            `var _v=src[${n}];` +
            `if(_v){if(!tgt[${n}]){var _C=${cReg}._models[${clsKey}];tgt[${n}]=_C?new _C():{}}` +
            `${cModel}.snapshot(_v,tgt[${n}])}else{tgt[${n}]=null}`
          );
        }
      } else if (field.type === 'array_model') {
        const rm = resolveModel(field.model);
        const cModel = cap(rm);
        const clsKey = JSON.stringify(rm.name);
        stmts.push(
          `{var _sub=src[${n}],_arr=tgt[${n}]||new Array(${field.count}),_C=${cReg}._models[${clsKey}];` +
          `_sub.$reset();` +
          `for(var _i=0;_i<${field.count};_i++){var _e=_arr[_i]||(_C?new _C():{});${cModel}.snapshot(_sub,_e);_arr[_i]=_e;_sub.$next()}` +
          `tgt[${n}]=_arr}`
        );
      } else if (field.type === 'array_primitive') {
        stmts.push(`tgt[${n}]=Array.from(src[${n}])`);
      } else if (field.type === 'array_pointer') {
        stmts.push(`tgt[${n}]=src[${n}].slice()`);
      } else {
        stmts.push(`tgt[${n}]=src[${n}]`);
      }
    }

    const body = `return function snapshot_${this.name}(src,tgt){\n  ${stmts.join(';\n  ')};\n  return tgt;\n}`;
    this._snapshotFn = new Function(...captureKeys, body)(...captureVals);
  }

  snapshot(source, target) {
    if (!this._snapshotFn) this._compileSnapshotFn();
    if (!target) target = {};
    return this._snapshotFn(source, target);
  }

  verifySize(expected) {
    if (this.size !== expected) {
      throw new Error(
        `MemoryModel '${this.name}' size mismatch: expected 0x${expected.toString(16).toUpperCase()} (${expected}), got 0x${this.size.toString(16).toUpperCase()} (${this.size})`
      );
    }
    return this;
  }

  extend(name, fields) {
    return new MemoryModel(name, fields, this);
  }

  static define(name, fields, parent, options = {}) {
    return new MemoryModel(name, fields, parent, options);
  }
}

// Global model registry: populated by every MemoryModel constructor.
// Cursor getters look up models here at use-time so forward-referenced and
// self-referential models always resolve correctly.
MemoryModel._registry = Object.create(null);

// Monotonically-increasing generation counter.  Every per-model _cache entry
// stores the generation at which it was written.  A cache hit is only valid
// when entry.gen === MemoryModel._generation.
//
// Call MemoryModel.invalidateCache() at the start of each game tick so that
// the next read for any address fetches fresh data from memory.
MemoryModel._generation = 0;
MemoryModel.invalidateCache = function invalidateCache() {
  MemoryModel._generation++;
};

const snapshotRegistry = {
  _models: {},

  // Register a class for a model name. When snapshot encounters this model,
  // it will create instances of this class.
  // Usage: registry.add(Seed, 'SeedModel')
  add(cls, modelName) {
    if (snapshotRegistry._models[modelName]) {
      console.warn(`snapshotRegistry: duplicate registration for '${modelName}', overwriting previous class`);
    }
    snapshotRegistry._models[modelName] = cls;
  },
};

// TODO: move game lock to game-lock.js

/**
 * Acquire the game lock (waits for game thread to open its window)
 * @param {number} timeout - Timeout in milliseconds (default: 100)
 * @returns {boolean} - True if lock was acquired
 */
function acquireGameLock(timeout = 100) {
  return binding.acquireGameLock(timeout);
}

/**
 * Release the game lock
 */
function releaseGameLock() {
  binding.releaseGameLock();
}

/**
 * Check if the game lock is currently held
 * @returns {boolean}
 */
function isGameLockHeld() {
  return binding.isGameLockHeld();
}

/**
 * Check if the game lock window is currently open
 * @returns {boolean}
 */
function isGameLockOpen() {
  return binding.isGameLockOpen();
}

/**
 * Execute a function while holding the game lock.
 * Ensures the lock is released even if an error occurs.
 *
 * @param {Function} fn - Function to execute while holding the lock
 * @param {number} timeout - Timeout in milliseconds to acquire lock (default: 100)
 * @returns {*} - Return value of fn
 * @throws {Error} - If lock cannot be acquired, or if fn throws
 *
 * @example
 * const data = withGameLock(() => {
 *   const player = memory.readMemoryFast(playerAddr, playerSize);
 *   const enemy = memory.readMemoryFast(enemyAddr, enemySize);
 *   return { player, enemy };
 * });
 */
function withGameLock(fn, timeout = 100) {
  if (!binding.acquireGameLock(timeout)) {
    throw new Error('Failed to acquire game lock');
  }
  try {
    return fn();
  } finally {
    binding.releaseGameLock();
  }
}

/**
 * Try to execute a function while holding the game lock.
 * Returns undefined if lock cannot be acquired (does not throw).
 *
 * @param {Function} fn - Function to execute while holding the lock
 * @param {number} timeout - Timeout in milliseconds to acquire lock (default: 100)
 * @returns {*} - Return value of fn, or undefined if lock not acquired
 *
 * @example
 * const data = tryWithGameLock(() => {
 *   return memory.readMemoryFast(addr, size);
 * });
 * if (data !== undefined) {
 *   // process data
 * }
 */
function tryWithGameLock(fn, timeout = 100) {
  if (!binding.acquireGameLock(timeout)) {
    return undefined;
  }
  try {
    return fn();
  } finally {
    binding.releaseGameLock();
  }
}

module.exports = {
  MemoryModel,
  DataTypes,
  snapshotRegistry,
  invalidateCache: MemoryModel.invalidateCache,

  readMemory: binding.readMemory,
  readMemoryFast: binding.readMemoryFast,
  readMemoryInto: binding.readMemoryInto,
  writeMemory: binding.writeMemory,

  allocateTestMemory: binding.allocateTestMemory,
  freeTestMemory: binding.freeTestMemory,
  freeAllTestMemory: binding.freeAllTestMemory,
  highResolutionTime: binding.highResolutionTime,

  acquireGameLock,
  releaseGameLock,
  isGameLockHeld,
  isGameLockOpen,
  withGameLock,
  tryWithGameLock,
};
