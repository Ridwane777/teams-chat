import { NextResponse } from "next/server";
import { getMongoClient, mongoConfigured, mongoDbName } from "@/lib/mongodb";

export async function POST(req: Request) {
  const body = (await req.json()) as {
    clientId?: string;
    direction?: string;
    line?: string;
  };
  if (!body.clientId || !body.line) {
    return NextResponse.json({ ok: false, error: "clientId et line requis" }, { status: 400 });
  }
  if (!mongoConfigured()) {
    return NextResponse.json({ ok: true, skipped: true });
  }
  const cl = await getMongoClient();
  if (!cl) {
    return NextResponse.json({ ok: true, skipped: true });
  }
  await cl.db(mongoDbName()).collection("activity_log").insertOne({
    clientId: body.clientId,
    direction: body.direction === "out" ? "out" : "in",
    line: String(body.line).slice(0, 4096),
    at: new Date(),
  });
  return NextResponse.json({ ok: true, stored: true });
}
