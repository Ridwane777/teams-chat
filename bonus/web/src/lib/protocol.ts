/** Aligné sur `Protocol::encode` / `Client::url_encode` côté C++. */
export function urlEncode(str: string): string {
  const hex = "0123456789ABCDEF";
  let res = "";
  for (let i = 0; i < str.length; i++) {
    const c = str.charCodeAt(i);
    if (
      (c >= 48 && c <= 57) ||
      (c >= 65 && c <= 90) ||
      (c >= 97 && c <= 122) ||
      c === 45 ||
      c === 95 ||
      c === 46 ||
      c === 126
    ) {
      res += str[i];
    } else {
      res += "%" + hex[(c >> 4) & 0xf] + hex[c & 0xf];
    }
  }
  return res;
}

export function urlDecode(str: string): string {
  let res = "";
  for (let i = 0; i < str.length; i++) {
    if (str[i] === "%" && i + 2 < str.length) {
      const v = parseInt(str.slice(i + 1, i + 3), 16);
      if (!Number.isNaN(v)) {
        res += String.fromCharCode(v);
        i += 2;
        continue;
      }
    }
    res += str[i];
  }
  return res;
}

export type ParsedFrame = { opcode: string; fields: Record<string, string> };

export function parseFrame(raw: string): ParsedFrame | null {
  let line = raw.replace(/\r$/, "");
  while (line.endsWith("\n")) line = line.slice(0, -1);
  const sp = line.indexOf(" ");
  const opcode = sp === -1 ? line : line.slice(0, sp);
  if (opcode.length !== 2 && opcode.length !== 3) return null;
  if (!/^[A-Z]+$/.test(opcode)) return null;
  const fields: Record<string, string> = {};
  if (sp === -1) return { opcode, fields };
  const rest = line.slice(sp + 1);
  const tokens = rest.split(/\s+/).filter(Boolean);
  for (const token of tokens) {
    const eq = token.indexOf("=");
    if (eq <= 0 || eq === token.length - 1) return null;
    const key = token.slice(0, eq);
    fields[key] = urlDecode(token.slice(eq + 1));
  }
  return { opcode, fields };
}
