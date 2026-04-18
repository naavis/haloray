# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

HaloRay is an atmospheric ice-halo (sun halo) simulator. The repository contains two codebases:

- **Root (this project)** — In-progress web port: React 19 + TypeScript + Vite + WebGPU (WGSL compute shaders). The `webgpu` branch is where active work happens.
- **[desktop/](desktop/)** — The original C++17 / Qt 5 / OpenGL 4.4 desktop application it is being ported from. Has its own [CLAUDE.md](desktop/CLAUDE.md); refer to it when cross-checking physics/behavior against the reference implementation.

## Commands

```bash
npm run dev      # Vite dev server with HMR
npm run build    # tsc -b && vite build (type-check then bundle)
npm run lint     # eslint .
npm run preview  # preview production build
```

Requires a WebGPU-capable browser. There is no test suite in the web port yet.

## Architecture

### Engine and rendering pipeline

[HaloEngine](src/engine/HaloEngine.ts) is the coordinator that owns the GPU device, canvas context, shared buffers, and the animation frame loop. It delegates work to separate pass classes:

- [HaloPass](src/engine/HaloPass.ts) — **raytrace** + **accumulate** compute passes (progressive). Owns pipelines, ray buffer, params buffer, `totalRays` counter. Writes into the shared accumulation buffer.
- [SkyPass](src/engine/SkyPass.ts) — **sky** compute pass. Writes per-pixel linear RGB into a sky buffer. Only re-runs when view params (camera + sun position) change.
- [GuidesPass](src/engine/GuidesPass.ts) — **guides** compute pass. Writes per-pixel linear RGBA into a guides buffer (4 floats/pixel). Only re-runs when view params change. Currently a placeholder that writes transparent black.

The engine owns the **display** render pass ([display.wgsl](src/shaders/display.wgsl)), which composites the outputs: it reads the accumulation buffer, sky buffer, and guides buffer, sums halo and sky in linear radiance space, applies Reinhard tone mapping and sRGB gamma, then alpha-blends the guides overlay on top.

The accumulation buffer and sky buffer are sized `width * height * 3 * 4` bytes; the guides buffer is `width * height * 4 * 4` bytes (RGBA). All are rebuilt on resize (via `ResizeObserver`). The display pass is gated by a `displayDirty` flag and only re-runs when something changed.

View params (`sunAlt`, `camPitch`, `camYaw`, `camFov`, `projection`) are tracked via `didViewChange()` in [params.ts](src/state/params.ts). Changes to view params mark the sky pass dirty; all simParams changes reset the halo accumulation.

### State → GPU uniform encoding ([src/state/encodeParams.ts](src/state/encodeParams.ts))

`encodeSimParams` serializes `SimParams` into a 128-byte `ArrayBuffer` matching the `Params` struct in `raytrace.wgsl`. `encodeDisplayParams` writes the 32-byte `DisplayParams`/`AccParams` layout shared by the accumulate and display shaders (5 used slots, padded to a 16-byte multiple for uniform alignment). `encodeSkyParams` writes the 8-byte `SkyParams` layout for `sky.wgsl`. `encodeGuidesParams` writes the 32-byte `GuidesParams` layout for `guides.wgsl`.

Invariants worth preserving:

- `PARAMS_SIZE` / `DISPLAY_PARAMS_SIZE` and the field order in `encodeParams.ts` must exactly match the WGSL struct layouts. Changing one without the other silently corrupts the simulation.
- Angles are stored in degrees in UI state and converted to radians at encode time.
- `rng_seed` is incremented every frame so successive compute dispatches sample different rays.
- Several slots (sub-horizon flag, pyramidal apex caps, prism face distances) are encoded with fixed placeholder values — they exist in the shader but aren't exposed in the UI yet.

### Parameter state ([src/state/](src/state/))

`ParamsProvider` holds two separate pieces of state and a monotonic `simVersion` counter:

- `simParams` — Inputs to the ray tracer (sun, crystal, camera). Changing any of these must discard the accumulated image, because rays from the old parameters would be physically inconsistent with new ones.
- `displayParams` — Inputs to the display pass only (e.g. brightness). Can change without invalidating accumulated samples.
- `simVersion` — Bumped whenever `setSimParam` or `reset` is called. `Canvas.tsx` calls `engine.setSimParams()` which resets the halo accumulation and, if view params changed, marks the sky pass dirty.

When adding a new parameter, decide which of the two buckets it belongs to — that decision determines whether adjusting it resets the image.

The `Canvas` effect reads params through `simRef` / `displayRef` (refreshed by small `useEffect`s) rather than re-running the full WebGPU setup effect on every change; the setup effect's dep array is intentionally empty.

When adding a new pass, create a new pass class in `src/engine/`, a shader in `src/shaders/`, and wire it into `HaloEngine`'s frame loop. If the pass writes a buffer that the display shader reads, add the binding to `display.wgsl` and update `createDisplayBindGroup()`.

### UI

Radix Themes (`@radix-ui/themes`) provides the component library and design tokens (CSS variables like `--gray-2`, `--color-panel-solid`). [SliderControl](src/components/SliderControl.tsx) is the shared labeled-slider wrapper used throughout the sidebar.

## Conventions

- WGSL shaders are imported as raw strings via Vite's `?raw` suffix.
- `@webgpu/types` provides the `GPU*` types; no runtime WebGPU polyfill is used.
