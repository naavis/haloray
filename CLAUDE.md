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

### Rendering pipeline ([src/components/Canvas.tsx](src/components/Canvas.tsx))

Three WebGPU passes per animation frame, driven by `requestAnimationFrame`:

1. **raytrace** (compute, [raytrace.wgsl](src/shaders/raytrace.wgsl)) - One invocation per ray. Generates a crystal, traces one ray through it, writes a `RayResult` (pixel coords + RGB) to `ray_buffer`. `RAYS_PER_STEP ≈ 500k` per frame, rounded up to a multiple of `WORKGROUP_SIZE = 64`. Once `totalRays` reaches `MAX_TOTAL_RAYS`, the raytrace and accumulate passes are skipped. The display pass is also gated by a `displayDirtyRef` flag: it re-runs only when something could have changed the output (fresh rays, displayParams edit, resize, or sim reset), so an idle-capped canvas does effectively no per-frame GPU work.
2. **accumulate** (compute, [accumulate.wgsl](src/shaders/accumulate.wgsl)) - Reads `ray_buffer`, atomically adds each ray's contribution (scaled to u32) into a per-pixel RGB accumulation buffer. Rays tagged `pixel_x == MISS (0xFFFFFFFF)` are skipped.
3. **display** (render, [display.wgsl](src/shaders/display.wgsl)) - Full-screen triangle that divides the accumulation buffer by `totalRays`, applies brightness, and writes to the canvas.

The accumulation buffer is sized `width * height * 3 * 4` bytes and is rebuilt on resize (via `ResizeObserver`).

### State → GPU uniform encoding ([src/state/encodeParams.ts](src/state/encodeParams.ts))

`encodeSimParams` serializes `SimParams` into a 128-byte `ArrayBuffer` matching the `Params` struct in `raytrace.wgsl`. `encodeDisplayParams` writes the 16-byte `DisplayParams`/`AccParams` layout shared by the accumulate and display shaders.

Invariants worth preserving:

- `PARAMS_SIZE` / `DISPLAY_PARAMS_SIZE` and the field order in `encodeParams.ts` must exactly match the WGSL struct layouts. Changing one without the other silently corrupts the simulation.
- Angles are stored in degrees in UI state and converted to radians at encode time.
- `rng_seed` is incremented every frame so successive compute dispatches sample different rays.
- Several slots (sub-horizon flag, pyramidal apex caps, prism face distances) are encoded with fixed placeholder values — they exist in the shader but aren't exposed in the UI yet.

### Parameter state ([src/state/](src/state/))

`ParamsProvider` holds two separate pieces of state and a monotonic `simVersion` counter:

- `simParams` — Inputs to the ray tracer (sun, crystal, camera). Changing any of these must discard the accumulated image, because rays from the old parameters would be physically inconsistent with new ones.
- `displayParams` — Inputs to the display pass only (e.g. brightness). Can change without invalidating accumulated samples.
- `simVersion` — Bumped whenever `setSimParam` or `reset` is called. `Canvas.tsx` watches this via `resetRequestedRef` and zeroes the accumulation buffer + resets `totalRays` on the next frame.

When adding a new parameter, decide which of the two buckets it belongs to — that decision determines whether adjusting it resets the image.

The `Canvas` effect reads params through `simRef` / `displayRef` (refreshed by small `useEffect`s) rather than re-running the full WebGPU setup effect on every change; the setup effect's dep array is intentionally empty.

### UI

Radix Themes (`@radix-ui/themes`) provides the component library and design tokens (CSS variables like `--gray-2`, `--color-panel-solid`). [SliderControl](src/components/SliderControl.tsx) is the shared labeled-slider wrapper used throughout the sidebar.

## Conventions

- WGSL shaders are imported as raw strings via Vite's `?raw` suffix.
- `@webgpu/types` provides the `GPU*` types; no runtime WebGPU polyfill is used.
