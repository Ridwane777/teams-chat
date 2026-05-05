"use client";

import { useState } from "react";
import { useDomain } from "@/context/domain-context";
import { useSession } from "@/context/session-context";
import { urlEncode } from "@/lib/protocol";

export default function PeoplePage() {
  const s = useSession();
  const d = useDomain();
  const [uuid, setUuid] = useState("");

  return (
    <div className="mx-auto flex h-full max-w-3xl flex-col gap-6 p-6 bg-white">
      <header>
        <h1 className="text-xl font-bold text-teams-text">Personnes</h1>
        <p className="mt-1 text-sm text-teams-muted">ULS / UGT - aligne sur le client CLI.</p>
      </header>
      <div className="flex flex-wrap gap-2">
        <button
          type="button"
          disabled={!s.connected}
          onClick={() => s.send("ULS", { listMode: true, pending: "ULS" })}
          className="rounded-md bg-teams-brand px-4 py-2 text-sm font-semibold text-white disabled:opacity-40"
        >
          Liste utilisateurs (ULS)
        </button>
      </div>
      <div className="rounded-lg border border-teams-border bg-white p-4">
        <label className="text-xs font-medium text-teams-muted">UUID utilisateur</label>
        <div className="mt-2 flex gap-2">
          <input
            className="flex-1 rounded border border-teams-border bg-white px-3 py-2 font-mono text-sm outline-none focus:ring-2 focus:ring-teams-brand/50"
            value={uuid}
            onChange={(e) => setUuid(e.target.value)}
          />
          <button
            type="button"
            disabled={!s.connected || !uuid.trim()}
            onClick={() => s.send(`UGT uuid=${urlEncode(uuid.trim())}`, { pending: "UGT" })}
            className="rounded-md border border-teams-border px-4 py-2 text-sm font-medium text-teams-text hover:bg-teams-hover disabled:opacity-40"
          >
            Détails (UGT)
          </button>
        </div>
        {d.infoDetail && Object.keys(d.infoDetail).length > 0 ? (
          <pre className="mt-4 max-h-48 overflow-auto rounded border border-teams-border bg-[#FAFAFA] p-3 font-mono text-xs text-teams-muted">
            {JSON.stringify(d.infoDetail, null, 2)}
          </pre>
        ) : null}
      </div>
      <div className="min-h-0 flex-1 overflow-y-auto rounded-lg border border-teams-border bg-[#FAFAFA] p-4">
        <h2 className="mb-3 text-xs font-semibold uppercase tracking-wide text-teams-muted">Résultat ULS</h2>
        <ul className="divide-y divide-teams-border/60">
          {d.users.map((u) => (
            <li key={u.uuid} className="flex items-center justify-between py-2 text-sm">
              <span className="font-medium text-teams-text">{u.name}</span>
              <span className="font-mono text-[10px] text-teams-muted">{u.uuid}</span>
              <span className={u.logged_in === "1" ? "text-emerald-400" : "text-teams-muted"}>
                {u.logged_in === "1" ? "●" : "○"}
              </span>
            </li>
          ))}
        </ul>
      </div>
    </div>
  );
}
