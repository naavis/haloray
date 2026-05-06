import { createContext } from "react";
import type { DisplayParams, SimParams } from "./params";
import type { CrystalPopulation, PresetKey } from "./populations";

export type ParamSetters<T> = { [K in keyof T]: (value: T[K]) => void };

export type ParamsContextValue = {
  simParams: SimParams;
  displayParams: DisplayParams;
  setSim: ParamSetters<SimParams>;
  setDisplay: ParamSetters<DisplayParams>;
  setPopField: <K extends keyof CrystalPopulation>(
    idx: number,
    key: K,
    value: CrystalPopulation[K],
  ) => void;
  addPopulation: (preset: PresetKey) => void;
  removePopulation: (idx: number) => void;
  reset: () => void;
};

export const ParamsContext = createContext<ParamsContextValue | null>(null);
