"use client";

import { useState } from "react";
import { useDomain } from "@/context/domain-context";
import { useSession } from "@/context/session-context";
import { urlEncode } from "@/lib/protocol";

const MAX_BODY = 512;

function clip(s: string, n: number) {
  return s.length > n ? s.slice(0, n) : s;
}

export default function ChatPage() {
  const s = useSession();
  const d = useDomain();
  const [peer, setPeer] = useState("");
  const [body, setBody] = useState("");

  return (
    <div className="mx-auto flex h-full max-w-3xl flex-col p-6 bg-white">
      <header className="mb-6">
        <h1 className="text-xl font-bold text-teams-text">Discussion</h1>
        <p className="mt-1 text-sm text-teams-muted">SND / MLS - memes trames que le client CLI.</p>
      </header>
      <div className="rounded-lg border border-teams-border bg-white p-4">
        <label className="block text-xs font-medium text-teams-muted">Destinataire (UUID)</label>
        <input
          className="mt-1 w-full rounded border border-teams-border bg-white px-3 py-2 font-mono text-sm outline-none focus:ring-2 focus:ring-teams-brand/50"
          value={peer}
          onChange={(e) => setPeer(e.target.value)}
        />
        <label className="mt-4 block text-xs font-medium text-teams-muted">Message</label>
        <textarea
          maxLength={MAX_BODY}
          className="mt-1 min-h-[120px] w-full rounded border border-teams-border bg-white px-3 py-2 text-sm outline-none focus:ring-2 focus:ring-teams-brand/50"
          value={body}
          onChange={(e) => setBody(e.target.value)}
        />
        <div className="mt-4 flex flex-wrap gap-2">
          <button
            type="button"
            disabled={!s.connected || !peer.trim() || !body.trim()}
            onClick={() => s.send(`SND to=${urlEncode(peer.trim())} body=${urlEncode(clip(body, MAX_BODY))}`)}
            className="rounded-md bg-teams-brand px-4 py-2 text-sm font-semibold text-white disabled:opacity-40"
          >
            Envoyer (SND)
          </button>
          <button
            type="button"
            disabled={!s.connected || !peer.trim()}
            onClick={() => s.send(`MLS with=${urlEncode(peer.trim())}`, { listMode: true, pending: "MLS" })}
            className="rounded-md border border-teams-border px-4 py-2 text-sm font-medium text-teams-text hover:bg-teams-hover disabled:opacity-40"
          >
            Historique (MLS)
          </button>
        </div>
      </div>
      <div className="mt-6 min-h-0 flex-1 overflow-y-auto rounded-lg border border-teams-border bg-[#FAFAFA] p-4">
        <h2 className="mb-2 text-xs font-semibold uppercase tracking-wide text-teams-muted">Derniers messages (réponse MLS)</h2>
        {d.dmMessages.length === 0 ? (
          <p className="text-sm text-teams-muted">Charge l’historique pour ce contact.</p>
        ) : (
          <ul className="space-y-3">
            {d.dmMessages.map((m, i) => (
              <li key={m.uuid ?? i} className="rounded-md border border-teams-border bg-white px-3 py-2 text-sm">
                <div className="text-[10px] text-teams-muted">
                  {m.timestamp ? new Date(Number(m.timestamp) * 1000).toLocaleString() : ""} · from {m.from?.slice(0, 8)}…
                </div>
                <p className="mt-1 whitespace-pre-wrap text-teams-text">{m.body}</p>
              </li>
            ))}
          </ul>
        )}
      </div>
    </div>
  );
}
