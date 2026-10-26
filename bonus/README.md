# Bonus: web interface (Next.js + React + Tailwind + MongoDB)

A web interface that drives the C++ TCP server `myteams_server` using the same protocol as the CLI client (`LOG`, `LST`, `TSB`, etc.). A WebSocket-to-TCP bridge (`bonus/bridge`) is needed because a browser cannot open a raw TCP socket to the server.

## Requirements

- Node.js 18+
- `myteams_server` built and running (e.g. on port `8080`)
- Optional: MongoDB to store connection preferences (host / ports) and a log of the protocol lines seen by the UI (`MONGODB_URI` in the Next.js environment)

## Getting started

Terminal 1, C++ server:

```bash
./myteams_server 8080
```

Terminal 2, bridge:

```bash
cd bonus/bridge
npm install
npm start
```

Defaults: WebSocket on port **4250**, TCP to **127.0.0.1:8080** (override with `MYTEAMS_HOST`, `MYTEAMS_PORT`, `BRIDGE_PORT`).

Terminal 3, Next.js front end:

```bash
cd bonus/web
cp .env.example .env.local   # optional: set MONGODB_URI
npm install
npm run dev
```

Open **http://127.0.0.1:3000**, go to **Settings**, connect the bridge, then log in (`LOG`). The Teams, Chat and People views only send frames already defined by the protocol.

## Layout

| Folder    | Role |
| --------- | ---- |
| `bridge/` | Node (`ws` + `net`): one WebSocket session maps to one TCP session to the server |
| `web/`    | Next.js (App Router): Teams-like shell, pages mirroring the CLI client, plus `prefs` / `activity` API routes (MongoDB optional) |

The C++ binaries do not depend on anything in this folder. The bridge, the web front end and MongoDB are optional extras.

Note: the UI labels are still in French.
