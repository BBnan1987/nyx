'use strict';

// TODO:
// This very simply scans the scripts directory set in C++ when creating Environment then runs all packages found.
// Instead rename (or create new) run_packages.js to run_nyx and initialize a feature-complete "manager" that loads
// config files (e.g. to override scripts directory), data files, create an imgui interface to configure etc.

// Load the package registry
const packages = require('internal/modules/package');

// Scan for packages (requires scriptsRoot to be set from C++)
packages.scanPackages();

// Load the module loader
const loader = require('internal/modules/loader');

// Execute all runtime packages
loader.runRuntimes();
