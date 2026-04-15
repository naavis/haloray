import { Flex, Slider, Text } from "@radix-ui/themes"

type Props = {
  label: string
  value: number
  min: number
  max: number
  step: number
  onChange: (value: number) => void
  precision?: number
}

function SliderControl({ label, value, min, max, step, onChange, precision = 2 }: Props) {
  return (
    <Flex direction="column" gap="1">
      <Flex justify="between" align="center">
        <Text size="2">{label}</Text>
        <Text size="2" color="gray">
          {value.toFixed(precision)}
        </Text>
      </Flex>
      <Slider
        value={[value]}
        min={min}
        max={max}
        step={step}
        onValueChange={(values) => onChange(values[0])}
      />
    </Flex>
  )
}

export default SliderControl
