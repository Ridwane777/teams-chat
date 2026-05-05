"use client";

import {
  createContext,
  useCallback,
  useContext,
  useMemo,
  useRef,
  useState,
  type ReactNode,
} from "react";
import { parseFrame, type ParsedFrame } from "@/lib/protocol";
import { getClientId } from "@/lib/client-id";

export type LogEntry = { id: string; at: number; dir: "in" | "out" | "sys" | "err"; text: string };

export type SendMeta = {
  listMode: boolean;
  pending: string | null;
  tag: "LOG" | "OUT" | null;
};

type FrameHandler = (frame: ParsedFrame, raw: string) => void;

type SessionCtx = {
  connected: boolean;
  connecting: boolean;
  bridgePort: number;
  setBridgePort: (n: number) => void;
  mtHost: string;
  mtPort: number;
  setMtHost: (h: string) => void;
  setMtPort: (p: number) => void;
  myUuid: string | null;
  myName: string | null;
  log: LogEntry[];
  clearLog: () => void;
  connect: () => void;
  disconnect: () => void;
  send: (raw: string, opts?: { tag?: "LOG" | "OUT"; listMode?: boolean; pending?: string | null }) => void;
  getLastSendMeta: () => SendMeta;
  ackListComplete: () => void;
  subscribeFrames: (fn: FrameHandler) => () => void;
};

const Ctx = createContext<SessionCtx | null>(null);

function uid() {
  return `${Date.now()}-${Math.random().toString(36).slice(2, 9)}`;
}

function logActivityToMongo(direction: "in" | "out", line: string) {
  const clientId = getClientId();
  if (!clientId) return;
  void fetch("/api/activity", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ clientId, direction, line: line.slice(0, 2048) }),
  }).catch(() => {});
}

export function SessionProvider({ children }: { children: ReactNode }) {
  const [connected, setConnected] = useState(false);
  const [connecting, setConnecting] = useState(false);
  const [bridgePort, setBridgePort] = useState(4250);
  const [mtHost, setMtHost] = useState("127.0.0.1");
  const [mtPort, setMtPort] = useState(8080);
  const [myUuid, setMyUuid] = useState<string | null>(null);
  const [myName, setMyName] = useState<string | null>(null);
  const [log, setLog] = useState<LogEntry[]>([]);
  const wsRef = useRef<WebSocket | null>(null);
  const expectTag = useRef<"LOG" | "OUT" | null>(null);
  const lastSendMetaRef = useRef<SendMeta>({ listMode: false, pending: null, tag: null });
  const frameSubsRef = useRef<Set<FrameHandler>>(new Set());

  const emitFrame = useCallback((frame: ParsedFrame, raw: string) => {
    frameSubsRef.current.forEach((fn) => {
      try {
        fn(frame, raw);
      } catch {
        /* ignore subscriber errors */
      }
    });
  }, []);

  const subscribeFrames = useCallback((fn: FrameHandler) => {
    frameSubsRef.current.add(fn);
    return () => {
      frameSubsRef.current.delete(fn);
    };
  }, []);

  const getLastSendMeta = useCallback(() => ({ ...lastSendMetaRef.current }), []);

  const ackListComplete = useCallback(() => {
    lastSendMetaRef.current = { ...lastSendMetaRef.current, listMode: false, pending: null };
  }, []);

  const push = useCallback((dir: LogEntry["dir"], text: string) => {
    setLog((prev) => [...prev.slice(-500), { id: uid(), at: Date.now(), dir, text }]);
  }, []);

  const clearLog = useCallback(() => setLog([]), []);

  const disconnect = useCallback(() => {
    expectTag.current = null;
    wsRef.current?.close();
    wsRef.current = null;
    setConnected(false);
    setConnecting(false);
  }, []);

  const connect = useCallback(() => {
    disconnect();
    setConnecting(true);
    const host = window.location.hostname;
    const q = `?mtHost=${encodeURIComponent(mtHost)}&mtPort=${encodeURIComponent(String(mtPort))}`;
    const url = `ws://${host}:${bridgePort}/${q}`;
    const ws = new WebSocket(url);
    wsRef.current = ws;

    ws.onopen = () => {
      setConnected(true);
      setConnecting(false);
      push("sys", `WebSocket → pont (${url})`);
    };
    ws.onclose = () => {
      setConnected(false);
      setConnecting(false);
      push("sys", "WebSocket fermé");
    };
    ws.onerror = () => {
      push("err", "Erreur WebSocket (pont lancé ?)");
      setConnecting(false);
    };
    ws.onmessage = (ev) => {
      try {
        const msg = JSON.parse(String(ev.data)) as { type: string; data?: string; message?: string };
        if (msg.type === "line" && msg.data !== undefined) {
          const raw = msg.data;
          push("in", raw);
          logActivityToMongo("in", raw);
          const frame = parseFrame(raw);
          if (frame) {
            if (frame.opcode === "ERR") expectTag.current = null;
            if (expectTag.current === "LOG" && frame.opcode === "OK" && frame.fields.uuid && !frame.fields.count) {
              setMyUuid(frame.fields.uuid);
              if (frame.fields.name) setMyName(frame.fields.name);
              expectTag.current = null;
            }
            if (
              expectTag.current === "OUT" &&
              frame.opcode === "OK" &&
              Object.keys(frame.fields).length === 0
            ) {
              setMyUuid(null);
              setMyName(null);
              expectTag.current = null;
            }
            emitFrame(frame, raw);
          }
        } else if (msg.type === "system" && msg.message) push("sys", msg.message);
        else if (msg.type === "error" && msg.message) push("err", msg.message);
      } catch {
        push("in", String(ev.data));
      }
    };
  }, [bridgePort, disconnect, emitFrame, mtHost, mtPort, push]);

  const send = useCallback(
    (raw: string, opts?: { tag?: "LOG" | "OUT"; listMode?: boolean; pending?: string | null }) => {
      const ws = wsRef.current;
      if (!ws || ws.readyState !== WebSocket.OPEN) {
        push("err", "Non connecté au pont");
        return;
      }
      lastSendMetaRef.current = {
        listMode: Boolean(opts?.listMode),
        pending: opts?.pending ?? null,
        tag: opts?.tag ?? null,
      };
      let s = raw.trimEnd();
      if (!s.endsWith("\n")) s += "\n";
      expectTag.current = opts?.tag ?? null;
      ws.send(s);
      const oneLine = s.replace(/\n$/, "");
      push("out", oneLine);
      logActivityToMongo("out", oneLine);
    },
    [push]
  );

  const value = useMemo<SessionCtx>(
    () => ({
      connected,
      connecting,
      bridgePort,
      setBridgePort,
      mtHost,
      mtPort,
      setMtHost,
      setMtPort,
      myUuid,
      myName,
      log,
      clearLog,
      connect,
      disconnect,
      send,
      getLastSendMeta,
      ackListComplete,
      subscribeFrames,
    }),
    [
      connected,
      connecting,
      bridgePort,
      mtHost,
      mtPort,
      myUuid,
      myName,
      log,
      clearLog,
      connect,
      disconnect,
      send,
      getLastSendMeta,
      ackListComplete,
      subscribeFrames,
    ]
  );

  return <Ctx.Provider value={value}>{children}</Ctx.Provider>;
}

export function useSession() {
  const v = useContext(Ctx);
  if (!v) throw new Error("useSession hors SessionProvider");
  return v;
}
