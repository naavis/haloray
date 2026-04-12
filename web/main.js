// main.js — WebGPU ice crystal halo simulator

const WORKGROUP_SIZE = 64;
const RAYS_PER_STEP = 500000;
const NUM_WORKGROUPS = Math.ceil(RAYS_PER_STEP / WORKGROUP_SIZE);
const ACTUAL_RAYS = NUM_WORKGROUPS * WORKGROUP_SIZE;
const RAY_RESULT_STRIDE = 20; // bytes per RayResult (2×u32 + 3×f32)
const PARAMS_SIZE = 128;
const DISPLAY_PARAMS_SIZE = 16;

function degToRad(d) { return d * Math.PI / 180; }

function formatNumber(n) {
    if (n >= 1e9) return (n / 1e9).toFixed(1) + 'B';
    if (n >= 1e6) return (n / 1e6).toFixed(1) + 'M';
    if (n >= 1e3) return (n / 1e3).toFixed(1) + 'K';
    return n.toString();
}

async function main() {
    if (!navigator.gpu) {
        document.body.innerHTML =
            '<div class="no-webgpu"><h1>WebGPU Not Available</h1>' +
            '<p>Please use a browser that supports WebGPU (Chrome 113+, Edge 113+).</p></div>';
        return;
    }

    const adapter = await navigator.gpu.requestAdapter();
    if (!adapter) {
        document.body.innerHTML =
            '<div class="no-webgpu"><h1>No GPU Adapter</h1>' +
            '<p>Could not find a suitable GPU adapter.</p></div>';
        return;
    }

    const device = await adapter.requestDevice();
    const canvas = document.getElementById('canvas');
    const ctx = canvas.getContext('webgpu');
    const format = navigator.gpu.getPreferredCanvasFormat();
    ctx.configure({ device, format, alphaMode: 'opaque' });

    // ── Load shaders ───────────────────────────────────────────────────

    const [raytraceCode, accumulateCode, displayCode] = await Promise.all([
        fetch('./shaders/raytrace.wgsl').then(r => r.text()),
        fetch('./shaders/accumulate.wgsl').then(r => r.text()),
        fetch('./shaders/display.wgsl').then(r => r.text()),
    ]);

    const raytraceModule   = device.createShaderModule({ label: 'raytrace',   code: raytraceCode });
    const accumulateModule = device.createShaderModule({ label: 'accumulate', code: accumulateCode });
    const displayModule    = device.createShaderModule({ label: 'display',    code: displayCode });

    // ── Static buffers ─────────────────────────────────────────────────

    const paramsBuffer = device.createBuffer({
        label: 'raytrace params', size: PARAMS_SIZE,
        usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
    });

    const rayBuffer = device.createBuffer({
        label: 'ray results', size: ACTUAL_RAYS * RAY_RESULT_STRIDE,
        usage: GPUBufferUsage.STORAGE,
    });

    const displayParamsBuffer = device.createBuffer({
        label: 'display params', size: DISPLAY_PARAMS_SIZE,
        usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
    });

    // ── Pipelines ──────────────────────────────────────────────────────

    const raytracePipeline = device.createComputePipeline({
        label: 'raytrace', layout: 'auto',
        compute: { module: raytraceModule, entryPoint: 'main' },
    });

    const accumulatePipeline = device.createComputePipeline({
        label: 'accumulate', layout: 'auto',
        compute: { module: accumulateModule, entryPoint: 'main' },
    });

    const displayPipeline = device.createRenderPipeline({
        label: 'display', layout: 'auto',
        vertex:   { module: displayModule, entryPoint: 'vs' },
        fragment: { module: displayModule, entryPoint: 'fs', targets: [{ format }] },
    });

    // ── Dynamic state ──────────────────────────────────────────────────

    let accBuffer = null;
    let canvasWidth = 0, canvasHeight = 0;
    let raytraceBindGroup = null;
    let accumulateBindGroup = null;
    let displayBindGroup = null;
    let totalRays = 0;
    let rngSeed = 0;
    let running = true;

    // Default simulation parameters
    const defaults = {
        sunAltitude: 15, sunDiameter: 0.5,
        caRatioAvg: 0.5, caRatioStd: 0,
        tiltDistribution: 0, tiltAvg: 0, tiltStd: 0,
        rotationDistribution: 0, rotationAvg: 0, rotationStd: 0,
        cameraPitch: 0, cameraYaw: 0, cameraFocalLength: 1,
        cameraProjection: 0, cameraHideSubHorizon: 0,
        upperApexAngle: 0, upperApexHeightAvg: 0, upperApexHeightStd: 0,
        lowerApexAngle: 0, lowerApexHeightAvg: 0, lowerApexHeightStd: 0,
        prismDistances: [1, 1, 1, 1, 1, 1],
    };
    const params = { ...defaults };

    // ── Buffer writers ─────────────────────────────────────────────────

    const paramsBuf = new ArrayBuffer(PARAMS_SIZE);
    const paramsU32 = new Uint32Array(paramsBuf);
    const paramsF32 = new Float32Array(paramsBuf);

    function writeParams() {
        paramsU32[0]  = rngSeed;
        paramsF32[1]  = degToRad(params.sunAltitude);
        paramsF32[2]  = degToRad(params.sunDiameter);
        paramsF32[3]  = params.caRatioAvg;
        paramsF32[4]  = params.caRatioStd;
        paramsU32[5]  = params.tiltDistribution;
        paramsF32[6]  = degToRad(params.tiltAvg);
        paramsF32[7]  = degToRad(params.tiltStd);
        paramsU32[8]  = params.rotationDistribution;
        paramsF32[9]  = degToRad(params.rotationAvg);
        paramsF32[10] = degToRad(params.rotationStd);
        paramsF32[11] = degToRad(params.cameraPitch);
        paramsF32[12] = degToRad(params.cameraYaw);
        paramsF32[13] = params.cameraFocalLength;
        paramsU32[14] = params.cameraProjection;
        paramsU32[15] = params.cameraHideSubHorizon;
        paramsU32[16] = canvasWidth;
        paramsU32[17] = canvasHeight;
        paramsF32[18] = degToRad(params.upperApexAngle);
        paramsF32[19] = params.upperApexHeightAvg;
        paramsF32[20] = params.upperApexHeightStd;
        paramsF32[21] = degToRad(params.lowerApexAngle);
        paramsF32[22] = params.lowerApexHeightAvg;
        paramsF32[23] = params.lowerApexHeightStd;
        // prism_distances: array<vec4f, 2> at byte offset 96 (index 24)
        paramsF32[24] = params.prismDistances[0];
        paramsF32[25] = params.prismDistances[1];
        paramsF32[26] = params.prismDistances[2];
        paramsF32[27] = params.prismDistances[3];
        paramsF32[28] = params.prismDistances[4];
        paramsF32[29] = params.prismDistances[5];
        paramsF32[30] = 0;
        paramsF32[31] = 0;
        device.queue.writeBuffer(paramsBuffer, 0, paramsBuf);
    }

    const dpBuf = new ArrayBuffer(DISPLAY_PARAMS_SIZE);
    const dpF32 = new Float32Array(dpBuf);

    function writeDisplayParams() {
        dpF32[0] = totalRays;
        dpF32[1] = canvasWidth;
        dpF32[2] = canvasHeight;
        dpF32[3] = 0;
        device.queue.writeBuffer(displayParamsBuffer, 0, dpBuf);
    }

    // ── Bind group creation ────────────────────────────────────────────

    function createBindGroups() {
        raytraceBindGroup = device.createBindGroup({
            label: 'raytrace', layout: raytracePipeline.getBindGroupLayout(0),
            entries: [
                { binding: 0, resource: { buffer: paramsBuffer } },
                { binding: 1, resource: { buffer: rayBuffer } },
            ],
        });
        accumulateBindGroup = device.createBindGroup({
            label: 'accumulate', layout: accumulatePipeline.getBindGroupLayout(0),
            entries: [
                { binding: 0, resource: { buffer: rayBuffer } },
                { binding: 1, resource: { buffer: accBuffer } },
                { binding: 2, resource: { buffer: displayParamsBuffer } },
            ],
        });
        displayBindGroup = device.createBindGroup({
            label: 'display', layout: displayPipeline.getBindGroupLayout(0),
            entries: [
                { binding: 0, resource: { buffer: accBuffer } },
                { binding: 1, resource: { buffer: displayParamsBuffer } },
            ],
        });
    }

    // ── Resize handling ────────────────────────────────────────────────

    function handleResize() {
        const rect = canvas.parentElement.getBoundingClientRect();
        const dpr = window.devicePixelRatio || 1;
        const w = Math.max(1, Math.min(Math.floor(rect.width * dpr), device.limits.maxTextureDimension2D));
        const h = Math.max(1, Math.min(Math.floor(rect.height * dpr), device.limits.maxTextureDimension2D));
        if (w === canvasWidth && h === canvasHeight) return;

        canvasWidth = w;
        canvasHeight = h;
        canvas.width = w;
        canvas.height = h;

        if (accBuffer) accBuffer.destroy();
        accBuffer = device.createBuffer({
            label: 'accumulation', size: w * h * 3 * 4,
            usage: GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_DST,
        });

        createBindGroups();
        totalRays = 0;
    }

    handleResize();
    new ResizeObserver(() => handleResize()).observe(canvas.parentElement);

    // ── Reset ──────────────────────────────────────────────────────────

    function resetSimulation() {
        totalRays = 0;
        if (accBuffer) {
            device.queue.writeBuffer(accBuffer, 0,
                new Uint8Array(canvasWidth * canvasHeight * 3 * 4));
        }
    }

    // ── FPS stats ──────────────────────────────────────────────────────

    let frameCount = 0;
    let lastFpsTime = performance.now();
    const statFps = document.getElementById('stat-fps');
    const statSamples = document.getElementById('stat-samples');

    // ── Animation loop ─────────────────────────────────────────────────

    function frame() {
        if (!running || !accBuffer) {
            requestAnimationFrame(frame);
            return;
        }

        rngSeed++;
        writeParams();
        totalRays += ACTUAL_RAYS;
        writeDisplayParams();

        const encoder = device.createCommandEncoder();

        // Pass 1: Raytrace
        const rp = encoder.beginComputePass();
        rp.setPipeline(raytracePipeline);
        rp.setBindGroup(0, raytraceBindGroup);
        rp.dispatchWorkgroups(NUM_WORKGROUPS);
        rp.end();

        // Pass 2: Accumulate
        const ap = encoder.beginComputePass();
        ap.setPipeline(accumulatePipeline);
        ap.setBindGroup(0, accumulateBindGroup);
        ap.dispatchWorkgroups(NUM_WORKGROUPS);
        ap.end();

        // Pass 3: Display
        const dp = encoder.beginRenderPass({
            colorAttachments: [{
                view: ctx.getCurrentTexture().createView(),
                loadOp: 'clear', storeOp: 'store',
                clearValue: { r: 0, g: 0, b: 0, a: 1 },
            }],
        });
        dp.setPipeline(displayPipeline);
        dp.setBindGroup(0, displayBindGroup);
        dp.draw(3);
        dp.end();

        device.queue.submit([encoder.finish()]);

        // Stats
        frameCount++;
        const now = performance.now();
        if (now - lastFpsTime >= 1000) {
            statFps.textContent = frameCount.toString();
            statSamples.textContent = formatNumber(totalRays);
            frameCount = 0;
            lastFpsTime = now;
        }

        requestAnimationFrame(frame);
    }

    // ── UI wiring ──────────────────────────────────────────────────────

    const sliderMap = [
        ['sunAlt',     'sunAltitude'],
        ['sunDiam',    'sunDiameter'],
        ['caRatio',    'caRatioAvg'],
        ['caRatioStd', 'caRatioStd'],
        ['tiltAvg',    'tiltAvg'],
        ['tiltStd',    'tiltStd'],
        ['rotAvg',     'rotationAvg'],
        ['rotStd',     'rotationStd'],
        ['camPitch',   'cameraPitch'],
        ['camYaw',     'cameraYaw'],
        ['camFov',     'cameraFocalLength'],
    ];

    for (const [id, key] of sliderMap) {
        const el = document.getElementById(id);
        const readout = document.getElementById('read-' + id);
        el.addEventListener('input', () => {
            params[key] = parseFloat(el.value);
            if (readout) readout.textContent = parseFloat(el.value).toFixed(2);
            resetSimulation();
        });
    }

    // Checkboxes → distribution type (0=uniform, 1=gaussian)
    const checkMap = [
        ['tiltGaussian', 'tiltDistribution'],
        ['rotGaussian',  'rotationDistribution'],
    ];
    for (const [id, key] of checkMap) {
        const el = document.getElementById(id);
        el.addEventListener('change', () => {
            params[key] = el.checked ? 1 : 0;
            resetSimulation();
        });
    }

    // Projection select
    document.getElementById('projection').addEventListener('change', (e) => {
        params.cameraProjection = parseInt(e.target.value);
        resetSimulation();
    });

    // Multiple scatter slider (wired but no effect — skipped in shader)
    const msEl = document.getElementById('multiScatter');
    const msRead = document.getElementById('read-multiScatter');
    if (msEl) {
        msEl.addEventListener('input', () => {
            if (msRead) msRead.textContent = parseFloat(msEl.value).toFixed(2);
        });
    }

    // Reset button
    document.getElementById('btn-reset').addEventListener('click', () => {
        Object.assign(params, { ...defaults });

        // Sync UI
        for (const [id, key] of sliderMap) {
            const el = document.getElementById(id);
            el.value = params[key];
            const readout = document.getElementById('read-' + id);
            if (readout) readout.textContent = params[key].toFixed(2);
        }
        for (const [id, key] of checkMap) {
            document.getElementById(id).checked = params[key] === 1;
        }
        document.getElementById('projection').value = params.cameraProjection;
        if (msEl) { msEl.value = 0; if (msRead) msRead.textContent = '0.00'; }

        resetSimulation();
    });

    // Start
    requestAnimationFrame(frame);
}

main();
