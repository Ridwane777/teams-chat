Network Working Group: Ridwane777,  Jean-Victor-vinksmark
Request for Comments: 11 April 2026
Category: Informational

═══════════════════════════════════════════════════════════════════════
         MyTeams Application Protocol (MTAP) - Version 1.0
═══════════════════════════════════════════════════════════════════════

Status of This Document

   This document describes the MyTeams Application Protocol (MTAP),
   the communication protocol used between the myteams_server and
   myteams_cli binaries. Distribution of this memo is unlimited.

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Table of Contents

   1.  Introduction
   2.  Conventions and Terminology
   3.  Transport Layer
   4.  Frame Format
       4.1.  Frame Header
       4.2.  Payload Encoding
       4.3.  Frame Size Limits
   5.  Opcodes
       5.1.  Client → Server Opcodes
       5.2.  Server → Client Opcodes
   6.  Command Reference (Client → Server)
       6.1.  LOGIN
       6.2.  LOGOUT
       6.3.  USERS_LIST
       6.4.  USER_GET
       6.5.  MSG_SEND
       6.6.  MSG_LIST
       6.7.  TEAM_CREATE
       6.8.  TEAM_SUBSCRIBE
       6.9.  TEAM_UNSUBSCRIBE
       6.10. TEAM_SUBSCRIBED
       6.11. CHANNEL_CREATE
       6.12. THREAD_CREATE
       6.13. COMMENT_CREATE
       6.14. LIST
       6.15. INFO
       6.16. USE
   7.  Event Reference (Server → Client)
       7.1.  User Events
       7.2.  Team Events
       7.3.  Channel Events
       7.4.  Thread Events
       7.5.  Comment Events
       7.6.  Message Events
   8.  Response Codes
   9.  Error Handling
   10. Session Lifecycle
   11. Security Considerations
   12. Example Exchanges

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

1. Introduction

   The MyTeams Application Protocol (MTAP) is a text-based, line-
   oriented, request/response protocol that operates over a persistent
   TCP connection. It is designed to support a collaborative
   communication service featuring teams, channels, threads, comments,
   and direct user-to-user messaging.

   The protocol is symmetric in that the server may push unsolicited
   event frames to connected clients at any time (e.g., to notify
   subscribers of new threads), while the client sends command frames
   and awaits response frames.

2. Conventions and Terminology

   The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHOULD",
   "MAY" in this document follow RFC 2119.

   CLIENT   - An instance of myteams_cli connected to the server.
   SERVER   - The myteams_server process.
   UUID     - A 36-character string in the canonical 8-4-4-4-12 hex
              format as produced by uuid_unparse_lower(3).
   FRAME    - The atomic unit of communication on the wire.
   OPCODE   - A 3-character ASCII identifier for a command or event.
   FIELD    - A key=value pair within a payload.

3. Transport Layer

   MTAP runs over TCP (RFC 793).

   - The server MUST listen on the port given at startup.
   - Each client opens exactly one persistent TCP connection.
   - The connection persists until the client sends LOGOUT or the
     TCP connection is closed.
   - The server MUST support multiple simultaneous clients via poll(2).
     Use of fork(2) or threads is prohibited.

4. Frame Format

4.1. Frame Header

   Every frame - whether a command, response, or server event - has
   the following ASCII text structure:

      <OPCODE> <PAYLOAD>\n

   Where:
   - <OPCODE>   is exactly 3 uppercase ASCII characters.
   - <PAYLOAD>  is a space-separated list of key=value fields,
                or empty for frames with no parameters.
   - \n         is a single newline character (0x0A) that terminates
                the frame. Carriage returns (0x0D) MUST be ignored.

   Example:

      LOG name=alice\n

   A frame with no payload fields uses an empty string after the opcode:

      LST\n

4.2. Payload Encoding

   Field values MUST be URL-encoded (percent-encoding, RFC 3986) when
   they contain any of the following reserved characters:

      SPACE  →  %20
      =      →  %3D
      &      →  %26
      \n     →  %0A
      \r     →  %0D

   Field ordering within a payload is not significant. Parsers MUST
   accept fields in any order. Unrecognised fields MUST be silently
   ignored.

   UUID fields are transmitted as their canonical 36-character string
   (e.g., "550e8400-e29b-41d4-a716-446655440000").

   Timestamps are transmitted as decimal UNIX epoch seconds (time_t).

