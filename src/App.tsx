import { useState } from "react";
import { Flex } from "@radix-ui/themes";
import Sidebar from "./components/Sidebar";
import Canvas from "./components/Canvas";

function App() {
  const [isRunning, setIsRunning] = useState(false);

  return (
    <Flex style={{ height: "100vh", width: "100vw" }}>
      <Sidebar
        isRunning={isRunning}
        onStart={() => setIsRunning(true)}
        onStop={() => setIsRunning(false)}
      />
      <Canvas isRunning={isRunning} onStop={() => setIsRunning(false)} />
    </Flex>
  );
}

export default App;
