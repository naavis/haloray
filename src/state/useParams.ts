import { useContext } from "react";
import { ParamsContext, type ParamsContextValue } from "./ParamsContext";

export function useParams(): ParamsContextValue {
  const ctx = useContext(ParamsContext);
  if (!ctx) throw new Error("useParams must be used inside <ParamsProvider>");
  return ctx;
}
