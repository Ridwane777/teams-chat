"use client";

import Link from "next/link";
import { usePathname } from "next/navigation";
import type { ReactNode } from "react";

function RailIcon({ active, icon, label }: { active: boolean; icon: string; label: string }) {
  return (
    <div className="flex w-full flex-col items-center gap-1">
      <span
        className={`flex h-9 w-9 items-center justify-center rounded-xl text-[15px] transition ${
          active ? "bg-[#464775] text-white shadow-[0_6px_16px_rgba(0,0,0,0.25)]" : "text-white/80 hover:bg-white/10"
        }`}
      >
        {icon}
      </span>
      <span className={`text-[9px] leading-none ${active ? "text-white" : "text-white/70"}`}>{label}</span>
    </div>
  );
}

export default function TeamsShell({ children }: { children: ReactNode }) {
  const pathname = usePathname() ?? "/";

  const nav = [
    { href: "/", label: "Activity", icon: "🏠" },
    { href: "/chat", label: "Chat", icon: "💬" },
    { href: "/people", label: "Teams", icon: "👥" },
    { href: "/protocol", label: "Files", icon: "📁" },
    { href: "/settings", label: "Settings", icon: "⚙️" },
  ];

  return (
    <div className="flex h-screen min-h-0 bg-[#f5f5f5] p-3">
      <div className="flex min-h-0 w-full overflow-hidden rounded-2xl border border-[#e6e6e6] bg-[#ffffff] shadow-[0_10px_36px_rgba(0,0,0,0.08)]">
        <aside className="flex w-[72px] shrink-0 flex-col items-center bg-[#2b2c4d] py-4">
          <div className="mb-4 mt-1 flex h-10 w-10 items-center justify-center rounded-xl bg-[#6264a7] text-sm font-bold text-white">
            MT
          </div>
          {nav.map((item) => {
            const active = item.href === "/" ? pathname === "/" : pathname === item.href || pathname.startsWith(`${item.href}/`);
            return (
              <Link key={item.href} href={item.href} className="mb-2 w-full px-2 py-0.5">
                <RailIcon active={active} icon={item.icon} label={item.label} />
              </Link>
            );
          })}
          <div className="mt-auto rounded-xl bg-white/10 px-2 py-2 text-[10px] text-white/80">v2</div>
        </aside>

        <section className="flex w-[280px] shrink-0 flex-col border-r border-[#edebe9] bg-[#f7f7f7]">
          <div className="border-b border-[#edebe9] px-4 py-4">
            <div className="mb-2 text-[20px] font-semibold text-[#252423]">My Teams</div>
            <div className="flex items-center rounded-md border border-[#d1d1d1] bg-white px-2.5 py-2 text-[13px] text-[#605e5c]">
              🔎 <span className="ml-2">Search</span>
            </div>
          </div>
          <div className="space-y-1 p-2">
            {["Home", "Calendar", "Calls", "Assigned", "Saved"].map((entry) => (
              <button key={entry} type="button" className="flex w-full items-center rounded-md px-3 py-2 text-left text-[13px] text-[#323130] hover:bg-[#ededed]">
                {entry}
              </button>
            ))}
          </div>
          <div className="mt-auto border-t border-[#edebe9] p-3 text-[12px] text-[#605e5c]">Workspace: collaborative mode</div>
        </section>

        <div className="min-h-0 flex-1 bg-white">{children}</div>
      </div>
    </div>
  );
}
