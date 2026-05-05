# Bonus — interface web (Next.js + React + Tailwind + MongoDB)

Interface web pilotant le **serveur TCP C++** `myteams_server`, avec le même **protocole** que le client CLI (`LOG`, `LST`, `TSB`, etc.). Un **pont WebSocket → TCP** (`bonus/bridge`) est nécessaire car le navigateur ne parle pas en socket brut vers le projet.

## Prérequis

- Node.js 18+
- `myteams_server` compilé et lancé (ex. port `8080`)
- **Optionnel** : MongoDB pour enregistrer les préférences de connexion (hôte / ports) et un journal des lignes de protocole vues par l’UI (`MONGODB_URI` dans l’environnement Next.js).

## Démarrage

Terminal 1 — serveur C++ :

```bash
./myteams_server 8080
```

Terminal 2 — pont :

```bash
cd bonus/bridge
npm install
npm start
```

Par défaut : WebSocket sur le port **4250**, TCP vers **127.0.0.1:8080** (variables `MYTEAMS_HOST`, `MYTEAMS_PORT`, `BRIDGE_PORT`).

Terminal 3 — interface **Next.js** :

```bash
cd bonus/web
cp .env.example .env.local   # optionnel : renseigner MONGODB_URI
npm install
npm run dev
```

Ouvre **http://127.0.0.1:3000**. Va dans **Paramètres** : connecte le pont, puis **Login** (`LOG`). Les vues **Équipes**, **Discussion**, **Personnes** n’envoient que les trames déjà définies par le sujet.

## Structure

| Dossier      | Rôle |
| ------------ | ---- |
| `bridge/`    | Node (`ws` + `net`) : une session WebSocket ↔ une session TCP vers myteams |
| `web/`       | Next.js (App Router) : shell façon Teams, pages alignées sur le client CLI + API `prefs` / `activity` (MongoDB optionnel) |

## Moulinette / Linux

Pour la notation Ridwane777, le binaire doit lier la **vraie** `libmyteams.so` sous Linux. Le pont, le front Next et MongoDB sont du bonus.
