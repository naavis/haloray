import { useState } from "react";
import { Flex } from "@radix-ui/themes";
import Sidebar from "./components/Sidebar";
import Canvas from "./components/Canvas";

function App() {
  const [isRunning, setIsRunning] = useState(false);
  const [renderProgress, setRenderProgress] = useState({ totalRays: 0, pixels: 0 });

  return (
    <Flex style={{ height: "100vh", width: "100vw" }}>
      <Sidebar
        isRunning={isRunning}
        onStart={() => setIsRunning(true)}
        onStop={() => setIsRunning(false)}
        totalRays={renderProgress.totalRays}
        canvasPixels={renderProgress.pixels}
      />
      <Canvas
        isRunning={isRunning}
        onStop={() => setIsRunning(false)}
        onProgressUpdate={(r, p) => setRenderProgress({ totalRays: r, pixels: p })}
      />
    </Flex>
  );
}

export default App;
