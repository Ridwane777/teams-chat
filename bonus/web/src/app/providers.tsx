"use client";

import type { ReactNode } from "react";
import { SessionProvider } from "@/context/session-context";
import { WorkspaceProvider } from "@/context/workspace-context";
import { DomainProvider } from "@/context/domain-context";

export function Providers({ children }: { children: ReactNode }) {
  return (
    <SessionProvider>
      <WorkspaceProvider>
        <DomainProvider>{children}</DomainProvider>
      </WorkspaceProvider>
    </SessionProvider>
  );
}
