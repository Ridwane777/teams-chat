import type { Config } from "tailwindcss";

const config: Config = {
  content: ["./src/**/*.{js,ts,jsx,tsx,mdx}"],
  theme: {
    extend: {
      colors: {
        teams: {
          brand: "#6264A7",
          brandDark: "#4F52A3",
          brandHover: "#4F52A3",
          activeBg: "#E8E8F4",
          shell: "#FFFFFF",
          rail: "#F5F5F5",
          panel: "#FFFFFF",
          panel2: "#F5F5F5",
          border: "#E0E0E0",
          text: "#242424",
          muted: "#616161",
          subtle: "#8A8886",
          hover: "#F0F0F0",
          success: "#92C353",
          accent: "#C4314B",
        },
      },
      fontFamily: {
        sans: ["var(--font-inter)", "Segoe UI", "system-ui", "sans-serif"],
      },
      boxShadow: {
        teams: "0 1px 2px rgba(0,0,0,0.08)",
      },
    },
  },
  plugins: [],
};

export default config;
