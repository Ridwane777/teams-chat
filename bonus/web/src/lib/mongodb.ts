import { MongoClient } from "mongodb";

const uri = process.env.MONGODB_URI;

let client: MongoClient | null = null;

export function mongoConfigured(): boolean {
  return Boolean(uri && uri.length > 0);
}

export async function getMongoClient(): Promise<MongoClient | null> {
  if (!uri) return null;
  if (!client) {
    client = new MongoClient(uri);
    await client.connect();
  }
  return client;
}

export function mongoDbName(): string {
  return process.env.MONGODB_DB ?? "myteams_bonus";
}
