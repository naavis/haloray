import { Flex } from "@radix-ui/themes";
import Sidebar from "./components/Sidebar";
import Canvas from "./components/Canvas";

function App() {
  return (
    <Flex style={{ height: "100vh", width: "100vw" }}>
      <Sidebar />
      <Canvas />
    </Flex>
  );
}

export default App;
