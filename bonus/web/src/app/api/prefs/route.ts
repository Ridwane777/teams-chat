import { NextRequest, NextResponse } from "next/server";
import { getMongoClient, mongoConfigured, mongoDbName } from "@/lib/mongodb";

export async function GET(req: NextRequest) {
  const clientId = req.nextUrl.searchParams.get("clientId");
  if (!clientId) {
    return NextResponse.json({ ok: false, error: "clientId requis" }, { status: 400 });
  }
  if (!mongoConfigured()) {
    return NextResponse.json({ ok: true, mongo: false, prefs: null });
  }
  const cl = await getMongoClient();
  if (!cl) {
    return NextResponse.json({ ok: true, mongo: false, prefs: null });
  }
  const doc = await cl.db(mongoDbName()).collection("prefs").findOne<{
    clientId: string;
    mtHost: string;
    mtPort: number;
    bridgePort: number;
  }>({ clientId });
  if (!doc) {
    return NextResponse.json({ ok: true, mongo: true, prefs: null });
  }
  return NextResponse.json({
    ok: true,
    mongo: true,
    prefs: { mtHost: doc.mtHost, mtPort: doc.mtPort, bridgePort: doc.bridgePort },
  });
}

export async function POST(req: NextRequest) {
  const body = (await req.json()) as {
    clientId?: string;
    mtHost?: string;
    mtPort?: number;
    bridgePort?: number;
  };
  if (!body.clientId) {
    return NextResponse.json({ ok: false, error: "clientId requis" }, { status: 400 });
  }
  if (!mongoConfigured()) {
    return NextResponse.json({ ok: true, mongo: false, saved: false });
  }
  const cl = await getMongoClient();
  if (!cl) {
    return NextResponse.json({ ok: true, mongo: false, saved: false });
  }
  await cl
    .db(mongoDbName())
    .collection("prefs")
    .updateOne(
      { clientId: body.clientId },
      {
        $set: {
          clientId: body.clientId,
          mtHost: String(body.mtHost ?? "127.0.0.1"),
          mtPort: Number(body.mtPort) || 8080,
          bridgePort: Number(body.bridgePort) || 4250,
          updatedAt: new Date(),
        },
      },
      { upsert: true }
    );
  return NextResponse.json({ ok: true, mongo: true, saved: true });
}