4.3. Frame Size Limits

   The maximum encoded frame length (including opcode, space, payload,
   and newline) is 4096 bytes. Frames exceeding this limit MUST be
   rejected with ERR code 400.

   Individual field value limits follow the data model:

      name          32 characters (before encoding)
      description  255 characters (before encoding)
      body         512 characters (before encoding)
      uuid          36 characters

5. Opcodes

5.1. Client → Server Opcodes

   ┌────────┬────────────────────────────────────────────────────────┐
   │ Opcode │ Meaning                                                │
   ├────────┼────────────────────────────────────────────────────────┤
   │ LOG    │ Login (set username for this session)                  │
   │ OUT    │ Logout                                                 │
   │ ULS    │ List all users                                         │
   │ UGT    │ Get details of one user                               │
   │ SND    │ Send a direct message to a user                       │
   │ MLS    │ List messages exchanged with a user                   │
   │ TCR    │ Create a team                                          │
   │ TSB    │ Subscribe to a team                                    │
   │ TUS    │ Unsubscribe from a team                               │
   │ TSD    │ List subscribed teams / users subscribed to a team    │
   │ CCR    │ Create a channel (requires team context)              │
   │ HCR    │ Create a thread (requires team+channel context)       │
   │ OCR    │ Create a comment (requires team+channel+thread ctx)   │
   │ LST    │ List sub-resources (context-dependent)                │
   │ INF    │ Show info (context-dependent)                         │
   │ USE    │ Set context (team / channel / thread)                 │
   └────────┴────────────────────────────────────────────────────────┘

5.2. Server → Client Opcodes

   ┌────────┬────────────────────────────────────────────────────────┐
   │ Opcode │ Meaning                                                │
   ├────────┼────────────────────────────────────────────────────────┤
   │ OK     │ Success response (with optional payload)              │
   │ ERR    │ Error response                                         │
   │ EVT    │ Asynchronous server event                             │
   └────────┴────────────────────────────────────────────────────────┘

   The EVT frame carries a sub-type field "event" to identify the
   specific event (see Section 7).

6. Command Reference (Client → Server)

6.1. LOGIN  -  Opcode: LOG

   Sent by the client to establish a named session. If a user with
   the given name already exists, the session is resumed. Otherwise
   a new user record is created.

   A client MUST send LOG before any other command. Commands received
   before LOG MUST be rejected with ERR code 401.

   Request:
      LOG name=<username>\n

   Fields:
      name    REQUIRED. 1–32 printable characters.

   Success response:
      OK uuid=<user_uuid> name=<username> logged_in=1\n

   Error responses:
      ERR code=400 reason=invalid_name\n      (name too long / empty)
      ERR code=409 reason=already_logged_in\n (session already active)

6.2. LOGOUT  -  Opcode: OUT

   Disconnects the client. The server marks the user as offline and
   broadcasts a USER_OFFLINE event to all connected clients.

   Request:
      OUT\n

   Success response (sent before closing the connection):
      OK\n

6.3. LIST ALL USERS  -  Opcode: ULS

   Returns one OK frame per user, followed by a final OK with
   field count=<n>.

   Request:
      ULS\n

   Success response (one frame per user):
      OK uuid=<uuid> name=<name> logged_in=<0|1>\n
      ...
      OK count=<n>\n

   Error:
      ERR code=401 reason=not_logged_in\n

6.4. GET USER  -  Opcode: UGT

   Request:
      UGT uuid=<user_uuid>\n

   Success:
      OK uuid=<uuid> name=<name> logged_in=<0|1>\n

   Errors:
      ERR code=401 reason=not_logged_in\n
      ERR code=404 reason=user_not_found\n

6.5. SEND MESSAGE  -  Opcode: SND

   Sends a direct message to another user. The server stores the
   message and, if the recipient is online, delivers an MSG_RECEIVED
   event to their connection.

   Request:
      SND to=<user_uuid> body=<message_body>\n

   Fields:
      to      REQUIRED. UUID of the recipient.
      body    REQUIRED. 1–512 characters (URL-encoded).

   Success:
      OK uuid=<message_uuid> timestamp=<epoch>\n

   Errors:
      ERR code=401 reason=not_logged_in\n
      ERR code=404 reason=user_not_found\n
      ERR code=400 reason=body_too_long\n
      ERR code=403 reason=cannot_message_self\n

