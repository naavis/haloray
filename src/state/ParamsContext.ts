import { createContext } from "react";
import type { DisplayParams, SimParams } from "./params";
import type { CrystalPopulation, PresetKey } from "./populations";

export type ParamSetters<T> = { [K in keyof T]: (value: T[K]) => void };

export type ParamsContextValue = {
  simParams: SimParams;
  displayParams: DisplayParams;
  simVersion: number;
  setSim: ParamSetters<SimParams>;
  setDisplay: ParamSetters<DisplayParams>;
  setCurrentPop: ParamSetters<CrystalPopulation>;
  addPopulation: (preset: PresetKey) => void;
  removePopulation: (idx: number) => void;
  reset: () => void;
};

export const ParamsContext = createContext<ParamsContextValue | null>(null);
