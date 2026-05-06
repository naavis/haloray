# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

HaloRay is an atmospheric ice-halo (sun halo) simulator. The repository contains two codebases:

- **Root (this project)** — In-progress web port: React 19 + TypeScript + Vite + WebGPU (WGSL compute shaders). The `webgpu` branch is where active work happens.
- **[desktop/](desktop/)** — The original C++17 / Qt 5 / OpenGL 4.4 desktop application it is being ported from. Has its own [CLAUDE.md](desktop/CLAUDE.md); refer to it when cross-checking physics/behavior against the reference implementation.

## Commands

```bash
npm run dev           # Vite dev server with HMR
npm run build         # tsc -b && vite build (type-check then bundle)
npm run lint          # eslint .
npm run lint -- --fix # eslint . --fix
npm run format        # prettier --write .
npm run format:check  # prettier --check . (used in CI / pre-commit)
npm run preview       # preview production build
```

Requires a WebGPU-capable browser. There is no test suite in the web port yet.

## Architecture

### Engine and rendering pipeline

[HaloEngine](src/engine/HaloEngine.ts) is the coordinator that owns the GPU device, shared buffers, and the animation frame loop. It delegates work to separate pass classes:

- [HaloPass](src/engine/HaloPass.ts) — **raytrace** + **accumulate** compute passes (progressive). Owns pipelines, ray buffer, sim-params buffer, a tiny acc-params uniform (just `resolution_x`), and the `totalRays` counter. Per frame it loops over all enabled `CrystalPopulation` entries in `simParams.populations`, allocating workgroups proportionally to each population's weight and issuing one raytrace+accumulate dispatch pair per population. All contributions write into the same shared accumulation buffer.
- [HaloEngine](src/engine/HaloEngine.ts) also computes the Hosek-Wilkie solar spectrum (via [sun-spectrum.ts](src/engine/hosek-wilkie-sky/sun-spectrum.ts), backed by [dataset-solar.ts](src/engine/hosek-wilkie-sky/dataset-solar.ts) ported from the desktop `solarDatasets` table) whenever the sun moves or `showSky` toggles, and feeds it to `HaloPass.setSunSpectrum`.
- [SkyPass](src/engine/SkyPass.ts) — **sky** compute pass. Evaluates the Hosek-Wilkie analytic sky model and writes per-pixel linear sRGB into a sky buffer. Rebuilds the `SkyState` (per-channel configs + radiance scales from [hosek-wilkie-sky/calculate.ts](src/engine/hosek-wilkie-sky/calculate.ts)) per dispatch — cheap CPU-side Bézier evaluation. Also renders the solar disk with limb darkening and a top/bottom radiance gradient via `buildSunDiskState` ([sun-spectrum.ts](src/engine/hosek-wilkie-sky/sun-spectrum.ts)), which integrates the spectral solar radiance against the CIE 1931 2° CMF tables to produce per-channel XYZ values fed to the shader's `render_sun()`. The disk is added to the sky XYZ before the XYZ→sRGB conversion. For sun altitudes near and below the horizon the shader crossfades to the simpler Preetham analytic model (which Hosek doesn't define), then fades to black at -10°. Only re-runs when view params (camera + sun position) change.
- [GuidesPass](src/engine/GuidesPass.ts) — **guides** compute pass. Writes per-pixel linear RGBA into a guides buffer (4 floats/pixel). Only re-runs when view params change. Currently a placeholder that writes transparent black.
- [DisplayPass](src/engine/DisplayPass.ts) — **display** render pass ([display.wgsl](src/shaders/display.wgsl)). Owns the render pipeline, display-params uniform, canvas context, and its own dirty flag. Composites the other passes' outputs: sums halo and sky in linear radiance space, applies Reinhard tone mapping and sRGB gamma, then alpha-blends the guides overlay on top.

The accumulation buffer and sky buffer are sized `width * height * 3 * 4` bytes; the guides buffer is `width * height * 4 * 4` bytes (RGBA). All are rebuilt on resize (via `ResizeObserver`). The display pass only re-runs when its own dirty flag is set — HaloEngine calls `displayPass.markDirty()` whenever any compute pass produced new output.

View params (`sunAlt`, `camPitch`, `camYaw`, `camFov`, `projection`) are tracked via `didViewChange()` in [params.ts](src/state/params.ts). Changes to view params mark the sky pass dirty; all simParams changes reset the halo accumulation.

#### Engine start/stop lifecycle

`HaloEngine` has an explicit start/stop model gated on a `userStarted` flag. `start()` sets `userStarted = true` and schedules the frame loop; `stop()` clears it and cancels the animation frame. The internal `ensureRunning()` (called on param changes that would otherwise restart a stalled loop) only reschedules if `userStarted` is true. When the frame loop terminates naturally (e.g. GPU error), it clears `userStarted` and calls the `onAutoStop` callback passed to `HaloEngine.create()` so the UI can sync its state. `App` holds the authoritative `isRunning` boolean and passes it down to both `Canvas` (which calls `engine.start()`/`engine.stop()`) and `Sidebar` (which renders the Start/Stop button). Canvas mouse-drag and scroll-wheel interactions are gated on `isRunning`; the cursor is `grab`/`grabbing` only while running.

### State → GPU uniform encoding ([src/state/encodeParams.ts](src/state/encodeParams.ts))