6.6. LIST MESSAGES  -  Opcode: MLS

   Lists all messages exchanged between the current user and another.
   Messages are returned in ascending chronological order.

   Request:
      MLS with=<user_uuid>\n

   Success (one frame per message):
      OK uuid=<uuid> from=<sender_uuid> to=<receiver_uuid>
         body=<body> timestamp=<epoch>\n
      ...
      OK count=<n>\n

   Errors:
      ERR code=401 reason=not_logged_in\n
      ERR code=404 reason=user_not_found\n

6.7. CREATE TEAM  -  Opcode: TCR

   Request:
      TCR name=<name> description=<description>\n

   Success:
      OK uuid=<team_uuid> name=<name>
         description=<description> creator=<user_uuid>\n

   Errors:
      ERR code=401 reason=not_logged_in\n
      ERR code=400 reason=name_too_long\n
      ERR code=409 reason=team_exists\n

6.8. SUBSCRIBE  -  Opcode: TSB

   Subscribes the current user to a team. After subscribing, the user
   receives all events related to that team (new channels, threads,
   comments).

   Request:
      TSB team=<team_uuid>\n

   Success:
      OK team=<team_uuid>\n

   Errors:
      ERR code=401 reason=not_logged_in\n
      ERR code=404 reason=team_not_found\n
      ERR code=409 reason=already_subscribed\n

6.9. UNSUBSCRIBE  -  Opcode: TUS

   Request:
      TUS team=<team_uuid>\n

   Success:
      OK team=<team_uuid>\n

   Errors:
      ERR code=401 reason=not_logged_in\n
      ERR code=404 reason=team_not_found\n
      ERR code=404 reason=not_subscribed\n

6.10. LIST SUBSCRIPTIONS  -  Opcode: TSD

   With no argument: lists all teams the current user is subscribed to.
   With a team UUID: lists all users subscribed to that team.

   Request (no team):
      TSD\n

   Request (with team):
      TSD team=<team_uuid>\n

   Success - teams (one per frame):
      OK uuid=<team_uuid> name=<name> description=<desc>\n
      ...
      OK count=<n>\n

   Success - members (one per frame):
      OK uuid=<user_uuid> name=<name> logged_in=<0|1>\n
      ...
      OK count=<n>\n

   Errors:
      ERR code=401 reason=not_logged_in\n
      ERR code=404 reason=team_not_found\n

6.11. CREATE CHANNEL  -  Opcode: CCR

   Requires an active team context (set via USE, Section 6.16).
   The requesting user MUST be subscribed to the team.

   Request:
      CCR name=<name> description=<description>\n

   Success:
      OK uuid=<channel_uuid> name=<name>
         description=<desc> team=<team_uuid>\n

   Errors:
      ERR code=401 reason=not_logged_in\n
      ERR code=403 reason=context_required\n    (no team in context)
      ERR code=403 reason=not_subscribed\n
      ERR code=409 reason=channel_exists\n

6.12. CREATE THREAD  -  Opcode: HCR

   Requires team + channel context. User MUST be subscribed.

   Request:
      HCR title=<title> body=<body>\n

   Success:
      OK uuid=<thread_uuid> title=<title> body=<body>
         channel=<channel_uuid> creator=<user_uuid>
         timestamp=<epoch>\n

   Errors:
      ERR code=401 reason=not_logged_in\n
      ERR code=403 reason=context_required\n
      ERR code=403 reason=not_subscribed\n

6.13. CREATE COMMENT  -  Opcode: OCR

   Requires team + channel + thread context. User MUST be subscribed.

   Request:
      OCR body=<body>\n

   Success:
      OK uuid=<comment_uuid> body=<body>
         thread=<thread_uuid> author=<user_uuid>
         timestamp=<epoch>\n

   Errors:
      ERR code=401 reason=not_logged_in\n
      ERR code=403 reason=context_required\n
      ERR code=403 reason=not_subscribed\n

6.14. LIST  -  Opcode: LST

   Context-sensitive listing command. Behaviour depends on USE context:

   ┌───────────────────────────┬─────────────────────────────────────┐
   │ Active Context            │ Lists                               │
   ├───────────────────────────┼─────────────────────────────────────┤
   │ None                      │ All existing teams                  │
   │ team_uuid                 │ All channels in the team            │
   │ team_uuid + channel_uuid  │ All threads in the channel          │
   │ team + channel + thread   │ All comments on the thread          │
   └───────────────────────────┴─────────────────────────────────────┘

   Request:
      LST\n

   Success: one OK frame per resource, then a summary:
      OK count=<n>\n

   Error:
      ERR code=401 reason=not_logged_in\n

