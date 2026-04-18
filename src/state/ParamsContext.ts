import { createContext } from "react";
import type { DisplayParams, SimParams } from "./params";

export type ParamsContextValue = {
  simParams: SimParams;
  displayParams: DisplayParams;
  simVersion: number;
  setSimParam: <K extends keyof SimParams>(key: K, value: SimParams[K]) => void;
  setDisplayParam: <K extends keyof DisplayParams>(key: K, value: DisplayParams[K]) => void;
  reset: () => void;
};

export const ParamsContext = createContext<ParamsContextValue | null>(null);
