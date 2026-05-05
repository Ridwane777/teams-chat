/** Aligné sur la logique `UserCommand::handleList` / `handleInfo` côté client C++. */

export function listPending(team: string, channel: string, thread: string): string {
  const t = team.trim();
  const c = channel.trim();
  const th = thread.trim();
  if (!t) return "/list_teams";
  if (!c) return "/list_channels";
  if (!th) return "/list_threads";
  return "/list_replies";
}

export function infoPending(team: string, channel: string, thread: string): string {
  const t = team.trim();
  const c = channel.trim();
  const th = thread.trim();
  if (!t) return "/info_user";
  if (!c) return "/info_team";
  if (!th) return "/info_channel";
  return "/info_thread";
}
