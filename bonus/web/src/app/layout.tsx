import type { Metadata } from "next";
import "./globals.css";
import { Providers } from "./providers";
import AppSplash from "@/components/teams/app-splash";

export const metadata: Metadata = {
  title: "MY TEAMS — interface bonus",
  description: "Interface web pilotant myteams_server (protocole TCP existant).",
};

export default function RootLayout({ children }: { children: React.ReactNode }) {
  return (
    <html lang="fr">
      <body className="font-sans">
        <Providers>
          <AppSplash>{children}</AppSplash>
        </Providers>
      </body>
    </html>
  );
}