6.15. INFO  -  Opcode: INF

   Context-sensitive info command:

   ┌───────────────────────────┬─────────────────────────────────────┐
   │ Active Context            │ Shows                               │
   ├───────────────────────────┼─────────────────────────────────────┤
   │ None                      │ Currently logged-in user            │
   │ team_uuid                 │ Team details                        │
   │ team_uuid + channel_uuid  │ Channel details                     │
   │ team + channel + thread   │ Thread details                      │
   └───────────────────────────┴─────────────────────────────────────┘

   Request:
      INF\n

   Success: one OK frame with the relevant fields.

   Error:
      ERR code=401 reason=not_logged_in\n
      ERR code=404 reason=resource_not_found\n

6.16. SET CONTEXT  -  Opcode: USE

   Sets the current working context for LST, INF, CCR, HCR, OCR.
   Sending USE with no arguments clears the context entirely.

   Request (clear context):
      USE\n

   Request (team only):
      USE team=<team_uuid>\n

   Request (team + channel):
      USE team=<team_uuid> channel=<channel_uuid>\n

   Request (team + channel + thread):
      USE team=<team_uuid> channel=<channel_uuid> thread=<thread_uuid>\n

   Success:
      OK team=<team_uuid|-> channel=<channel_uuid|-> thread=<thread_uuid|->\n

   "-" indicates the field is unset.

   Errors:
      ERR code=401 reason=not_logged_in\n
      ERR code=404 reason=team_not_found\n
      ERR code=404 reason=channel_not_found\n
      ERR code=404 reason=thread_not_found\n
      ERR code=400 reason=invalid_context\n   (e.g. thread w/o channel)

7. Event Reference (Server → Client)

   The server sends EVT frames to notify subscribed clients of changes.
   EVT frames are unsolicited - they may arrive at any time between
   command responses.

   General EVT format:
      EVT event=<event_name> [fields...]\n

7.1. User Events

   USER_ONLINE
      Sent to ALL connected clients when a user logs in.
      EVT event=USER_ONLINE uuid=<uuid> name=<name>\n

   USER_OFFLINE
      Sent to ALL connected clients when a user logs out or disconnects.
      EVT event=USER_OFFLINE uuid=<uuid> name=<name>\n

7.2. Team Events

   TEAM_CREATED
      Sent to ALL connected clients when a new team is created.
      EVT event=TEAM_CREATED uuid=<uuid> name=<name>
          description=<desc> creator=<user_uuid>\n

7.3. Channel Events

   CHANNEL_CREATED
      Sent to all clients subscribed to the team.
      EVT event=CHANNEL_CREATED uuid=<uuid> name=<name>
          description=<desc> team=<team_uuid>\n

7.4. Thread Events

   THREAD_CREATED
      Sent to all clients subscribed to the team.
      EVT event=THREAD_CREATED uuid=<uuid> title=<title>
          body=<body> channel=<channel_uuid> creator=<user_uuid>
          timestamp=<epoch>\n

7.5. Comment Events

   COMMENT_CREATED
      Sent to all clients subscribed to the team.
      EVT event=COMMENT_CREATED uuid=<uuid> body=<body>
          thread=<thread_uuid> author=<user_uuid>
          timestamp=<epoch>\n

7.6. Message Events

   MSG_RECEIVED
      Sent to the recipient if they are currently connected.
      EVT event=MSG_RECEIVED uuid=<msg_uuid> from=<sender_uuid>
          body=<body> timestamp=<epoch>\n

8. Response Codes

   Response codes follow HTTP conventions for familiarity:

   ┌──────┬─────────────────────────────────────────────────────────┐
   │ Code │ Meaning                                                 │
   ├──────┼─────────────────────────────────────────────────────────┤
   │  200 │ OK (implicit on all OK frames)                          │
   │  400 │ Bad Request (malformed frame, value too long, etc.)     │
   │  401 │ Unauthorized (command sent before LOGIN)                │
   │  403 │ Forbidden (missing subscription, invalid context)       │
   │  404 │ Not Found (unknown UUID)                                │
   │  409 │ Conflict (duplicate name, already subscribed, etc.)     │
   │  500 │ Internal Server Error                                   │
   └──────┴─────────────────────────────────────────────────────────┘

