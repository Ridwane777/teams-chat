"use client";

import {
  createContext,
  useCallback,
  useContext,
  useEffect,
  useMemo,
  useRef,
  useState,
  type ReactNode,
} from "react";
import type { ParsedFrame } from "@/lib/protocol";
import { useSession } from "./session-context";

export type Row = Record<string, string>;

export type ActivityItem = { id: string; at: number; event: string; summary: string; fields: Row };

function uid() {
  return `${Date.now()}-${Math.random().toString(36).slice(2, 9)}`;
}

function isListTerminator(f: ParsedFrame): boolean {
  return f.opcode === "OK" && f.fields.count !== undefined && Object.keys(f.fields).length === 1;
}

function evtSummary(event: string, fields: Row): string {
  switch (event) {
    case "USER_ONLINE":
      return `${fields.name ?? "?"} en ligne`;
    case "USER_OFFLINE":
      return `${fields.name ?? "?"} hors ligne`;
    case "MSG_RECEIVED":
      return `Message de ${fields.from?.slice(0, 8) ?? "?"}…`;
    case "TEAM_CREATED":
      return `Équipe « ${fields.name ?? "?"} »`;
    case "CHANNEL_CREATED":
      return `Canal « ${fields.name ?? "?"} »`;
    case "THREAD_CREATED":
      return `Fil « ${fields.title ?? "?"} »`;
    case "COMMENT_CREATED":
      return `Réponse sur fil ${fields.thread?.slice(0, 8) ?? "?"}…`;
    default:
      return event;
  }
}

type DomainCtx = {
  allTeams: Row[];
  subscribedTeams: Row[];
  channels: Row[];
  threads: Row[];
  replies: Row[];
  users: Row[];
  dmMessages: Row[];
  infoDetail: Row | null;
  useEcho: Row | null;
  activity: ActivityItem[];
  lastError: string | null;
  clearActivity: () => void;
};

const Ctx = createContext<DomainCtx | null>(null);

export function DomainProvider({ children }: { children: ReactNode }) {
  const { subscribeFrames, getLastSendMeta, ackListComplete } = useSession();
  const [allTeams, setAllTeams] = useState<Row[]>([]);
  const [subscribedTeams, setSubscribedTeams] = useState<Row[]>([]);
  const [channels, setChannels] = useState<Row[]>([]);
  const [threads, setThreads] = useState<Row[]>([]);
  const [replies, setReplies] = useState<Row[]>([]);
  const [users, setUsers] = useState<Row[]>([]);
  const [dmMessages, setDmMessages] = useState<Row[]>([]);
  const [infoDetail, setInfoDetail] = useState<Row | null>(null);
  const [useEcho, setUseEcho] = useState<Row | null>(null);
  const [activity, setActivity] = useState<ActivityItem[]>([]);
  const [lastError, setLastError] = useState<string | null>(null);
  const listBufRef = useRef<Row[]>([]);

  const pushActivity = useCallback((event: string, fields: Row) => {
    setActivity((prev) =>
      [
        ...prev.slice(-199),
        {
          id: uid(),
          at: Date.now(),
          event,
          summary: evtSummary(event, fields),
          fields,
        },
      ]
    );
  }, []);

  const clearActivity = useCallback(() => setActivity([]), []);

  const finalizeList = useCallback(
    (rows: Row[], count: number, pending: string | null) => {
      void count;
      const p = pending ?? "";
      if (p === "ULS" || p === "TSD_MEMBERS") {
        setUsers(rows);
        return;
      }
      if (p === "MLS") {
        setDmMessages(rows);
        return;
      }
      if (p === "TSD_SUBS") {
        setSubscribedTeams(rows);
        return;
      }
      if (p === "/list_teams") {
        setAllTeams(rows);
        return;
      }
      if (p === "/list_channels") {
        setChannels(rows);
        return;
      }
      if (p === "/list_threads") {
        setThreads(rows);
        return;
      }
      if (p === "/list_replies") {
        setReplies(rows);
        return;
      }
      if (rows.length && rows[0].from && rows[0].to) {
        setDmMessages(rows);
        return;
      }
      if (rows.length && rows[0].title && rows[0].channel) {
        setThreads(rows);
        return;
      }
      if (rows.length && rows[0].team && rows[0].uuid && !rows[0].title) {
        setChannels(rows);
        return;
      }
      if (rows.length && rows[0].logged_in !== undefined) {
        setUsers(rows);
        return;
      }
      if (rows.length && rows[0].description !== undefined && !rows[0].team) {
        setAllTeams(rows);
        return;
      }
      if (rows.length && rows[0].author && rows[0].thread) {
        setReplies(rows);
        return;
      }
    },
    []
  );

  const handleSingleton = useCallback((frame: ParsedFrame, pending: string | null) => {
    if (frame.opcode !== "OK") return;
    const f = frame.fields;
    if (pending === "/use" && (f.team !== undefined || f.channel !== undefined || f.thread !== undefined)) {
      setUseEcho(f);
      return;
    }
    if (pending?.startsWith("/info_")) {
      setInfoDetail(f);
      return;
    }
    if (pending === "UGT" && f.uuid && f.name) {
      setInfoDetail(f);
    }
  }, []);

  useEffect(() => {
    return subscribeFrames((frame, raw) => {
      const meta = getLastSendMeta();

      if (frame.opcode === "ERR") {
        listBufRef.current = [];
        ackListComplete();
        const reason = frame.fields.reason ?? "";
        const code = frame.fields.code ?? "";
        setLastError(`ERR ${code} ${reason}`.trim() || raw);
        return;
      }

      if (frame.opcode === "EVT") {
        const ev = frame.fields.event ?? "";
        pushActivity(ev, frame.fields);
        setLastError(null);
        return;
      }

      if (frame.opcode === "OK") {
        setLastError(null);
        if (isListTerminator(frame)) {
          const c = Number(frame.fields.count);
          finalizeList(listBufRef.current, Number.isFinite(c) ? c : 0, meta.pending);
          listBufRef.current = [];
          ackListComplete();
          return;
        }
        if (meta.listMode) {
          listBufRef.current.push({ ...frame.fields });
          return;
        }
        handleSingleton(frame, meta.pending);
      }
    });
  }, [subscribeFrames, getLastSendMeta, ackListComplete, finalizeList, handleSingleton, pushActivity]);

  const value = useMemo<DomainCtx>(
    () => ({
      allTeams,
      subscribedTeams,
      channels,
      threads,
      replies,
      users,
      dmMessages,
      infoDetail,
      useEcho,
      activity,
      lastError,
      clearActivity,
    }),
    [
      allTeams,
      subscribedTeams,
      channels,
      threads,
      replies,
      users,
      dmMessages,
      infoDetail,
      useEcho,
      activity,
      lastError,
      clearActivity,
    ]
  );

  return <Ctx.Provider value={value}>{children}</Ctx.Provider>;
}

export function useDomain() {
  const v = useContext(Ctx);
  if (!v) throw new Error("useDomain hors DomainProvider");
  return v;
}
