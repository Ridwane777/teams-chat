# teams-chat

[![CI](https://github.com/Ridwane777/teams-chat/actions/workflows/ci.yml/badge.svg)](https://github.com/Ridwane777/teams-chat/actions/workflows/ci.yml)

A Microsoft Teams-style chat in C++20: a TCP server and a command-line client that share a small text protocol. Users log in, create teams, channels, threads and replies, subscribe to teams and send private messages. Events (new team, new reply, incoming message, someone logging in) are pushed to connected clients in real time. A Next.js web front end in `bonus/` talks to the same server through a WebSocket bridge.

## Features

- Users: login/logout, list users, user details, online status
- Teams, channels, threads and replies, with a `/use` context to navigate between them
- Team subscriptions; only subscribers can read or post in a team
- Private messages with history
- Server-pushed events for team, channel, thread and reply creation, logins and private messages
- State saved to disk on shutdown and reloaded on startup
- Unit tests for the protocol, server state, entities, client command parsing and response handling

## Architecture

**Protocol.** Line-based text frames, `<OPCODE> key=value ...\n`, with URL-encoded values. Clients send three-letter opcodes (`LOG`, `OUT`, `ULS`, `TCR`, `LST`, `INF`, `USE`...), the server answers `OK` / `ERR code=... reason=...`, and pushes `EVT` frames for asynchronous events. The full spec (frame format, size limits, every opcode and error code) is in [RFC-0001-MTAP.md](RFC-0001-MTAP.md).

**Server** (`src/server/`). A single-threaded `poll()` loop accepts clients and reads frames. `Protocol` decodes them, `CommandHandler` checks the session and context and dispatches each opcode, and `ServerState` holds users, teams, subscriptions, channels, threads, replies and private messages in memory. On `SIGINT` the state is written to `myteams.dat` (binary format) and loaded again at the next start.

**Client** (`src/client/`). A `poll()` loop watches both stdin and the server socket, so pushed events show up while the user is typing. `ClientCommand` parses commands such as `/login "alice"` or `/create "name" "description"` and encodes them as protocol frames. `ResponseHandler` decodes server replies and events and prints them through the logging library.

**Logging library** (`libs/logging/`). The project originally linked a course-provided display library that cannot be redistributed. `libs/logging` is a clean-room MIT-licensed reimplementation of the same C API, written without its source. Client functions print readable lines to stdout (errors to stderr), and server functions log one line per event to stderr. It is compiled from source by the Makefile, so the build works the same on Linux and macOS.

```text
include/, src/
  client/        client loop, ClientCommand, ResponseHandler
  server/        server loop, Protocol, CommandHandler, ServerState
  Entities.*     User, Team, Channel, Thread, Comment, Message
libs/logging/    display/log library (MIT, clean-room)
tests/           unit tests (custom runner, no external framework)
bonus/           WebSocket bridge + Next.js web client
RFC-0001-MTAP.md protocol specification
```

## Try it with Docker

One command starts the server, the WebSocket bridge and the web interface:

```bash
docker compose up --build
```

Then open http://localhost:3000, go to **Settings**, connect, and log in. The server also listens on port 8080 for
the CLI client:

```bash
docker compose exec server myteams_cli 127.0.0.1 8080
```

The server state is kept in the `teams-data` volume across restarts (`docker compose down -v` resets it).

## Build and run

Requirements: `clang++` with C++20, `make`, and `libuuid` on Linux (`sudo apt install clang make uuid-dev`).

```bash
make                 # builds myteams_server and myteams_cli
./myteams_server 4242
./myteams_cli 127.0.0.1 4242
```

Example session:

```text
/login "alice"
/create "core" "Core team"
/subscribe "<team_uuid>"
/use "<team_uuid>"
/create "general" "General channel"
/logout
```

Type `/help` in the client for the full command list. Other targets: `make server`, `make client`, `make clean`, `make fclean`, `make re`.

## Tests

```bash
make tests_run       # builds and runs the unit tests (21 tests)
make coverage        # same, plus a gcovr summary (needs gcovr in .venv)
./test_persistance.sh  # end-to-end check that state survives a server restart (needs nc)
```

`tests_run` leaves coverage-instrumented objects behind, so run `make re` before building the binaries again.

The CI workflow (`.github/workflows/ci.yml`) runs on every push and pull request to `main`: unit tests, build of both binaries, then the persistence test against a real server.

## Web bonus

`bonus/` contains a Teams-like web interface built with Next.js, React and Tailwind. Browsers cannot open raw TCP sockets, so a small Node.js bridge (`bonus/bridge`) maps each WebSocket session to one TCP connection to `myteams_server`. The web app sends the same protocol frames as the CLI client. MongoDB can optionally store connection preferences and a protocol log. See [bonus/README.md](bonus/README.md).

## Team

Built as a team of three. **My contribution (Ridwane777):** the whole CLI client - command parsing/encoding (ClientCommand), server response decoding and display (ResponseHandler), the poll()-based network loop - plus server-side data persistence and the CI.

Teammates:

- Jean-Victor Brun ([@Jean-Victor-vinksmark](https://github.com/Jean-Victor-vinksmark))
- Christopher Guidibi

## License

[MIT](LICENSE)