`encodeSimParams` serializes one `CrystalPopulation` plus the view fields from `SimParams` into a 272-byte `ArrayBuffer` matching the `Params` struct in `raytrace.wgsl` (128 base bytes + `atmosphere_enabled` u32 with vec4 padding + `sun_spectrum` `array<vec4f, 8>` holding the 31-sample Hosek-Wilkie solar spectrum). It is called once per enabled population per frame by `HaloPass.encode()`. `encodeDisplayParams` writes the 32-byte `DisplayParams` layout for `display.wgsl` (resolution + brightness + show flags + camera fov in degrees, used to compensate halo exposure for FOV). `encodeAccParams` writes the 16-byte `AccParams` layout for `accumulate.wgsl` (only `resolution_x` is meaningful; the rest is uniform-alignment padding). `encodeSkyParams` writes the 256-byte `SkyParams` layout for `sky.wgsl` (resolution + projection + sun altitude + camera + turbidity + Hosek-Wilkie radiance scales + 9 config coefficients packed as `array<vec4f, 9>` + sun disk state: top/bottom XYZ, limb-darkening scaler, solar radius, raw elevation). `encodeGuidesParams` writes the 32-byte `GuidesParams` layout for `guides.wgsl`.

Invariants worth preserving:

- The `*_PARAMS_SIZE` constants and field order in `encodeParams.ts` must exactly match the WGSL struct layouts. Changing one without the other silently corrupts the simulation.
- Angles are stored in degrees in UI state and converted to radians at encode time.
- `simParams.camFov` holds the camera field of view in degrees (matching desktop). The raytrace, sky, and guides shaders consume a focal length, so `encodeParams.ts` calls `fovDegToFocalLength(fovDeg, projection)` (mirror of desktop `Camera::getFocalLength`) when serializing.
- `rng_seed` is incremented every frame so successive compute dispatches sample different rays.
- The pyramidal apex caps and prism face distances now live on each `CrystalPopulation` (populated with the same defaults as before) but are not yet exposed in the UI. Atmospheric turbidity and ground albedo are still hardcoded inside `SkyPass`.

### Parameter state ([src/state/](src/state/))

`ParamsProvider` holds two separate pieces of state and a monotonic `simVersion` counter:

- `simParams` — Inputs to the ray tracer (sun, camera, populations list). Changing any of these must discard the accumulated image, because rays from the old parameters would be physically inconsistent with new ones. **Exception:** `selectedPopIndex` is UI-only selection state and must not bump `simVersion` — the `setSim.selectedPopIndex(v)` setter is special-cased in the Proxy to skip the version bump.
- `displayParams` — Inputs to the display pass only (e.g. brightness). Can change without invalidating accumulated samples. **Exception:** toggling `showSky` swaps the halo's spectral weighting (Hosek-Wilkie sun spectrum vs. flat daylight estimate) and therefore resets halo accumulation.
- `simVersion` — Bumped whenever any `setSim.<field>(...)` setter (except `selectedPopIndex`), `setPopField`, `addPopulation`, `removePopulation`, or `reset` is called. `Canvas.tsx` calls `engine.setSimParams()` which resets the halo accumulation and, if view params changed, marks the sky pass dirty.

`setSim` and `setDisplay` are Proxy objects exposing one setter per field (`setSim.sunAlt(v)`, `setDisplay.brightness(v)`, …). The shape is typed as `{ [K in keyof T]: (value: T[K]) => void }`, so each field's setter accepts only its own value type. `setSim` setters bump `simVersion`; `setDisplay` setters do not.

Crystal populations are managed through three explicit context methods: `setPopField(idx, key, value)` mutates a single field of one population; `addPopulation(preset)` appends a preset-initialized population and selects it; `removePopulation(idx)` removes a population (no-op when only one remains). All three bump `simVersion`. Presets (Random, Plate, Column, Parry, Lowitz, Pyramid) are defined in [src/state/populations.ts](src/state/populations.ts) and ported from the desktop's `CrystalPopulation::create*()` factories.

When adding a new parameter, decide which of the two buckets it belongs to — that decision determines whether adjusting it resets the image.

The `Canvas` effect reads params through `simRef` / `displayRef` (refreshed by small `useEffect`s) rather than re-running the full WebGPU setup effect on every change; the setup effect's dep array is intentionally empty.

When adding a new pass, create a new pass class in `src/engine/`, a shader in `src/shaders/`, and wire it into `HaloEngine`'s frame loop. If the pass writes a buffer that the display shader reads, add the binding to `display.wgsl` and update `DisplayPass.createBindGroups()`.

### UI

Radix Themes (`@radix-ui/themes`) provides the component library and design tokens (CSS variables like `--gray-2`, `--color-panel-solid`). [SliderControl](src/components/SliderControl.tsx) is the shared labeled-slider wrapper used throughout the sidebar. [Sidebar](src/components/Sidebar.tsx) renders a Start/Stop button at the top (above the scroll area) that drives the `isRunning` state in `App`.

## Conventions

- WGSL shaders are imported as raw strings via Vite's `?raw` suffix.
- `@webgpu/types` provides the `GPU*` types; no runtime WebGPU polyfill is used.
- Formatting is owned by Prettier ([.prettierrc.json](.prettierrc.json)); ESLint handles correctness only. `eslint-config-prettier` is applied last in [eslint.config.js](eslint.config.js) to disable any stylistic rules that would conflict. Don't add formatting rules to ESLint — change the Prettier config instead.
