'use strict';

const binding = internalBinding('fs');

function statSync(path) {
  const stats = binding.statSync(path);

  const _isFile = stats.isFile;
  const _isDirectory = stats.isDirectory;

  stats.isFile = function () {
    return _isFile;
  };
  stats.isDirectory = function () {
    return _isDirectory;
  };

  return stats;
}

function appendFileSync(path, data, options) {
  let existing = '';
  try {
    if (binding.existsSync(path)) {
      existing = binding.readFileSync(path, 'utf8');
    }
  } catch (e) {
    // File doesn't exist, that's fine
  }

  const content = typeof data === 'string' ? data : String(data);
  binding.writeFileSync(path, existing + content, options);
}

function copyFileSync(src, dest) {
  const content = binding.readFileSync(src);
  binding.writeFileSync(dest, content);
}

function rmSync(path, options = {}) {
  const { recursive = false, force = false } = options;

  try {
    const stats = statSync(path);

    if (stats.isDirectory()) {
      if (recursive) {
        const entries = binding.readdirSync(path);
        for (const entry of entries) {
          rmSync(path + '/' + entry, { recursive: true, force });
        }
        binding.rmdirSync(path);
      } else {
        binding.rmdirSync(path);
      }
    } else {
      binding.unlinkSync(path);
    }
  } catch (e) {
    if (!force) {
      throw e;
    }
  }
}

function wrapStats(stats) {
  const _isFile = stats.isFile;
  const _isDirectory = stats.isDirectory;

  stats.isFile = function () {
    return _isFile;
  };
  stats.isDirectory = function () {
    return _isDirectory;
  };

  return stats;
}

// readFile(path, encoding?) -> Promise<string|Uint8Array>
function readFile(path, encoding) {
  return binding.readFile(path, encoding);
}

// writeFile(path, data) -> Promise<void>
function writeFile(path, data) {
  return binding.writeFile(path, data);
}

// stat(path) -> Promise<Stats>
async function stat(path) {
  const stats = await binding.stat(path);
  return wrapStats(stats);
}

// readdir(path) -> Promise<string[]>
function readdir(path) {
  return binding.readdir(path);
}

// mkdir(path, options?) -> Promise<void>
function mkdir(path, options) {
  return binding.mkdir(path, options);
}

// unlink(path) -> Promise<void>
function unlink(path) {
  return binding.unlink(path);
}

// rmdir(path) -> Promise<void>
function rmdir(path) {
  return binding.rmdir(path);
}

// rename(oldPath, newPath) -> Promise<void>
function rename(oldPath, newPath) {
  return binding.rename(oldPath, newPath);
}

async function rm(path, options = {}) {
  const { recursive = false, force = false } = options;

  try {
    const stats = await stat(path);

    if (stats.isDirectory()) {
      if (recursive) {
        const entries = await readdir(path);
        for (const entry of entries) {
          await rm(path + '/' + entry, { recursive: true, force });
        }
        await rmdir(path);
      } else {
        await rmdir(path);
      }
    } else {
      await unlink(path);
    }
  } catch (e) {
    if (!force) {
      throw e;
    }
  }
}

const promises = {
  readFile,
  writeFile,
  stat,
  readdir,
  mkdir,
  unlink,
  rmdir,
  rename,
  rm
};

module.exports = {
  readFileSync: binding.readFileSync,
  writeFileSync: binding.writeFileSync,
  existsSync: binding.existsSync,
  statSync,
  readdirSync: binding.readdirSync,
  mkdirSync: binding.mkdirSync,
  unlinkSync: binding.unlinkSync,
  rmdirSync: binding.rmdirSync,
  renameSync: binding.renameSync,
  realpathSync: binding.realpathSync,
  appendFileSync,
  copyFileSync,
  rmSync,

  readFile,
  writeFile,
  stat,
  readdir,
  mkdir,
  unlink,
  rmdir,
  rename,
  rm,
  promises
};
