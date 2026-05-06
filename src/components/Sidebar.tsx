import {
  Box,
  Button,
  Checkbox,
  DropdownMenu,
  Flex,
  Heading,
  IconButton,
  ScrollArea,
  Select,
  Separator,
  Text,
} from "@radix-ui/themes";
import SliderControl from "./SliderControl";
import { useParams } from "../state/useParams";
import type { Projection } from "../state/params";
import type { PresetKey } from "../state/populations";
import { getMaxFovDeg } from "../state/encodeParams";

const PRESET_LABELS: Record<PresetKey, string> = {
  random: "Random",
  plate: "Plate",
  column: "Column",
  parry: "Parry",
  lowitz: "Lowitz",
  pyramid: "Pyramid",
};

const PRESET_KEYS: PresetKey[] = ["random", "plate", "column", "parry", "lowitz", "pyramid"];

function Sidebar({
  isRunning,
  onStart,
  onStop,
}: {
  isRunning: boolean;
  onStart: () => void;
  onStop: () => void;
}) {
  const {
    simParams,
    displayParams,
    setSim,
    setDisplay,
    setCurrentPop,
    addPopulation,
    removePopulation,
    reset,
  } = useParams();
  const { populations, selectedPopIndex } = simParams;
  const pop = populations[selectedPopIndex];

  return (
    <Box
      style={{
        width: 320,
        height: "100vh",
        borderRight: "1px solid var(--gray-a5)",
        background: "var(--color-panel-solid)",
        display: "flex",
        flexDirection: "column",
      }}
    >
      <Box p="3" style={{ borderBottom: "1px solid var(--gray-a5)" }}>
        {isRunning ? (
          <Button variant="solid" color="red" onClick={onStop} style={{ width: "100%" }}>
            Stop
          </Button>
        ) : (
          <Button variant="solid" onClick={onStart} style={{ width: "100%" }}>
            Start
          </Button>
        )}
      </Box>
      <ScrollArea type="auto" scrollbars="vertical" style={{ flex: 1 }}>
        <Flex direction="column" gap="4" p="4">
          <Heading size="4">Parameters</Heading>

          <Flex direction="column" gap="3">
            <Heading size="2">Sun</Heading>
            <SliderControl
              label="Altitude (°)"
              value={simParams.sunAlt}
              min={-90}
              max={90}
              step={0.1}
              onChange={setSim.sunAlt}
            />
            <SliderControl
              label="Diameter (°)"
              value={simParams.sunDiam}
              min={0.1}
              max={5}
              step={0.1}
              onChange={setSim.sunDiam}
            />
          </Flex>

          <Separator size="4" />

          <Flex direction="column" gap="3">
            <Heading size="2">Crystal</Heading>

            {/* Population selector row */}
            <Flex gap="2" align="center">
              <Box flexGrow="1">
                <Select.Root
                  value={String(selectedPopIndex)}
                  onValueChange={(v) => setSim.selectedPopIndex(Number(v))}
                >
                  <Select.Trigger style={{ width: "100%" }} />
                  <Select.Content>
                    {populations.map((p, i) => (
                      <Select.Item key={i} value={String(i)}>
                        {p.name}
                      </Select.Item>
                    ))}
                  </Select.Content>
                </Select.Root>
              </Box>
              <DropdownMenu.Root>
                <DropdownMenu.Trigger>
                  <IconButton variant="soft" aria-label="Add population">
                    +
                  </IconButton>
                </DropdownMenu.Trigger>
                <DropdownMenu.Content>
                  {PRESET_KEYS.map((key) => (
                    <DropdownMenu.Item key={key} onSelect={() => addPopulation(key)}>
                      {PRESET_LABELS[key]}
                    </DropdownMenu.Item>
                  ))}
                </DropdownMenu.Content>
              </DropdownMenu.Root>
              <IconButton
                variant="soft"
                color="red"
                aria-label="Remove population"
                disabled={populations.length <= 1}
                onClick={() => removePopulation(selectedPopIndex)}
              >
                −
              </IconButton>
            </Flex>

            {/* Per-population controls */}
            <Flex gap="3" align="center">
              <Text as="label" size="2">
                <Flex gap="2" align="center">
                  <Checkbox
                    checked={pop.enabled}
                    onCheckedChange={(v) => setCurrentPop.enabled(v === true)}
                  />
                  Enabled
                </Flex>
              </Text>
            </Flex>
            <SliderControl
              label="Weight"
              value={pop.weight}
              min={0}
              max={20}
              step={0.1}
              onChange={setCurrentPop.weight}
            />

            <Separator size="4" />

            <SliderControl
              label="C/A Ratio"
              value={pop.caRatio}
              min={0.1}
              max={5}
              step={0.1}
              onChange={setCurrentPop.caRatio}
            />
            <SliderControl
              label="C/A Std Dev"
              value={pop.caRatioStd}
              min={0}
              max={2}
              step={0.1}
              onChange={setCurrentPop.caRatioStd}
            />
            <Separator size="4" />
            <Text as="label" size="2">
              <Flex gap="2" align="center">
                <Checkbox
                  checked={pop.tiltGaussian}
                  onCheckedChange={(v) => setCurrentPop.tiltGaussian(v === true)}
                />
                Gaussian Tilt
              </Flex>
            </Text>
            <SliderControl
              label="Tilt Average (°)"
              value={pop.tiltAvg}
              min={0}
              max={90}
              step={0.5}
              onChange={setCurrentPop.tiltAvg}
              disabled={!pop.tiltGaussian}
            />
            <SliderControl
              label="Tilt Std Dev (°)"
              value={pop.tiltStd}
              min={0}
              max={45}
              step={0.1}
              onChange={setCurrentPop.tiltStd}
              disabled={!pop.tiltGaussian}
            />
            <Separator size="4" />
            <Text as="label" size="2">
              <Flex gap="2" align="center">
                <Checkbox
                  checked={pop.rotGaussian}
                  onCheckedChange={(v) => setCurrentPop.rotGaussian(v === true)}
                />
                Gaussian Rotation
              </Flex>
            </Text>
            <SliderControl
              label="Rotation Average (°)"
              value={pop.rotAvg}
              min={0}
              max={180}
              step={1}
              onChange={setCurrentPop.rotAvg}
              disabled={!pop.rotGaussian}
            />
            <SliderControl
              label="Rotation Std Dev (°)"
              value={pop.rotStd}
              min={0}
              max={90}
              step={0.1}
              onChange={setCurrentPop.rotStd}
              disabled={!pop.rotGaussian}
            />
          </Flex>

          <Separator size="4" />

          <Flex direction="column" gap="3">
            <Heading size="2">Camera</Heading>
            <SliderControl
              label="Pitch (°)"
              value={simParams.camPitch}
              min={-90}
              max={90}
              step={0.5}
              onChange={setSim.camPitch}
            />
            <SliderControl
              label="Yaw (°)"
              value={simParams.camYaw}
              min={-180}
              max={180}
              step={0.5}
              onChange={setSim.camYaw}
            />
            <SliderControl
              label="Field of View (°)"
              value={simParams.camFov}
              min={1.5}
              max={getMaxFovDeg(simParams.projection)}
              step={0.5}
              onChange={setSim.camFov}
            />
            <Flex direction="column" gap="1">
              <Text size="2">Projection</Text>
              <Select.Root
                value={simParams.projection}
                onValueChange={(v) => {
                  const projection = v as Projection;
                  setSim.projection(projection);
                  const maxFov = getMaxFovDeg(projection);
                  if (simParams.camFov > maxFov) {
                    setSim.camFov(maxFov);
                  }
                }}
              >
                <Select.Trigger />
                <Select.Content>
                  <Select.Item value="0">Stereographic</Select.Item>
                  <Select.Item value="1">Rectilinear</Select.Item>
                  <Select.Item value="2">Equidistant</Select.Item>
                  <Select.Item value="3">Equal Area</Select.Item>
                  <Select.Item value="4">Orthographic</Select.Item>
                </Select.Content>
              </Select.Root>
            </Flex>
            <Text as="label" size="2">
              <Flex gap="2" align="center">
                <Checkbox
                  checked={simParams.hideSubHorizon}
                  onCheckedChange={(v) => setSim.hideSubHorizon(v === true)}
                />
                Hide Sub-Horizon Rays
              </Flex>
            </Text>
          </Flex>

          <Separator size="4" />

          <Flex direction="column" gap="3">
            <Heading size="2">Display</Heading>
            <SliderControl
              label="Brightness"
              value={displayParams.brightness}
              min={0.1}
              max={30}
              step={0.1}
              onChange={setDisplay.brightness}
            />
            <Text as="label" size="2">
              <Flex gap="2" align="center">
                <Checkbox
                  checked={displayParams.showSky}
                  onCheckedChange={(v) => setDisplay.showSky(v === true)}
                />
                Show Sky
              </Flex>
            </Text>
            <Text as="label" size="2">
              <Flex gap="2" align="center">
                <Checkbox
                  checked={displayParams.showGuides}
                  onCheckedChange={(v) => setDisplay.showGuides(v === true)}
                />
                Show Guides
              </Flex>
            </Text>
          </Flex>

          <Separator size="4" />

          <Button variant="soft" onClick={reset}>
            Reset Parameters
          </Button>
        </Flex>
      </ScrollArea>
    </Box>
  );
}

export default Sidebar;
