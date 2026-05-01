import { useCallback, useMemo, useState } from "react";
import type { ReactNode } from "react";
import { DEFAULT_DISPLAY, DEFAULT_SIM, type DisplayParams, type SimParams } from "./params";
import { ParamsContext, type ParamSetters, type ParamsContextValue } from "./ParamsContext";

export function ParamsProvider({ children }: { children: ReactNode }) {
  const [simParams, setSimState] = useState<SimParams>(DEFAULT_SIM);
  const [displayParams, setDisplayState] = useState<DisplayParams>(DEFAULT_DISPLAY);
  // Monotonic counter incremented on every simParams change; the engine
  // watches this to know when to discard accumulated rays and restart.
  const [simVersion, setSimVersion] = useState(0);

  // `setSim.<field>(v)` returns a per-field setter. Reading a property on the
  // Proxy yields a closure that does the same `setSimState(prev => ({...}))` +
  // `simVersion` bump the old `setSimParam("<field>", v)` did. The Proxy is
  // memoized once so its identity is stable across renders.
  const setSim = useMemo<ParamSetters<SimParams>>(
    () =>
      new Proxy({} as ParamSetters<SimParams>, {
        get(_t, key: string) {
          return (value: unknown) => {
            setSimState((prev) => ({ ...prev, [key]: value }));
            setSimVersion((v) => v + 1);
          };
        },
      }),
    [],
  );

  // Same shape as setSim but for display-only params. No simVersion bump —
  // display changes don't invalidate accumulated rays.
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

  // Restores everything to defaults and bumps simVersion to force a full restart.
  const reset = useCallback(() => {
    setSimState(DEFAULT_SIM);
    setDisplayState(DEFAULT_DISPLAY);
    setSimVersion((v) => v + 1);
  }, []);

  const value: ParamsContextValue = {
    simParams,
    displayParams,
    simVersion,
    setSim,
    setDisplay,
    reset,
  };

  return <ParamsContext.Provider value={value}>{children}</ParamsContext.Provider>;
}
