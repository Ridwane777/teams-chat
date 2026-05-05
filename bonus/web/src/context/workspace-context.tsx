"use client";

import { createContext, useCallback, useContext, useMemo, useState, type ReactNode } from "react";
import { urlEncode } from "@/lib/protocol";
import { useSession } from "./session-context";

type Ws = {
  teamUuid: string;
  channelUuid: string;
  threadUuid: string;
  setTeamUuid: (v: string) => void;
  setChannelUuid: (v: string) => void;
  setThreadUuid: (v: string) => void;
  clearAll: () => void;
  sendUse: () => void;
  /** Met à jour le contexte local et envoie la trame USE (sans attendre un re-render). */
  applyContext: (team: string, channel?: string, thread?: string) => void;
};

const Ctx = createContext<Ws | null>(null);

export function WorkspaceProvider({ children }: { children: ReactNode }) {
  const { send } = useSession();
  const [teamUuid, setTeamUuid] = useState("");
  const [channelUuid, setChannelUuid] = useState("");
  const [threadUuid, setThreadUuid] = useState("");

  const clearAll = useCallback(() => {
    setTeamUuid("");
    setChannelUuid("");
    setThreadUuid("");
  }, []);

  const sendUse = useCallback(() => {
    let msg = "USE";
    if (teamUuid.trim()) msg += ` team=${urlEncode(teamUuid.trim())}`;
    if (channelUuid.trim()) msg += ` channel=${urlEncode(channelUuid.trim())}`;
    if (threadUuid.trim()) msg += ` thread=${urlEncode(threadUuid.trim())}`;
    send(msg, { pending: "/use" });
  }, [send, teamUuid, channelUuid, threadUuid]);

  const applyContext = useCallback(
    (team: string, channel = "", thread = "") => {
      setTeamUuid(team);
      setChannelUuid(channel);
      setThreadUuid(thread);
      let msg = "USE";
      if (team.trim()) msg += ` team=${urlEncode(team.trim())}`;
      if (channel.trim()) msg += ` channel=${urlEncode(channel.trim())}`;
      if (thread.trim()) msg += ` thread=${urlEncode(thread.trim())}`;
      send(msg, { pending: "/use" });
    },
    [send]
  );

  const value = useMemo(
    () => ({
      teamUuid,
      channelUuid,
      threadUuid,
      setTeamUuid,
      setChannelUuid,
      setThreadUuid,
      clearAll,
      sendUse,
      applyContext,
    }),
    [teamUuid, channelUuid, threadUuid, clearAll, sendUse, applyContext]
  );

  return <Ctx.Provider value={value}>{children}</Ctx.Provider>;
}

export function useWorkspace() {
  const v = useContext(Ctx);
  if (!v) throw new Error("useWorkspace hors WorkspaceProvider");
  return v;
}
