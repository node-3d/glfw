# GLFW for Node.js

This is a part of [Node3D](https://github.com/node-3d) project.

[![NPM](https://badge.fury.io/js/@node-3d%2Fglfw.svg)](https://badge.fury.io/js/@node-3d/glfw)
[![Lint](https://github.com/node-3d/glfw/actions/workflows/lint.yml/badge.svg)](https://github.com/node-3d/glfw/actions/workflows/lint.yml)
[![Test](https://github.com/node-3d/glfw/actions/workflows/test.yml/badge.svg)](https://github.com/node-3d/glfw/actions/workflows/test.yml)
[![Cpplint](https://github.com/node-3d/glfw/actions/workflows/cpplint.yml/badge.svg)](https://github.com/node-3d/glfw/actions/workflows/cpplint.yml)

```bash
npm install @node-3d/glfw
```



**Node.js** addon with **GLFW3** bindings.

![Example](examples/screenshot.jpg)

* **GLFW** version **3.4.0** backend.
* Exposes low-level **GLFW** interface.
* Multiple windows for a single **Node.js** process.
* Able to switch to fullscreen and back.
* Has `Window` class, simplifying low-level interactions.
* Has `Document` class, capable of tricking other libs, as if we are in a browser.

The package has named exports only. Use `glfw` for the raw native bindings,
and import `Window` or `Document` directly for the higher-level classes.

```ts
import { glfw, Window } from '@node-3d/glfw';

const wnd = new Window({ title: 'GLFW Test', vsync: true });

wnd.loop(() => {
	if (wnd.shouldClose || wnd.getKey(glfw.KEY_ESCAPE)) {
		process.exit(0);
		return;
	}
	
	glfw.testScene(wnd.width, wnd.height);
});
```

> Note: this **addon uses N-API**, and therefore is ABI-compatible across different
Node.js versions. Addon binaries are precompiled and **there is no compilation**
step during the `npm install` command.


## GLFW

This is a low-level interface, where most of the stuff is directly reflecting
GLFW API. GLFW **does NOT EXPOSE** OpenGL commands, it only [controls the window-related
setup and resources](http://www.glfw.org/docs/latest/group__window.html).
To access OpenGL/WebGL API you can use [@node-3d/webgl](https://github.com/node-3d/webgl)
or any other similar addon.

Aside from several additional features, this addon directly exposes the GLFW API to JS. E.g.:

```cpp
DBG_EXPORT JS_METHOD(pollEvents) {
	glfwPollEvents();
	RET_GLFW_VOID;
}
```

Nothing is added between you and GLFW, unless necessary or explicitly mentioned.

* All `glfw*` functions are accessible as
    `glfw.*`. E.g. `glfwPollEvents` -> `glfw.pollEvents`.
* All `GLFW_*` constants are accessible as
    `glfw.*`. E.g. `GLFW_TRUE` -> `glfw.TRUE`.
* Higher-level helpers are separate named exports.
    E.g. `import { Window, Document } from '@node-3d/glfw'`.
* Method `glfw.createWindow` takes some additional arguments. This is mostly related to
    JS events being generated from GLFW callbacks,
    and here's where you provide an Emitter object.
* Pointers are directly exposed as numbers to JS and are expected as
    arguments in specific methods. Such as, `glfw.createWindow` returns a number
    (pointer), and then you provide it back to e.g. `glfw.setWindowTitle`.

See [this example](examples/vulkan.ts) for raw GLFW calls.

The public entrypoint exports `glfw`, `Window`, `Document`, and event/window option types.
The lower-level raw API is on `glfw`; the higher-level classes are imported directly.

----------


### class Window

```ts
import { Window } from '@node-3d/glfw';

const wnd = new Window({ title: 'GLFW Test', vsync: true });
```

This class helps managing window objects and their events. It can also switch between
fullscreen, borderless and windowed modes.

The first window creates an additional invisible root-window for context sharing
(so that you can also close any window and still keep the root context).
The platform context (pointers/handles) for sharing may be obtained when necessary.

See [`ts/window.ts`](ts/window.ts) for more details.

----------

### class Document

```ts
import { Document } from '@node-3d/glfw';

const doc = new Document({ title: 'GLFW Test', vsync: true });
```

Document inherits from `Window` and has the same features in general.
It exposes additional APIs to mimic the content of web `document`.
There are some tricks to provide WebGL libraries with necessary environment.
Document is specifically designed for compatibility with [three.js](https://threejs.org/).
Other web libraries may work too, but may require additional tweaking.

See [`ts/document.ts`](ts/document.ts) for more details.

----------

### Extras

* `glfw.hideConsole(): void` - tries to hide the console window on Windows.
* `glfw.showConsole(): void` - shows the console window if it has been hidden.
* `glfw.drawWindow(w: number, cb: (dateNow: number) => void): void` - this is a shortcut
    to call `pollEvents`, then `cb`, and then `swapBuffers`. `Window#drawWindow`
    wraps this call and supplies the window handle for you.
* `glfw.platformDevice(): number` - returns the native display or device handle,
    or whatever is similar on other systems.
* `glfw.platformWindow(w: number): number` - returns the window HWND on Windows,
    or whatever is similar on other systems.
* `glfw.platformContext(w: number): number` - returns the window WGL Context on Windows,
    or whatever is similar on other systems.

## Binary Origin

Release archives are built by this repository's public GitHub Actions workflows.

Attestations: https://github.com/node-3d/glfw/attestations

To verify a downloaded archive:

```bash
gh release download <tag> -R node-3d/glfw -p <platform>.gz
gh attestation verify <platform>.gz -R node-3d/glfw
```
