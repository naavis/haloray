import { useContext } from "react";
import { ParamsCtx, type ParamsContextValue } from "./paramsCtx";

export function useParams(): ParamsContextValue {
  const ctx = useContext(ParamsCtx);
  if (!ctx) throw new Error("useParams must be used inside <ParamsProvider>");
  return ctx;
}
