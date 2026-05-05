"use client";

import { useMemo, useState } from "react";
import { useDomain } from "@/context/domain-context";
import { useSession } from "@/context/session-context";
import { useWorkspace } from "@/context/workspace-context";
import { urlEncode } from "@/lib/protocol";

const MAX_NAME = 32;
const MAX_BODY = 512;

function clip(v: string, n: number) {
  return v.length > n ? v.slice(0, n) : v;
}

function toTime(ts?: string) {
  if (!ts) return "30 min";
  const d = new Date(Number(ts) * 1000);
  if (Number.isNaN(d.getTime())) return "30 min";
  return d.toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" });
}

export default function TeamWorkspace() {
  const s = useSession();
  const d = useDomain();
  const w = useWorkspace();
  const [search, setSearch] = useState("");
  const [draft, setDraft] = useState("");

  const teams = useMemo(() => d.allTeams.filter((t) => (t.name ?? "").toLowerCase().includes(search.toLowerCase())), [d.allTeams, search]);
  const messages = useMemo(() => {
    const rows = d.replies.length > 0 ? d.replies : d.threads;
    return rows.slice(-8);
  }, [d.replies, d.threads]);

  const sendCreate = () => {
    const body = clip(draft.trim(), MAX_BODY);
    if (!body) return;
    if (!w.teamUuid) s.send(`TCR name=${urlEncode(clip(body, MAX_NAME))} description=${urlEncode("New team from web")}`);
    else if (!w.channelUuid) s.send(`CCR name=${urlEncode(clip(body, MAX_NAME))} description=${urlEncode("New channel from web")}`);
    else if (!w.threadUuid) s.send(`HCR title=${urlEncode(clip(body, MAX_NAME))} body=${urlEncode(body)}`);
    else s.send(`OCR body=${urlEncode(body)}`);
    setDraft("");
  };

  return (
    <div className="flex h-full min-h-0 bg-white">
      <section className="flex w-[320px] shrink-0 flex-col border-r border-[#edebe9]">
        <div className="flex h-[58px] items-center justify-between border-b border-[#edebe9] px-4">
          <div className="text-[18px] font-semibold text-[#252423]">Chat</div>
          <button type="button" className="rounded-md px-2 py-1 text-sm text-[#6264a7] hover:bg-[#f3f2f1]">
            New
          </button>
        </div>

        <div className="border-b border-[#edebe9] p-3">
          <div className="flex h-9 w-full items-center rounded-md border border-[#d1d1d1] bg-white px-3 text-[13px] text-[#605e5c]">
            🔎
            <input
              value={search}
              onChange={(e) => setSearch(e.target.value)}
              placeholder="Search in chats"
              className="ml-2 w-full bg-transparent text-[#323130] outline-none placeholder:text-[#8a8886]"
            />
          </div>
        </div>

        <div className="border-b border-[#edebe9] px-4 py-2">
          <div className="mb-1.5 text-[11px] font-semibold uppercase tracking-wide text-[#8a8886]">Favorites</div>
          <div className="flex gap-2">
            <span className="flex h-7 w-7 items-center justify-center rounded-full bg-[#ffd8b1] text-[10px] font-semibold text-[#8a5a00]">AL</span>
            <span className="flex h-7 w-7 items-center justify-center rounded-full bg-[#c7e0f4] text-[10px] font-semibold text-[#005a9e]">MT</span>
            <span className="flex h-7 w-7 items-center justify-center rounded-full bg-[#d9d2f0] text-[10px] font-semibold text-[#5c2d91]">JS</span>
            <span className="flex h-7 w-7 items-center justify-center rounded-full bg-[#cce4d4] text-[10px] font-semibold text-[#0b6a0b]">DEV</span>
          </div>
        </div>

        <div className="min-h-0 flex-1 overflow-y-auto p-2.5">
          <div className="mb-2 text-[11px] font-semibold uppercase tracking-wide text-[#8a8886]">Recent</div>
          {teams.length === 0 ? (
            <div className="rounded-md bg-[#f3f2f1] p-2.5 text-[12px] text-[#605e5c]">Aucun team. Utilise LST.</div>
          ) : (
            teams.map((t, i) => (
              <button
                key={t.uuid ?? `${i}`}
                type="button"
                onClick={() => {
                  w.applyContext(t.uuid ?? "", "", "");
                  s.send("LST", { listMode: true, pending: "/list_channels" });
                }}
                className={`mb-1.5 flex w-full items-center gap-2.5 rounded-md px-2.5 py-2.5 text-left transition ${
                  w.teamUuid === t.uuid ? "bg-[#e8ebfa] ring-1 ring-[#c7d2f0]" : "hover:bg-[#f3f2f1]"
                }`}
              >
                <span className={`flex h-8 w-8 items-center justify-center rounded-full text-[11px] font-semibold ${w.teamUuid === t.uuid ? "bg-[#6264a7] text-white" : "bg-[#deecf9] text-[#005a9e]"}`}>
                  {(t.name?.[0] ?? "T").toUpperCase()}
                </span>
                <span className="min-w-0 flex-1">
                  <span className="block truncate text-[13px] font-semibold text-[#252423]">{t.name || "Team"}</span>
                  <span className="block truncate text-[11px] text-[#605e5c]">
                    {t.description || "Celestial bodies that capture"}
                  </span>
                </span>
                <span className="text-[10px] text-[#8a8886]">{i + 1}m</span>
              </button>
            ))
          )}
        </div>
      </section>

      <section className="flex min-w-0 flex-1 flex-col border-r border-[#edebe9] bg-white">
        <div className="flex h-[58px] items-center justify-between border-b border-[#edebe9] px-5">
          <div>
            <div className="text-[16px] font-semibold text-[#252423]">{w.teamUuid ? "Selected team room" : "General room"}</div>
            <div className="text-[12px] text-[#605e5c]">{s.connected ? "Connected to bridge" : "Disconnected"}</div>
          </div>
          <div className="flex items-center gap-2 text-[#605e5c]">
            <button type="button" className="rounded-md p-1.5 hover:bg-[#f3f2f1]">📞</button>
            <button type="button" className="rounded-md p-1.5 hover:bg-[#f3f2f1]">🎥</button>
            <button type="button" className="rounded-md p-1.5 hover:bg-[#f3f2f1]">⋯</button>
          </div>
        </div>

        <div className="min-h-0 flex-1 overflow-y-auto bg-[#faf9f8] px-6 py-4">
          {messages.length === 0 ? (
            <div className="text-[13px] text-[#605e5c]">Aucun message pour l&apos;instant.</div>
          ) : (
            <>
              {messages.map((m, i) => {
                const me = i % 2 === 0;
                const text = m.body ?? m.title ?? "Message";
                return (
                  <div key={`${m.uuid ?? i}`} className={`mb-3 flex ${me ? "justify-start" : "justify-end"}`}>
                    <div className={`max-w-[70%] rounded-2xl px-3.5 py-2.5 text-[13px] ${
                      me ? "bg-[#e1dfdd] text-[#252423]" : "bg-[#6264a7] text-white"
                    }`}>
                      <div className={`mb-1 flex items-center justify-between gap-3 text-[10px] ${me ? "text-[#605e5c]" : "text-white/80"}`}>
                        <span className="font-semibold">{me ? "Team member" : "You"}</span>
                        <span>{toTime(m.timestamp)}</span>
                      </div>
                      <p className="leading-5">{text}</p>
                    </div>
                  </div>
                );
              })}

              <div className="my-5 flex items-center gap-2">
                <div className="h-px flex-1 bg-[#e1dfdd]" />
                <span className="text-[10px] text-[#8a8886]">Today</span>
                <div className="h-px flex-1 bg-[#e1dfdd]" />
              </div>
            </>
          )}
        </div>

        <div className="border-t border-[#edebe9] bg-white px-4 py-3">
          <div className="flex items-center gap-2 rounded-xl border border-[#d1d1d1] bg-white px-3 py-2">
            <button type="button" className="text-[#605e5c]">＋</button>
            <textarea
              value={draft}
              onChange={(e) => setDraft(e.target.value)}
              maxLength={MAX_BODY}
              placeholder="Write a new message..."
              className="h-6 min-h-[24px] flex-1 resize-none border-0 bg-transparent pt-1 text-[13px] leading-5 text-[#323130] outline-none"
            />
            <button type="button" className="text-[#605e5c]">☺</button>
            <button type="button" className="text-[#605e5c]">📎</button>
            <button
              type="button"
              disabled={!s.connected || !draft.trim()}
              onClick={sendCreate}
              className="rounded-md bg-[#6264a7] px-3 py-1.5 text-[12px] font-semibold text-white hover:bg-[#585a99] disabled:opacity-40"
            >
              Send
            </button>
          </div>
        </div>
      </section>

      <aside className="hidden w-[280px] shrink-0 flex-col bg-white xl:flex">
        <div className="border-b border-[#edebe9] px-5 py-5 text-center">
          <div className="mx-auto mb-2 flex h-16 w-16 items-center justify-center rounded-full bg-[#deecf9] text-lg font-bold text-[#005a9e]">
            {s.myName?.[0] ?? "J"}
          </div>
          <div className="text-[16px] font-semibold text-[#252423]">{s.myName || "John Doe"}</div>
          <div className="mt-0.5 text-[12px] text-[#605e5c]">{s.myUuid || "john.doe@mail.com"}</div>
        </div>

        <div className="border-b border-[#edebe9] px-5 py-3">
          <div className="mb-2 flex items-center justify-center gap-3 text-[#605e5c]">
            <button type="button" className="rounded-md p-1.5 hover:bg-[#f3f2f1]">📞</button>
            <button type="button" className="rounded-md p-1.5 hover:bg-[#f3f2f1]">📹</button>
            <button type="button" className="rounded-md p-1.5 hover:bg-[#f3f2f1]">✉️</button>
          </div>
        </div>

        <div className="p-5">
          <div className="mb-2 text-[12px] font-semibold uppercase tracking-wide text-[#8a8886]">Shared Files</div>
          <ul className="space-y-2 text-[12px] text-[#323130]">
            <li className="rounded-md bg-[#f3f2f1] px-2.5 py-2">📁 Product-roadmap.pdf</li>
            <li className="rounded-md bg-[#f3f2f1] px-2.5 py-2">📄 Sprint-backlog.docx</li>
            <li className="rounded-md bg-[#f3f2f1] px-2.5 py-2">🖼️ Release-banner.png</li>
          </ul>
        </div>
      </aside>
    </div>
  );
}
