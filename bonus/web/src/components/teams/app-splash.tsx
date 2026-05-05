"use client";

import { useEffect, useState } from "react";

const nodes = [
  { left: "8%", top: "58%" },
  { left: "20%", top: "24%" },
  { left: "35%", top: "72%" },
  { left: "49%", top: "38%" },
  { left: "66%", top: "60%" },
  { left: "79%", top: "28%" },
  { left: "90%", top: "54%" },
];

const links = [
  { left: "9.5%", top: "61%", width: 88, rotate: -30 },
  { left: "22%", top: "28%", width: 102, rotate: 26 },
  { left: "36%", top: "74%", width: 86, rotate: -37 },
  { left: "50.5%", top: "41%", width: 94, rotate: 21 },
  { left: "67%", top: "62.5%", width: 74, rotate: -35 },
  { left: "80%", top: "31%", width: 72, rotate: 28 },
];

export default function AppSplash({ children }: { children: React.ReactNode }) {
  const [showSplash, setShowSplash] = useState(true);

  useEffect(() => {
    const timer = window.setTimeout(() => setShowSplash(false), 1200);
    return () => window.clearTimeout(timer);
  }, []);

  if (!showSplash) return <>{children}</>;

  return (
    <div className="flex h-full min-h-screen flex-col items-center justify-center bg-[#f3f2f1] px-6">
      <div className="mb-6 flex h-20 w-20 items-center justify-center rounded-2xl bg-[#6264a7] text-2xl font-bold text-white shadow-[0_14px_30px_rgba(98,100,167,0.35)]">
        MT
      </div>
      <h1 className="mb-2 text-3xl font-bold tracking-tight text-[#252423]">My_Teams</h1>
      <p className="mb-8 text-sm text-[#605e5c]">Synchronisation des espaces collaboratifs...</p>

      <div className="network-loader">
        {links.map((link, i) => (
          <span
            key={`link-${i}`}
            className="network-link"
            style={{ left: link.left, top: link.top, width: `${link.width}px`, transform: `rotate(${link.rotate}deg)` }}
          />
        ))}
        {nodes.map((node, i) => (
          <span key={`node-${i}`} className="network-node" style={{ left: node.left, top: node.top, animationDelay: `${i * 120}ms` }} />
        ))}
      </div>
    </div>
  );
}
