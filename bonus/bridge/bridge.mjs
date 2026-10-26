import { WebSocketServer } from "ws";
import net from "node:net";

const DEFAULT_TCP_HOST = process.env.MYTEAMS_HOST ?? "127.0.0.1";
const DEFAULT_TCP_PORT = Number(process.env.MYTEAMS_PORT ?? 8080);
const WS_PORT = Number(process.env.BRIDGE_PORT ?? 4250);

const wss = new WebSocketServer({ port: WS_PORT });

function parseTarget(req) {
  try {
    const u = new URL(req.url ?? "/", "http://127.0.0.1");
    const host = u.searchParams.get("mtHost") || DEFAULT_TCP_HOST;
    const port = Number(u.searchParams.get("mtPort") || DEFAULT_TCP_PORT);
    return { host, port: Number.isFinite(port) ? port : DEFAULT_TCP_PORT };
  } catch {
    return { host: DEFAULT_TCP_HOST, port: DEFAULT_TCP_PORT };
  }
}

wss.on("connection", (ws, req) => {
  const { host, port } = parseTarget(req);
  const socket = new net.Socket();
  let tcpBuf = "";

  const sendJson = (obj) => {
    if (ws.readyState === ws.OPEN) ws.send(JSON.stringify(obj));
  };

  socket.setEncoding("utf8");
  socket.once("connect", () => {
    sendJson({ type: "system", message: `TCP connected to ${host}:${port}` });
  });
  socket.on("data", (chunk) => {
    tcpBuf += chunk;
    let i;
    while ((i = tcpBuf.indexOf("\n")) >= 0) {
      const line = tcpBuf.slice(0, i).replace(/\r$/, "");
      tcpBuf = tcpBuf.slice(i + 1);
      sendJson({ type: "line", data: line });
    }
  });
  socket.on("error", (err) => {
    sendJson({ type: "error", message: err.message });
  });
  socket.on("close", () => {
    sendJson({ type: "system", message: "TCP connection closed" });
    ws.close();
  });

  socket.connect(port, host);

  ws.on("message", (data) => {
    let s = typeof data === "string" ? data : data.toString("utf8");
    if (!s.endsWith("\n")) s += "\n";
    socket.write(s);
  });
  ws.on("close", () => socket.destroy());
  ws.on("error", () => socket.destroy());
});

console.log(
  `WebSocket bridge ws://127.0.0.1:${WS_PORT}/?mtHost=…&mtPort=… → TCP (${DEFAULT_TCP_HOST}:${DEFAULT_TCP_PORT} by default)`
);
