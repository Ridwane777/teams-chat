export function getClientId(): string {
  if (typeof window === "undefined") return "";
  let id = localStorage.getItem("myteams_web_cid");
  if (!id) {
    id = crypto.randomUUID();
    localStorage.setItem("myteams_web_cid", id);
  }
  return id;
}
