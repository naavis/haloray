import { useCallback, useState } from "react";
import type { ReactNode } from "react";
import { DEFAULT_DISPLAY, DEFAULT_SIM, type DisplayParams, type SimParams } from "./params";
import { ParamsContext, type ParamsContextValue } from "./ParamsContext";

export function ParamsProvider({ children }: { children: ReactNode }) {
  const [simParams, setSim] = useState<SimParams>(DEFAULT_SIM);
  const [displayParams, setDisplay] = useState<DisplayParams>(DEFAULT_DISPLAY);
  // Monotonic counter incremented on every simParams change; the engine
  // watches this to know when to discard accumulated rays and restart.
  const [simVersion, setSimVersion] = useState(0);

  // useCallback memoizes the function so the same object reference is returned
  // on every render. Without it, each render would produce a new function object,
  // causing any child component that receives it as a prop to re-render needlessly.
  // The empty dep array [] means "create this function once and never recreate it",
  // which is safe because the only captured values are the React state setters
  // (setSim, setDisplay, setSimVersion), which React guarantees are stable.
  //
  // The generic <K extends keyof SimParams> links the types of key and value
  // together: K is inferred from whichever key you pass, and SimParams[K] then
  // resolves to the type of that specific field. For example, passing
  // key="sunAlt" sets K="sunAlt", so value must be number; passing
  // key="tiltGaussian" requires value to be boolean. This means TypeScript
  // catches mismatches like setSimParam("sunAlt", true) at compile time.
  //
  // The functional updater form setSim(prev => ...) is used instead of reading
  // simParams directly, so that if two updates are batched together each one
  // sees the latest state rather than a stale snapshot from when the function
  // was defined.
  const setSimParam = useCallback(<K extends keyof SimParams>(key: K, value: SimParams[K]) => {
    setSim((prev) => ({ ...prev, [key]: value }));
    // Bumping simVersion signals the engine to discard accumulated rays and
    // restart — sim params affect the physics, so old rays are now invalid.
    setSimVersion((v) => v + 1);
  }, []);

  // Same pattern as setSimParam, but for display-only params (e.g. brightness).
  // Does NOT bump simVersion because display changes don't invalidate accumulated rays.
  const setDisplayParam = useCallback(
    <K extends keyof DisplayParams>(key: K, value: DisplayParams[K]) => {
      setDisplay((prev) => ({ ...prev, [key]: value }));
    },
    [],
  );

  // Restores everything to defaults and bumps simVersion to force a full restart.
  const reset = useCallback(() => {
    setSim(DEFAULT_SIM);
    setDisplay(DEFAULT_DISPLAY);
    setSimVersion((v) => v + 1);
  }, []);

  const value: ParamsContextValue = {
    simParams,
    displayParams,
    simVersion,
    setSimParam,
    setDisplayParam,
    reset,
  };

  return <ParamsContext.Provider value={value}>{children}</ParamsContext.Provider>;
}
