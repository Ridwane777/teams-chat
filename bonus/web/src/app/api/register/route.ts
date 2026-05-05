import { NextResponse } from "next/server";
import { getMongoClient, mongoConfigured, mongoDbName } from "@/lib/mongodb";

export async function POST(req: Request) {
  const body = (await req.json()) as {
    displayName?: string;
    email?: string;
  };

  const displayName = (body.displayName ?? "").trim();
  const email = (body.email ?? "").trim().toLowerCase();

  if (!displayName || !email) {
    return NextResponse.json(
      { ok: false, error: "displayName and email are required" },
      { status: 400 }
    );
  }

  if (!mongoConfigured()) {
    return NextResponse.json(
      { ok: false, error: "MongoDB not configured in .env.local" },
      { status: 400 }
    );
  }

  const cl = await getMongoClient();
  if (!cl) {
    return NextResponse.json({ ok: false, error: "Mongo unavailable" }, { status: 503 });
  }

  const users = cl.db(mongoDbName()).collection("web_users");
  const existing = await users.findOne({ email });
  if (existing) {
    return NextResponse.json({ ok: false, error: "Account already exists" }, { status: 409 });
  }

  const doc = {
    displayName,
    email,
    createdAt: new Date(),
  };
  await users.insertOne(doc);

  return NextResponse.json({ ok: true, user: { displayName, email } });
}
