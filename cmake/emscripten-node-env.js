// SPDX-License-Identifier: GPL-2.0-only
// Node-backed command-line tools need the host environment as well as its files.
// Populate Emscripten's virtual environment before C code calls getenv().
Module['preRun'] = [function () {
    if (typeof ENV === 'object' && typeof process === 'object'
        && process.versions && process.versions.node) {
        Object.keys(process.env).forEach(function (name) {
            if (!Object.prototype.hasOwnProperty.call(ENV, name)) {
                ENV[name] = process.env[name];
            }
        });
    }
}].concat(Module['preRun'] || []);
