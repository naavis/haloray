import { useCallback, useMemo, useState } from "react";
import type { ReactNode } from "react";
import {
  DEFAULT_DISPLAY,
  DEFAULT_SIM,
  type DisplayParams,
  type SimParams,
} from "./params";
import { ParamsCtx, type ParamsContextValue } from "./paramsCtx";

export function ParamsProvider({ children }: { children: ReactNode }) {
  const [simParams, setSim] = useState<SimParams>(DEFAULT_SIM);
  const [displayParams, setDisplay] = useState<DisplayParams>(DEFAULT_DISPLAY);
  const [simVersion, setSimVersion] = useState(0);

  const setSimParam = useCallback(
    <K extends keyof SimParams>(key: K, value: SimParams[K]) => {
      setSim((prev) => ({ ...prev, [key]: value }));
      setSimVersion((v) => v + 1);
    },
    [],
  );

  const setDisplayParam = useCallback(
    <K extends keyof DisplayParams>(key: K, value: DisplayParams[K]) => {
      setDisplay((prev) => ({ ...prev, [key]: value }));
    },
    [],
  );

  const reset = useCallback(() => {
    setSim(DEFAULT_SIM);
    setDisplay(DEFAULT_DISPLAY);
    setSimVersion((v) => v + 1);
  }, []);

  const value = useMemo<ParamsContextValue>(
    () => ({ simParams, displayParams, simVersion, setSimParam, setDisplayParam, reset }),
    [simParams, displayParams, simVersion, setSimParam, setDisplayParam, reset],
  );

  return <ParamsCtx.Provider value={value}>{children}</ParamsCtx.Provider>;
}
