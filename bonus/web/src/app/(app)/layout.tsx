import type { ReactNode } from "react";
import TeamsShell from "@/components/teams/teams-shell";

export default function AppLayout({ children }: { children: ReactNode }) {
  return <TeamsShell>{children}</TeamsShell>;
}
