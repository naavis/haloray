import {
  Box,
  Button,
  Checkbox,
  Flex,
  Heading,
  ScrollArea,
  Select,
  Separator,
  Text,
} from "@radix-ui/themes";
import SliderControl from "./SliderControl";
import { useParams } from "../state/useParams";
import type { Projection } from "../state/params";

function Sidebar() {
  const { simParams, displayParams, setSimParam, setDisplayParam, reset } = useParams();

  return (
    <Box
      style={{
        width: 320,
        height: "100vh",
        borderRight: "1px solid var(--gray-a5)",
        background: "var(--color-panel-solid)",
      }}
    >
      <ScrollArea type="auto" scrollbars="vertical" style={{ height: "100%" }}>
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
              onChange={(v) => setSimParam("sunAlt", v)}
            />
            <SliderControl
              label="Diameter (°)"
              value={simParams.sunDiam}
              min={0.1}
              max={5}
              step={0.1}
              onChange={(v) => setSimParam("sunDiam", v)}
            />
          </Flex>

          <Separator size="4" />

          <Flex direction="column" gap="3">
            <Heading size="2">Crystal</Heading>
            <SliderControl
              label="C/A Ratio"
              value={simParams.caRatio}
              min={0.1}
              max={5}
              step={0.01}
              onChange={(v) => setSimParam("caRatio", v)}
            />
            <SliderControl
              label="C/A Std Dev"
              value={simParams.caRatioStd}
              min={0}
              max={2}
              step={0.01}
              onChange={(v) => setSimParam("caRatioStd", v)}
            />
            <Separator size="4" />
            <Text as="label" size="2">
              <Flex gap="2" align="center">
                <Checkbox
                  checked={simParams.tiltGaussian}
                  onCheckedChange={(v) => setSimParam("tiltGaussian", v === true)}
                />
                Gaussian Tilt
              </Flex>
            </Text>
            <SliderControl
              label="Tilt Average (°)"
              value={simParams.tiltAvg}
              min={0}
              max={90}
              step={0.5}
              onChange={(v) => setSimParam("tiltAvg", v)}
              disabled={!simParams.tiltGaussian}
            />
            <SliderControl
              label="Tilt Std Dev (°)"
              value={simParams.tiltStd}
              min={0}
              max={45}
              step={0.5}
              onChange={(v) => setSimParam("tiltStd", v)}
              disabled={!simParams.tiltGaussian}
            />
            <Separator size="4" />
            <Text as="label" size="2">
              <Flex gap="2" align="center">
                <Checkbox
                  checked={simParams.rotGaussian}
                  onCheckedChange={(v) => setSimParam("rotGaussian", v === true)}
                />
                Gaussian Rotation
              </Flex>
            </Text>
            <SliderControl
              label="Rotation Average (°)"
              value={simParams.rotAvg}
              min={0}
              max={180}
              step={1}
              onChange={(v) => setSimParam("rotAvg", v)}
              precision={0}
              disabled={!simParams.rotGaussian}
            />
            <SliderControl
              label="Rotation Std Dev (°)"
              value={simParams.rotStd}
              min={0}
              max={90}
              step={1}
              onChange={(v) => setSimParam("rotStd", v)}
              precision={0}
              disabled={!simParams.rotGaussian}
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
              onChange={(v) => setSimParam("camPitch", v)}
            />
            <SliderControl
              label="Yaw (°)"
              value={simParams.camYaw}
              min={-180}
              max={180}
              step={0.5}
              onChange={(v) => setSimParam("camYaw", v)}
            />
            <SliderControl
              label="Focal Length"
              value={simParams.camFov}
              min={0.1}
              max={5}
              step={0.05}
              onChange={(v) => setSimParam("camFov", v)}
            />
            <Flex direction="column" gap="1">
              <Text size="2">Projection</Text>
              <Select.Root
                value={simParams.projection}
                onValueChange={(v) => setSimParam("projection", v as Projection)}
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
              onChange={(v) => setDisplayParam("brightness", v)}
            />
            <Text as="label" size="2">
              <Flex gap="2" align="center">
                <Checkbox
                  checked={displayParams.showSky}
                  onCheckedChange={(v) => setDisplayParam("showSky", v === true)}
                />
                Show Sky
              </Flex>
            </Text>
            <Text as="label" size="2">
              <Flex gap="2" align="center">
                <Checkbox
                  checked={displayParams.showGuides}
                  onCheckedChange={(v) => setDisplayParam("showGuides", v === true)}
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
