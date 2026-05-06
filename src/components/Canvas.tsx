import { useEffect, useRef, useState } from "react";
import { Box } from "@radix-ui/themes";
import { useParams } from "../state/useParams";
import { HaloEngine } from "../engine/HaloEngine";
import { getMaxFovDeg } from "../state/encodeParams";

const DRAG_SENSITIVITY = 0.001;
const SCROLL_SENSITIVITY = 0.001;

function clamp(value: number, min: number, max: number) {
  return Math.min(max, Math.max(min, value));
}

function Canvas({
  isRunning,
  onStop,
  onProgressUpdate,
}: {
  isRunning: boolean;
  onStop: () => void;
  onProgressUpdate: (totalRays: number, canvasPixels: number) => void;
}) {
  const containerRef = useRef<HTMLDivElement>(null);
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const engineRef = useRef<HaloEngine | null>(null);
  const { simParams, displayParams, setSim } = useParams();

  const simParamsRef = useRef(simParams);
  const displayParamsRef = useRef(displayParams);
  const isRunningRef = useRef(isRunning);
  const onStopRef = useRef(onStop);
  const onProgressUpdateRef = useRef(onProgressUpdate);

  const isDragging = useRef(false);
  const dragStart = useRef<{ x: number; y: number; pitch: number; yaw: number } | null>(null);
  const [grabbing, setGrabbing] = useState(false);

  useEffect(() => {
    onStopRef.current = onStop;
  }, [onStop]);

  useEffect(() => {
    onProgressUpdateRef.current = onProgressUpdate;
  }, [onProgressUpdate]);

  useEffect(() => {
    if (isRunning) {
      engineRef.current?.start();
    } else {
      engineRef.current?.stop();
    }
    isRunningRef.current = isRunning;
  }, [isRunning]);

  useEffect(() => {
    simParamsRef.current = simParams;
    engineRef.current?.setSimParams(simParams);
  }, [simParams]);

  useEffect(() => {
    displayParamsRef.current = displayParams;
    engineRef.current?.setDisplayParams(displayParams);
  }, [displayParams]);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) {
      return;
    }

    const handleWheel = (e: WheelEvent) => {
      if (!isRunningRef.current) {
        return;
      }
      e.preventDefault();
      const sim = simParamsRef.current;
      const maxFov = getMaxFovDeg(sim.projection);
      const newFov = clamp(sim.camFov * Math.exp(e.deltaY * SCROLL_SENSITIVITY), 1.5, maxFov);
      setSim.camFov(newFov);
    };

    canvas.addEventListener("wheel", handleWheel, { passive: false });
    return () => canvas.removeEventListener("wheel", handleWheel);
  }, [setSim]);

  useEffect(() => {
    const container = containerRef.current;
    const canvas = canvasRef.current;
    if (!container || !canvas) {
      return;
    }

    let destroyed = false;

    (async () => {
      let engine: HaloEngine;
      try {
        engine = await HaloEngine.create(
          canvas,
          simParamsRef.current,
          displayParamsRef.current,
          () => onStopRef.current(),
          (r, p) => onProgressUpdateRef.current(r, p),
        );
      } catch (e) {
        console.error(e);
        return;
      }
      if (destroyed) {
        engine.destroy();
        return;
      }
      engineRef.current = engine;

      const rect = container.getBoundingClientRect();
      engine.resize(rect.width, rect.height);
    })();

    const observer = new ResizeObserver(() => {
      const rect = container.getBoundingClientRect();
      engineRef.current?.resize(rect.width, rect.height);
    });
    observer.observe(container);

    return () => {
      destroyed = true;
      observer.disconnect();
      engineRef.current?.destroy();
      engineRef.current = null;
    };
  }, []);

  const handleMouseDown = (e: React.MouseEvent) => {
    if (!isRunningRef.current) {
      return;
    }
    isDragging.current = true;
    dragStart.current = {
      x: e.clientX,
      y: e.clientY,
      pitch: simParamsRef.current.camPitch,
      yaw: simParamsRef.current.camYaw,
    };
    setGrabbing(true);
  };

  const handleMouseMove = (e: React.MouseEvent) => {
    if (!isDragging.current || !dragStart.current) {
      return;
    }
    const dx = e.clientX - dragStart.current.x;
    const dy = e.clientY - dragStart.current.y;
    const fov = simParamsRef.current.camFov;
    let newYaw = dragStart.current.yaw + dx * DRAG_SENSITIVITY * fov;
    if (newYaw > 180) {
      newYaw -= 360;
    } else if (newYaw < -180) {
      newYaw += 360;
    }
    setSim.camYaw(newYaw);
    setSim.camPitch(clamp(dragStart.current.pitch + dy * DRAG_SENSITIVITY * fov, -90, 90));
  };

  const handleMouseUp = () => {
    isDragging.current = false;
    setGrabbing(false);
  };

  return (
    <Box
      ref={containerRef}
      style={{
        flex: 1,
        height: "100vh",
        background: "var(--gray-2)",
        overflow: "hidden",
        cursor: isRunning ? (grabbing ? "grabbing" : "grab") : "default",
      }}
      onMouseDown={handleMouseDown}
      onMouseMove={handleMouseMove}
      onMouseUp={handleMouseUp}
      onMouseLeave={handleMouseUp}
    >
      <canvas ref={canvasRef} style={{ display: "block" }} />
    </Box>
  );
}

export default Canvas;
