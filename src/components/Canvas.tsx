import { useEffect, useRef } from "react";
import { Box } from "@radix-ui/themes";
import { useParams } from "../state/useParams";
import { HaloEngine } from "../engine/HaloEngine";

function Canvas() {
  const containerRef = useRef<HTMLDivElement>(null);
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const engineRef = useRef<HaloEngine | null>(null);
  const { simParams, displayParams } = useParams();

  const simParamsRef = useRef(simParams);
  const displayParamsRef = useRef(displayParams);

  useEffect(() => {
    simParamsRef.current = simParams;
    engineRef.current?.setSimParams(simParams);
  }, [simParams]);

  useEffect(() => {
    displayParamsRef.current = displayParams;
    engineRef.current?.setDisplayParams(displayParams);
  }, [displayParams]);

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
        engine = await HaloEngine.create(canvas, simParamsRef.current, displayParamsRef.current);
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

  return (
    <Box
      ref={containerRef}
      style={{
        flex: 1,
        height: "100vh",
        background: "var(--gray-2)",
        overflow: "hidden",
      }}
    >
      <canvas ref={canvasRef} style={{ display: "block" }} />
    </Box>
  );
}

export default Canvas;
