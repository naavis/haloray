import { useState } from "react"
import {
  Box,
  Button,
  Flex,
  Heading,
  ScrollArea,
  Select,
  Separator,
  Text,
  Checkbox,
} from "@radix-ui/themes"
import SliderControl from "./SliderControl"

const DEFAULTS = {
  sunAlt: 15,
  sunDiam: 0.5,
  caRatio: 0.5,
  caRatioStd: 0,
  tiltGaussian: false,
  tiltAvg: 0,
  tiltStd: 0,
  rotGaussian: false,
  rotAvg: 0,
  rotStd: 0,
  camPitch: 0,
  camYaw: 0,
  camFov: 1,
  projection: "0",
  brightness: 1,
}

function Sidebar() {
  const [params, setParams] = useState(DEFAULTS)

  const update = <K extends keyof typeof DEFAULTS>(key: K, value: (typeof DEFAULTS)[K]) => {
    setParams((prev) => ({ ...prev, [key]: value }))
  }

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
              value={params.sunAlt}
              min={-10}
              max={90}
              step={0.5}
              onChange={(v) => update("sunAlt", v)}
            />
            <SliderControl
              label="Diameter (°)"
              value={params.sunDiam}
              min={0.1}
              max={5}
              step={0.1}
              onChange={(v) => update("sunDiam", v)}
            />
          </Flex>

          <Separator size="4" />

          <Flex direction="column" gap="3">
            <Heading size="2">Crystal</Heading>
            <SliderControl
              label="C/A Ratio"
              value={params.caRatio}
              min={0.1}
              max={5}
              step={0.01}
              onChange={(v) => update("caRatio", v)}
            />
            <SliderControl
              label="C/A Std Dev"
              value={params.caRatioStd}
              min={0}
              max={2}
              step={0.01}
              onChange={(v) => update("caRatioStd", v)}
            />
            <Text as="label" size="2">
              <Flex gap="2" align="center">
                <Checkbox
                  checked={params.tiltGaussian}
                  onCheckedChange={(v) => update("tiltGaussian", v === true)}
                />
                Gaussian Tilt
              </Flex>
            </Text>
            <SliderControl
              label="Tilt Average (°)"
              value={params.tiltAvg}
              min={0}
              max={90}
              step={0.5}
              onChange={(v) => update("tiltAvg", v)}
            />
            <SliderControl
              label="Tilt Std Dev (°)"
              value={params.tiltStd}
              min={0}
              max={45}
              step={0.5}
              onChange={(v) => update("tiltStd", v)}
            />
            <Text as="label" size="2">
              <Flex gap="2" align="center">
                <Checkbox
                  checked={params.rotGaussian}
                  onCheckedChange={(v) => update("rotGaussian", v === true)}
                />
                Gaussian Rotation
              </Flex>
            </Text>
            <SliderControl
              label="Rotation Average (°)"
              value={params.rotAvg}
              min={0}
              max={180}
              step={1}
              onChange={(v) => update("rotAvg", v)}
              precision={0}
            />
            <SliderControl
              label="Rotation Std Dev (°)"
              value={params.rotStd}
              min={0}
              max={90}
              step={1}
              onChange={(v) => update("rotStd", v)}
              precision={0}
            />
          </Flex>

          <Separator size="4" />

          <Flex direction="column" gap="3">
            <Heading size="2">Camera</Heading>
            <SliderControl
              label="Pitch (°)"
              value={params.camPitch}
              min={-90}
              max={90}
              step={0.5}
              onChange={(v) => update("camPitch", v)}
            />
            <SliderControl
              label="Yaw (°)"
              value={params.camYaw}
              min={-180}
              max={180}
              step={0.5}
              onChange={(v) => update("camYaw", v)}
            />
            <SliderControl
              label="Focal Length"
              value={params.camFov}
              min={0.1}
              max={5}
              step={0.05}
              onChange={(v) => update("camFov", v)}
            />
            <Flex direction="column" gap="1">
              <Text size="2">Projection</Text>
              <Select.Root
                value={params.projection}
                onValueChange={(v) => update("projection", v)}
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
              value={params.brightness}
              min={0.1}
              max={10}
              step={0.1}
              onChange={(v) => update("brightness", v)}
            />
          </Flex>

          <Separator size="4" />

          <Button variant="soft" onClick={() => setParams(DEFAULTS)}>
            Reset Parameters
          </Button>
        </Flex>
      </ScrollArea>
    </Box>
  )
}

export default Sidebar
