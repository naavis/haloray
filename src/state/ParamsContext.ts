import { createContext } from "react";
import type { DisplayParams, SimParams } from "./params";

export type ParamSetters<T> = { [K in keyof T]: (value: T[K]) => void };

export type ParamsContextValue = {
  simParams: SimParams;
  displayParams: DisplayParams;
  simVersion: number;
  setSim: ParamSetters<SimParams>;
  setDisplay: ParamSetters<DisplayParams>;
  reset: () => void;
};

export const ParamsContext = createContext<ParamsContextValue | null>(null);
