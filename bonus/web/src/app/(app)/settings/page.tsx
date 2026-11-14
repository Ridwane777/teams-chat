"use client";

import { useEffect, useState } from "react";
import { useSession } from "@/context/session-context";
import { getClientId } from "@/lib/client-id";
import { urlEncode } from "@/lib/protocol";

type PrefsPayload = {
  ok: boolean;
  mongo?: boolean;
  prefs?: { mtHost?: string; mtPort?: number; bridgePort?: number } | null;
  error?: string;
};

export default function SettingsPage() {
  const s = useSession();
  const { setMtHost, setMtPort, setBridgePort } = s;
  const [mongo, setMongo] = useState<boolean | null>(null);
  const [loadErr, setLoadErr] = useState<string | null>(null);
  const [saveMsg, setSaveMsg] = useState<string | null>(null);
  const [registerMsg, setRegisterMsg] = useState<string | null>(null);

  useEffect(() => {
    const id = getClientId();
    if (!id) return;
    void fetch(`/api/prefs?clientId=${encodeURIComponent(id)}`)
      .then((r) => r.json() as Promise<PrefsPayload>)
      .then((j) => {
        if (!j.ok) {
          setLoadErr(j.error ?? "Erreur");
          return;
        }
        setMongo(j.mongo ?? false);
        if (j.prefs?.mtHost) setMtHost(j.prefs.mtHost);
        if (typeof j.prefs?.mtPort === "number") setMtPort(j.prefs.mtPort);
        if (typeof j.prefs?.bridgePort === "number") setBridgePort(j.prefs.bridgePort);
      })
      .catch(() => setLoadErr("Impossible de joindre l’API Next."));
  }, [setMtHost, setMtPort, setBridgePort]);

  const savePrefs = () => {
    const id = getClientId();
    setSaveMsg(null);
    void fetch("/api/prefs", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({
        clientId: id,
        mtHost: s.mtHost,
        mtPort: s.mtPort,
        bridgePort: s.bridgePort,
      }),
    })
      .then((r) => r.json() as Promise<PrefsPayload & { saved?: boolean }>)
      .then((j) => {
        if (!j.ok) setSaveMsg(j.error ?? "Erreur");
        else {
          setMongo(j.mongo ?? false);
          setSaveMsg(j.mongo ? "Enregistré dans MongoDB." : "MongoDB non configuré (MONGODB_URI) - rien n’a été persisté.");
        }
      })
      .catch(() => setSaveMsg("Échec réseau."));
  };

  return (
    <div className="mx-auto max-w-xl space-y-8 bg-white p-6">
      <header>
        <h1 className="text-xl font-bold text-teams-text">Parametres</h1>
        <p className="mt-1 text-sm text-teams-muted">Connexion au pont et persistance optionnelle (MongoDB).</p>
      </header>

      <section className="rounded-lg border border-teams-border bg-white p-5">
        <h2 className="text-sm font-semibold text-teams-text">Pont WebSocket vers TCP</h2>
        <div className="mt-4 grid gap-3 sm:grid-cols-2">
          <label className="text-xs text-teams-muted">
            Hôte myteams_server
            <input
              className="mt-1 w-full rounded border border-teams-border bg-white px-3 py-2 text-sm text-teams-text outline-none focus:ring-2 focus:ring-teams-brand/40"
              value={s.mtHost}
              onChange={(e) => s.setMtHost(e.target.value)}
            />
          </label>
          <label className="text-xs text-teams-muted">
            Port TCP
            <input
              type="number"
              className="mt-1 w-full rounded border border-teams-border bg-white px-3 py-2 text-sm text-teams-text outline-none focus:ring-2 focus:ring-teams-brand/40"
              value={s.mtPort}
              onChange={(e) => s.setMtPort(Number(e.target.value) || 0)}
            />
          </label>
          <label className="text-xs text-teams-muted sm:col-span-2">
            Port du pont (WebSocket)
            <input
              type="number"
              className="mt-1 w-full max-w-xs rounded border border-teams-border bg-white px-3 py-2 text-sm text-teams-text outline-none focus:ring-2 focus:ring-teams-brand/40"
              value={s.bridgePort}
              onChange={(e) => s.setBridgePort(Number(e.target.value) || 0)}
            />
          </label>
        </div>
        <div className="mt-4 flex flex-wrap gap-2">
          <button
            type="button"
            disabled={s.connected || s.connecting}
            onClick={() => s.connect()}
            className="rounded-md bg-teams-brand px-4 py-2 text-sm font-semibold text-white disabled:opacity-40"
          >
            Connecter au pont
          </button>
          <button
            type="button"
            disabled={!s.connected}
            onClick={() => s.disconnect()}
            className="rounded-md border border-teams-border px-4 py-2 text-sm font-medium text-teams-text hover:bg-teams-hover disabled:opacity-40"
          >
            Déconnecter
          </button>
        </div>
      </section>

      <section className="rounded-lg border border-teams-border bg-white p-5">
        <h2 className="text-sm font-semibold text-teams-text">Session (LOG / OUT)</h2>
        <p className="mt-2 text-xs text-teams-muted">Identifiant local : {getClientId() || "-"}</p>
        <LoginForm />
        <RegisterForm
          connected={s.connected}
          onRegisterMessage={setRegisterMsg}
          onAutoLogin={(name) => s.send(`LOG name=${urlEncode(name)}`, { tag: "LOG" })}
        />
        {registerMsg ? <p className="mt-2 text-xs text-teams-muted">{registerMsg}</p> : null}
      </section>

      <section className="rounded-lg border border-teams-border bg-white p-5">
        <h2 className="text-sm font-semibold text-teams-text">MongoDB</h2>
        <p className="mt-2 text-xs text-teams-muted">
          Variable <code className="rounded bg-[#F3F2F1] px-1">MONGODB_URI</code> cote Next.js. Etat detecte :{" "}
          {mongo === null ? "..." : mongo ? "connecte" : "non configure"}
        </p>
        {loadErr ? <p className="mt-2 text-xs text-red-600">{loadErr}</p> : null}
        <button
          type="button"
          onClick={savePrefs}
          className="mt-4 rounded-md border border-teams-brand/40 px-4 py-2 text-sm font-medium text-teams-brand hover:bg-teams-activeBg"
        >
          Enregistrer hote / ports dans MongoDB
        </button>
        {saveMsg ? <p className="mt-2 text-xs text-teams-muted">{saveMsg}</p> : null}
      </section>
    </div>
  );
}

