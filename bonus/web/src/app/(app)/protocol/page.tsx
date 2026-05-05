"use client";

import { useSession } from "@/context/session-context";

export default function ProtocolPage() {
  const s = useSession();

  return (
    <div className="flex h-full flex-col p-6 bg-white">
      <header className="mb-4 flex flex-wrap items-end justify-between gap-3">
        <div>
          <h1 className="text-xl font-bold text-teams-text">Flux protocole</h1>
          <p className="mt-1 text-sm text-teams-muted">Lignes brutes reçues / envoyées via le pont WebSocket.</p>
        </div>
        <button
          type="button"
          onClick={() => s.clearLog()}
          className="rounded-md border border-teams-border px-3 py-1.5 text-xs font-medium text-teams-text hover:bg-teams-hover"
        >
          Effacer
        </button>
      </header>
      <div className="min-h-0 flex-1 overflow-y-auto rounded-lg border border-teams-border bg-[#FAFAFA] p-3 font-mono text-[11px] leading-relaxed">
        {s.log.length === 0 ? (
          <span className="text-teams-muted">Aucune entrée.</span>
        ) : (
          s.log.map((e) => (
            <div
              key={e.id}
              className={`mb-1 border-b border-white/5 pb-1 ${
                e.dir === "in" ? "text-emerald-700" : e.dir === "out" ? "text-sky-700" : e.dir === "err" ? "text-red-700" : "text-teams-muted"
              }`}
            >
              <span className="opacity-50">[{e.dir}]</span> {e.text}
            </div>
          ))
        )}
      </div>
    </div>
  );
}
