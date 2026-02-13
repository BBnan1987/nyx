'use strict';

// Package registry module
// Provides package discovery and resolution for user-land modules

const fs = require('fs');

const process = internalBinding('process');

const packageRegistry = new Map();
const packagesByPath = new Map();

/**
 * Parse a package.json file
 * @param {string} packageJsonPath - Path to package.json
 * @returns {Object|null} Package info
 */
function parsePackageJson(packageJsonPath) {
  try {
    const content = fs.readFileSync(packageJsonPath, 'utf8');
    const pkg = JSON.parse(content);

    if (!pkg.name) {
      debugLog('Warning: package.json missing "name" field: ' + packageJsonPath);
      return null;
    }

    // get directory containing package.json
    const lastSlash = Math.max(
      packageJsonPath.lastIndexOf('/'),
      packageJsonPath.lastIndexOf('\\')
    );
    const dir = packageJsonPath.substring(0, lastSlash).replace(/\\/g, '/');

    // resolve main entry point
    let main = pkg.main || 'index.js';
    if (!main.endsWith('.js') && !main.endsWith('.mjs')) {
      main = main + '.js';
    }
    const mainPath = dir + '/' + main;

    // parse dependencies
    const dependencies = [];
    if (pkg.dependencies && typeof pkg.dependencies === 'object') {
      for (const depName of Object.keys(pkg.dependencies)) {
        dependencies.push(depName);
      }
    }

    return {
      name: pkg.name,
      dir: dir,
      main: mainPath,
      library: pkg.library === true,
      dependencies: dependencies,
      raw: pkg
    };
  } catch (e) {
    debugLog('Warning: Failed to parse package.json: ' + packageJsonPath + ' - ' + e.message);
    return null;
  }
}

/**
 * Scan the scripts root for packages
 * Only scans immediate subdirectories (scripts/foo/package.json)
 */
function scanPackages() {
  let scriptsRoot = process.scriptsRoot();
  debugLog('Debug: scriptsRoot from C++ = ' + scriptsRoot);
  if (!scriptsRoot) {
    debugLog('Warning: Scripts root not set, package scanning skipped');
    return;
  }

  // normalize path separators
  scriptsRoot = scriptsRoot.replace(/\\/g, '/');
  debugLog('Debug: normalized scriptsRoot = ' + scriptsRoot);

  packageRegistry.clear();
  packagesByPath.clear();

  let entries;
  try {
    entries = fs.readdirSync(scriptsRoot);
  } catch (e) {
    debugLog('Warning: Failed to read scripts root: ' + scriptsRoot + ' - ' + e.message);
    return;
  }

  for (let i = 0; i < entries.length; i++) {
    const entry = entries[i];
    const subdir = scriptsRoot + '/' + entry;

    try {
      const stats = fs.statSync(subdir);
      if (!stats.isDirectory()) {
        continue;
      }
    } catch (e) {
      continue;
    }

    const packageJsonPath = subdir + '/package.json';
    if (!fs.existsSync(packageJsonPath)) {
      continue;
    }

    const pkg = parsePackageJson(packageJsonPath);
    if (!pkg) {
      continue;
    }

    if (packageRegistry.has(pkg.name)) {
      debugLog('Warning: Duplicate package name "' + pkg.name + '" found at: ' + subdir);
      continue;
    }

    packageRegistry.set(pkg.name, pkg);
    packagesByPath.set(pkg.dir, pkg);

    debugLog('Registered package: ' + pkg.name + ' (' + (pkg.library ? 'library' : 'runtime') + ')');
  }
}

/**
 * Get a package by name
 */
function getPackage(name) {
  return packageRegistry.get(name);
}

/**
 * Get a package by directory path
 */
function getPackageByPath(dir) {
  const normalized = dir.replace(/\\/g, '/');
  return packagesByPath.get(normalized);
}

/**
 * Find which package a file belongs to
 */
function getPackageForFile(filePath) {
  const normalized = filePath.replace(/\\/g, '/');

  for (const [dir, pkg] of packagesByPath) {
    if (normalized.startsWith(dir + '/') || normalized === dir) {
      return pkg;
    }
  }

  return undefined;
}

/**
 * Resolve a bare specifier (package name) from a referrer
 * @returns {string|null} Resolved path to main entry, or null if not found
 */
function resolvePackage(specifier, referrer) {
  const normalizedReferrer = referrer.replace(/\\/g, '/');
  const referrerPkg = getPackageForFile(normalizedReferrer);

  if (!referrerPkg) {
    debugLog('Debug: Looking for package containing: ' + normalizedReferrer);
    debugLog('Debug: Registered packages:');
    for (const [dir, pkg] of packagesByPath) {
      debugLog('  - ' + dir + ' (' + pkg.name + ')');
    }
    return null;
  }

  if (!referrerPkg.dependencies.includes(specifier)) {
    debugLog('Debug: Package "' + referrerPkg.name + '" does not have "' + specifier + '" in dependencies');
    debugLog('Debug: Dependencies are: ' + referrerPkg.dependencies.join(', '));
    return null;
  }

  const targetPkg = packageRegistry.get(specifier);
  if (!targetPkg) {
    debugLog('Debug: Target package "' + specifier + '" not found in registry');
    return null;
  }

  if (!targetPkg.library) {
    debugLog('Warning: Cannot import non-library package "' + specifier + '"');
    return null;
  }

  return targetPkg.main;
}

/**
 * Get all registered packages
 */
function getAllPackages() {
  return packageRegistry;
}

/**
 * Check if a specifier is a bare specifier (package name)
 */
function isBareSpecifier(specifier) {
  if (specifier.startsWith('./') || specifier.startsWith('../')) {
    return false;
  }
  if (specifier.startsWith('/') || /^[a-zA-Z]:/.test(specifier)) {
    return false;
  }
  if (specifier.startsWith('nyx:')) {
    return false;
  }
  return true;
}

/**
 * Get all runtime (non-library) packages
 * @returns {Array} Array of runtime package info objects
 */
function getRuntimePackages() {
  const runtimes = [];
  for (const [name, pkg] of packageRegistry) {
    if (!pkg.library) {
      runtimes.push(pkg);
    }
  }
  return runtimes;
}

module.exports = {
  scanPackages,
  getPackage,
  getPackageByPath,
  getPackageForFile,
  resolvePackage,
  getAllPackages,
  getRuntimePackages,
  isBareSpecifier
};