function RegisterForm({
  connected,
  onRegisterMessage,
  onAutoLogin,
}: {
  connected: boolean;
  onRegisterMessage: (msg: string) => void;
  onAutoLogin: (displayName: string) => void;
}) {
  const [displayName, setDisplayName] = useState("");
  const [email, setEmail] = useState("");

  const register = () => {
    onRegisterMessage("");
    void fetch("/api/register", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ displayName, email }),
    })
      .then(async (r) => ({ ok: r.ok, body: await r.json() }))
      .then(({ ok, body }) => {
        if (!ok) {
          onRegisterMessage(body.error ?? "Registration failed");
          return;
        }
        onRegisterMessage("Compte cree. Tu peux maintenant te connecter.");
        if (connected) onAutoLogin(displayName.trim());
      })
      .catch(() => onRegisterMessage("Erreur reseau pendant inscription"));
  };

  return (
    <div className="mt-5 border-t border-teams-border pt-4">
      <h3 className="text-sm font-semibold text-teams-text">Inscription web</h3>
      <p className="mt-1 text-xs text-teams-muted">
        Cree un profil dans MongoDB puis login automatique si le pont est connecte.
      </p>
      <div className="mt-3 grid gap-2 sm:grid-cols-2">
        <input
          value={displayName}
          onChange={(e) => setDisplayName(e.target.value)}
          placeholder="Nom affiché"
          className="rounded border border-teams-border px-3 py-2 text-sm outline-none focus:ring-2 focus:ring-teams-brand/40"
        />
        <input
          value={email}
          onChange={(e) => setEmail(e.target.value)}
          placeholder="Email"
          className="rounded border border-teams-border px-3 py-2 text-sm outline-none focus:ring-2 focus:ring-teams-brand/40"
        />
      </div>
      <button
        type="button"
        onClick={register}
        disabled={!displayName.trim() || !email.trim()}
        className="mt-3 rounded-md bg-[#4D98EC] px-4 py-2 text-sm font-semibold text-white disabled:opacity-40"
      >
        S&apos;inscrire
      </button>
    </div>
  );
}

function LoginForm() {
  const s = useSession();
  const [name, setName] = useState("");

  return (
    <div className="mt-4 flex flex-col gap-3 sm:flex-row sm:items-end">
      <label className="flex-1 text-xs text-teams-muted">
        Nom (login LOG)
        <input
          className="mt-1 w-full rounded border border-teams-border bg-white px-3 py-2 text-sm text-teams-text outline-none focus:ring-2 focus:ring-teams-brand/40"
          value={name}
          onChange={(e) => setName(e.target.value)}
        />
      </label>
      <button
        type="button"
        disabled={!s.connected || !name.trim()}
        onClick={() => s.send(`LOG name=${urlEncode(name.trim())}`, { tag: "LOG" })}
        className="rounded-md bg-teams-brand px-4 py-2 text-sm font-semibold text-white disabled:opacity-40"
      >
        Login
      </button>
      <button
        type="button"
        disabled={!s.connected}
        onClick={() => s.send("OUT", { tag: "OUT" })}
        className="rounded-md border border-teams-border px-4 py-2 text-sm text-teams-text hover:bg-teams-hover disabled:opacity-40"
      >
        Logout
      </button>
    </div>
  );
}
