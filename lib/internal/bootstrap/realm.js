'use strict';

// Bootstrap the internal module system
// This is the first file executed to set up module loading infrastructure

// internalBinding returns native bindings by name
const moduleLoadList = [];
let internalBinding;
{
  const bindingObj = { __proto__: null };
  internalBinding = function internalBinding(module) {
    let mod = bindingObj[module];
    if (typeof mod !== 'object') {
      mod = bindingObj[module] = getInternalBinding(module);
      moduleLoadList.push(`Internal Binding ${module}`);
    }
    return mod;
  };
}
globalThis.internalBinding = internalBinding;

const selfId = 'internal/bootstrap/realm';
const {
  builtinIds,
  compileFunction,
  setInternalLoaders,
} = internalBinding('builtins');

const { ModuleWrap } = internalBinding('module_wrap');
Object.setPrototypeOf(ModuleWrap.prototype, null);

const getOwn = (target, property, receiver) => {
  return Object.prototype.hasOwnProperty.call(target, property) ?
    Reflect.get(target, property, receiver) :
    undefined;
}

const publicBuiltinIds = builtinIds.filter((id) =>
  !id.startsWith('internal/'));
const internalBuiltinIds = builtinIds.filter((id) =>
  id.startsWith('internal/') && id !== selfId);


let canBeRequiredByUsersList = new Set(publicBuiltinIds);
let canBeRequiredByUsersWithoutSchemeList = new Set(publicBuiltinIds);

class BuiltinModule {
  /**
   * A map from the module IDs to the module instances.
   * @type {Map<string, BuiltinModule>}
   */
  static map = new Map(builtinIds.map((id) => [id, new BuiltinModule(id)]));

  constructor(id) {
    this.filename = `${id}.js`;
    this.id = id;

    // The CJS exports object of the module
    this.exports = {};
    // States used to work around circular dependencies
    this.loaded = false;
    this.loading = false;

    // The following properties are used by the ESM implementation and only initialized when the built-in module is loaded by users
    /**
     * The C++ ModuleWrap binding used to interface with the ESM implementation
     * @type {ModuleWrap|undefined}
     */
    this.module = undefined;
    /**
     * Exported names for the ESM imports
     */
    this.exportKeys = undefined;
  }

  static allowRequireByUsers(id) {
    if (id === selfId) {
      throw new Error(`Should not allow ${id}`);
    }
    canBeRequiredByUsersList.add(id);
    canBeRequiredByUsersWithoutSchemeList.add(id);
  }

  static setRealmAllowRequireByUsers(ids) {
    canBeRequiredByUsersList = new Set(ids.filter((id) => publicBuiltinIds.includes(id)));
    canBeRequiredByUsersWithoutSchemeList = new Set(ids);
  }

  // To be called during pre-execution.
  // Enabled the user-land module loader to access internal modules
  static exposeInternals() {
    for (let i = 0; i < internalBuiltinIds.length; ++i) {
      BuiltinModule.allowRequireByUsers(internalBuiltinIds[i]);
    }
  }

  static exists(id) {
    return BuiltinModule.map.has(id);
  }

  static canBeRequiredByUsers(id) {
    return canBeRequiredByUsersList.has(id);
  }

  static canBeRequiredWithoutScheme(id) {
    return canBeRequiredByUsersWithoutSchemeList.has(id);
  }

  static normalizeRequirableId(id) {
    if (id.startsWith('nyx:')) {
      const normalizeId = id.slice(4);
      if (BuiltinModule.canBeRequiredByUsers(normalizeId)) {
        return normalizeId;
      }
    } else if (BuiltinModule.canBeRequiredWithoutScheme(id)) {
      return id;
    }
    return undefined;
  }

  static isBuiltin(id) {
    return BuiltinModule.canBeRequiredWithoutScheme(id) || (
      typeof id === 'string' &&
      id.startsWith('nyx:') &&
      BuiltinModule.canBeRequiredByUsers(id.slice(4)));
  }

  static getAllBuiltinModuleIds() {
    const allBuiltins = Array.from(canBeRequiredByUsersWithoutSchemeList);
    return allBuiltins;
  }

  // Used by user-land module loaders to compile and load builtins
  compileForPublicLoader() {
    if (!BuiltinModule.canBeRequiredByUsers(this.id)) {
      throw new Error(`Should not compile ${this.id} for public use`);
    }
    this.compileForInternalLoader();
    if (!this.exportKeys) {
      // When exposing internal, we do not want to reflect the named exports from the core modules as this can trigger unnecessary getters.
      const internal = this.id.startsWith('internal/');
      this.exportKeys = internal ? [] : Object.keys(this.exports);
    }
    return this.exports;
  }

  getESMFacade() {
    if (this.module) { return this.module }
    const url = `nyx:${this.id}`;
    const builtin = this;
    const exportsKeys = this.exportKeys.slice();
    if (!exportsKeys.includes('default')) {
      exportsKeys.push('default');
    }
    this.module = new ModuleWrap(
      url, exportsKeys, function () {
        builtin.syncExports();
        this.setExport('default', builtin.exports);
      });
    this.module.instantiate();
    this.module.evaluate();
    return this.module;
  }

  // Provide named exports for all builtin libraries so that the libraries
  // may be imported in a nicer way for ESM users. The default export is left
  // as the entire namespace (module.exports) and updates when this function is
  // called so that APMs and other behavior are supported.
  syncExports() {
    const names = this.exportKeys;
    if (this.module) {
      for (let i = 0; i < names.length; ++i) {
        const exportName = names[i];
        if (exportName === 'default') { continue; }
        this.module.setExport(exportName, getOwn(this.exports, exportName, this.exports));
      }
    }
  }

  compileForInternalLoader() {
    if (this.loaded || this.loading) {
      return this.exports;
    }

    const id = this.id;
    this.loading = true;

    try {
      const requireFn = requireBuiltin;
      const fn = compileFunction(id);
      // Must match arguments in BuiltinLoader::CompileAndCall
      fn(this.exports, requireFn, this, internalBinding);

      this.loaded = true;
    } finally {
      this.loading = false;
    }

    moduleLoadList.push(`BuiltinModule ${id}`);
    return this.exports;
  }
}

const loaderExports = {
  internalBinding,
  BuiltinModule,
  require: requireBuiltin,
};

function requireBuiltin(id) {
  if (id === selfId) {
    return loaderExports;
  }

  const mod = BuiltinModule.map.get(id);
  if (!mod) {
    throw new TypeError(`Missing internal module '${id}'`);
  }
  return mod.compileForInternalLoader();
}

// Store the internal loaders in C++.
setInternalLoaders(internalBinding, requireBuiltin);