9. Error Handling

   9.1  A client that sends a malformed frame (bad encoding, missing
        required field, frame too long) MUST receive an ERR code=400
        response. The session remains open.

   9.2  If the server encounters an internal error processing a
        command, it MUST send ERR code=500 and keep the session open.

   9.3  If the TCP connection drops unexpectedly, the server MUST mark
        the associated user as offline and broadcast USER_OFFLINE to
        all connected clients.

   9.4  The client SHOULD reconnect and re-send LOG on any connection
        drop. The server will resume the existing user session.

10. Session Lifecycle

        CLIENT                          SERVER
          |                               |
          |----[ TCP connect ]----------->|
          |                               |
          |-- LOG name=alice ------------>|  (1) Create / resume user
          |<- OK uuid=... logged_in=1 ----|
          |                               |
          |<- EVT USER_ONLINE uuid=alice--|  (2) Broadcast to others
          |                               |
          |-- TSB team=<uuid> ----------->|  (3) Subscribe
          |<- OK team=<uuid> -------------|
          |                               |
          |-- HCR title=... body=... ---->|  (4) Create thread
          |<- OK uuid=<thread_uuid> ------|
          |                               |
          |<- EVT THREAD_CREATED ... -----|  (5) Sent to all subscribers
          |                               |
          |-- OUT ----------------------->|  (6) Logout
          |<- OK --------------------------|
          |                               |
          |<- EVT USER_OFFLINE uuid=alice-|  (7) Broadcast to others
          |                               |
          |----[ TCP close ]------------->|

11. Security Considerations

   11.1  There is no password-based authentication. A user is
         identified by their name. The server MUST ensure that two
         concurrent connections cannot claim the same username
         simultaneously (respond ERR code=409 to the second).

   11.2  A client that has not sent a valid LOG command MUST NOT
         receive any information about users, teams, channels, or
         messages. All such commands MUST return ERR code=401.

   11.3  A user MUST NOT receive events for teams they are not
         subscribed to (no channel/thread/comment events).

   11.4  A user that is not subscribed to a team MUST NOT be able to
         create channels, threads, or comments within that team.

   11.5  MTAP does not specify TLS. Operators deploying this service
         on untrusted networks SHOULD wrap connections in TLS via a
         proxy.

12. Example Exchanges

12.1  Successful login and team creation:

   C: LOG name=alice\n
   S: OK uuid=550e8400-e29b-41d4-a716-446655440000 name=alice logged_in=1\n
   S: EVT event=USER_ONLINE uuid=550e8400-e29b-41d4-a716-446655440000 name=alice\n

   C: TCR name=Engineering description=Engineering%20team\n
   S: OK uuid=6ba7b810-9dad-11d1-80b4-00c04fd430c8 name=Engineering
      description=Engineering%20team
      creator=550e8400-e29b-41d4-a716-446655440000\n

12.2  Setting context and creating a thread:

   C: USE team=6ba7b810-9dad-11d1-80b4-00c04fd430c8
         channel=6ba7b811-9dad-11d1-80b4-00c04fd430c8\n
   S: OK team=6ba7b810-9dad-11d1-80b4-00c04fd430c8
         channel=6ba7b811-9dad-11d1-80b4-00c04fd430c8 thread=-\n

   C: HCR title=Hello body=First%20post\n
   S: OK uuid=6ba7b812-9dad-11d1-80b4-00c04fd430c8
         title=Hello body=First%20post
         channel=6ba7b811-9dad-11d1-80b4-00c04fd430c8
         creator=550e8400-e29b-41d4-a716-446655440000
         timestamp=1743811200\n

12.3  Receiving an event while idle:

   S: EVT event=MSG_RECEIVED
         uuid=6ba7b813-9dad-11d1-80b4-00c04fd430c8
         from=6ba7b814-9dad-11d1-80b4-00c04fd430c8
         body=Hey%20Alice!
         timestamp=1743811260\n

12.4  Error - command before login:

   C: ULS\n
   S: ERR code=401 reason=not_logged_in\n

12.5  Error - body too long:

   C: SND to=<uuid> body=<513-character-string>\n
   S: ERR code=400 reason=body_too_long\n

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Authors

   MyTeams Project Team
   April 2026

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
End of RFC-0001
