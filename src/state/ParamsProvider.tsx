import { useCallback, useMemo, useState } from "react";
import type { ReactNode } from "react";
import { DEFAULT_DISPLAY, DEFAULT_SIM, type DisplayParams, type SimParams } from "./params";
import {
  PRESETS,
  uniquePopulationName,
  type CrystalPopulation,
  type PresetKey,
} from "./populations";
import { ParamsContext, type ParamSetters, type ParamsContextValue } from "./ParamsContext";

export function ParamsProvider({ children }: { children: ReactNode }) {
  const [simParams, setSimState] = useState<SimParams>(DEFAULT_SIM);
  const [displayParams, setDisplayState] = useState<DisplayParams>(DEFAULT_DISPLAY);

  const setSim = useMemo<ParamSetters<SimParams>>(
    () =>
      new Proxy({} as ParamSetters<SimParams>, {
        get(_t, key: string) {
          return (value: unknown) => {
            setSimState((prev) => ({ ...prev, [key]: value }));
          };
        },
      }),
    [],
  );

  const setDisplay = useMemo<ParamSetters<DisplayParams>>(
    () =>
      new Proxy({} as ParamSetters<DisplayParams>, {
        get(_t, key: string) {
          return (value: unknown) => {
            setDisplayState((prev) => ({ ...prev, [key]: value }));
          };
        },
      }),
    [],
  );

  const setPopField = useCallback(
    <K extends keyof CrystalPopulation>(idx: number, key: K, value: CrystalPopulation[K]) => {
      setSimState((prev) => ({
        ...prev,
        populations: prev.populations.map((p, i) => (i === idx ? { ...p, [key]: value } : p)),
      }));
    },
    [],
  );

  const addPopulation = useCallback((preset: PresetKey) => {
    setSimState((prev) => {
      const freshParameters = PRESETS[preset]();
      freshParameters.name = uniquePopulationName(prev.populations, freshParameters.name);
      return { ...prev, populations: [...prev.populations, freshParameters] };
    });
  }, []);

  const removePopulation = useCallback((idx: number) => {
    setSimState((prev) => {
      if (prev.populations.length <= 1) {
        return prev;
      }
      return { ...prev, populations: prev.populations.filter((_, i) => i !== idx) };
    });
  }, []);

  const reset = useCallback(() => {
    setSimState(DEFAULT_SIM);
    setDisplayState(DEFAULT_DISPLAY);
  }, []);

  const value: ParamsContextValue = {
    simParams,
    displayParams,
    setSim,
    setDisplay,
    setPopField,
    addPopulation,
    removePopulation,
    reset,
  };

  return <ParamsContext.Provider value={value}>{children}</ParamsContext.Provider>;
}
