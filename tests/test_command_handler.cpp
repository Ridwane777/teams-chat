#include "../include/server/CommandHandler/CommandHandler.hpp"
#include "test_helpers.hpp"

static std::string read_from_fd(int fd)
{
    char buffer[8192] = {0};
    ssize_t n = read(fd, buffer, sizeof(buffer) - 1);
    if (n <= 0)
        return "";
    return std::string(buffer, static_cast<size_t>(n));
}

static std::string extract_uuid(const std::string &payload)
{
    size_t pos = payload.find("uuid=");
    while (pos != std::string::npos) {
        if (pos + 41 <= payload.size()) {
            std::string candidate = payload.substr(pos + 5, 36);
            if (candidate.size() == 36)
                return candidate;
        }
        pos = payload.find("uuid=", pos + 1);
    }
    return "";
}

TEST_CASE(command_handler_auth_and_basic_flow)
{
    int a[2];
    int b[2];
    pipe(a);
    pipe(b);

    CommandHandler h;
    h.register_client_fd(a[1]);
    h.register_client_fd(b[1]);

    h.handle_command(a[1], "ULS");
    std::string unauthorized = read_from_fd(a[0]);
    ASSERT_TRUE(unauthorized.find("code=401") != std::string::npos);

    h.handle_command(a[1], "LOG name=alice");
    ASSERT_TRUE(read_from_fd(a[0]).find("OK") != std::string::npos);

    h.handle_command(b[1], "LOG name=bob");
    ASSERT_TRUE(read_from_fd(b[0]).find("OK") != std::string::npos);

    h.handle_command(a[1], "TCR name=t description=d");
    std::string teamResp = read_from_fd(a[0]);
    ASSERT_TRUE(teamResp.find("creator=") != std::string::npos);

    h.handle_command(a[1], "USE");
    read_from_fd(a[0]);
    h.handle_command(a[1], "LST");
    std::string listResp = read_from_fd(a[0]);
    std::string teamUuid = extract_uuid(listResp);
    ASSERT_EQ(teamUuid.size(), static_cast<size_t>(36));

    h.handle_command(b[1], std::string("TSB team=") + teamUuid);
    std::string subResp = read_from_fd(b[0]);
    ASSERT_TRUE(subResp.find("code=404") == std::string::npos);

    h.handle_command(a[1], std::string("USE team=") + teamUuid);
    std::string useTeamResp = read_from_fd(a[0]);
    ASSERT_TRUE(useTeamResp.find("OK") != std::string::npos);
    ASSERT_TRUE(useTeamResp.find("team=") != std::string::npos);

    h.handle_command(a[1], "CCR name=general description=main");
    std::string chResp = read_from_fd(a[0]);
    ASSERT_TRUE(chResp.find("team=") != std::string::npos);

    std::string channelUuid = extract_uuid(chResp);

    h.handle_command(a[1], std::string("USE team=") + teamUuid + " channel=" + channelUuid);
    ASSERT_TRUE(read_from_fd(a[0]).find("channel=") != std::string::npos);

    h.handle_command(a[1], "HCR title=t body=b");
    std::string thResp = read_from_fd(a[0]);
    ASSERT_TRUE(thResp.find("timestamp=") != std::string::npos);

    std::string threadUuid = extract_uuid(thResp);

    h.handle_command(a[1], std::string("USE team=") + teamUuid + " channel=" + channelUuid + " thread=" + threadUuid);
    ASSERT_TRUE(read_from_fd(a[0]).find("thread=") != std::string::npos);

    h.handle_command(a[1], "OCR body=reply");
    ASSERT_TRUE(read_from_fd(a[0]).find("author=") != std::string::npos);

    h.handle_command(a[1], "LST");
    ASSERT_TRUE(read_from_fd(a[0]).find("count=") != std::string::npos);

    h.handle_command(a[1], "INF");
    ASSERT_TRUE(read_from_fd(a[0]).find("title=") != std::string::npos);

    h.handle_command(a[1], "OUT");
    ASSERT_TRUE(read_from_fd(a[0]).find("OK") != std::string::npos);

    h.unregister_client_fd(a[1]);
    h.unregister_client_fd(b[1]);
    close(a[0]); close(a[1]); close(b[0]); close(b[1]);
}

TEST_CASE(command_handler_invalid_command_paths)
{
    int a[2];
    pipe(a);
    CommandHandler h;
    h.register_client_fd(a[1]);

    h.handle_command(a[1], "bad command");
    ASSERT_TRUE(read_from_fd(a[0]).find("code=400") != std::string::npos);

    Frame f;
    f.opcode = "XXX";
    h.execute_command(a[1], f);
    ASSERT_TRUE(read_from_fd(a[0]).find("unknown_command") != std::string::npos);

    h.handle_command(a[1], "LOG name=");
    ASSERT_TRUE(read_from_fd(a[0]).find("invalid_name") != std::string::npos);

    h.unregister_client_fd(a[1]);
    close(a[0]);
    close(a[1]);
}

TEST_CASE(command_handler_message_error_paths)
{
    int a[2];
    pipe(a);
    CommandHandler h;
    h.register_client_fd(a[1]);
    h.handle_command(a[1], "LOG name=alice");
    std::string login = read_from_fd(a[0]);
    std::string self = extract_uuid(login);

    h.handle_command(a[1], std::string("SND to=") + self + " body=hello");
    ASSERT_TRUE(read_from_fd(a[0]).find("cannot_message_self") != std::string::npos);

    h.handle_command(a[1], "SND body=hello");
    ASSERT_TRUE(read_from_fd(a[0]).find("missing_to") != std::string::npos);

    h.handle_command(a[1], "SND to=not-a-uuid body=hello");
    ASSERT_TRUE(read_from_fd(a[0]).find("invalid_uuid") != std::string::npos);

    h.handle_command(a[1], "SND to=11111111-1111-1111-1111-111111111111 body=hello");
    ASSERT_TRUE(read_from_fd(a[0]).find("user_not_found") != std::string::npos);

    h.handle_command(a[1], "MLS with=not-a-uuid");
    ASSERT_TRUE(read_from_fd(a[0]).find("invalid_uuid") != std::string::npos);

    h.handle_command(a[1], "MLS with=11111111-1111-1111-1111-111111111111");
    ASSERT_TRUE(read_from_fd(a[0]).find("user_not_found") != std::string::npos);

    h.unregister_client_fd(a[1]);
    close(a[0]);
    close(a[1]);
}

TEST_CASE(command_handler_team_and_context_error_paths)
{
    int a[2];
    pipe(a);
    CommandHandler h;
    h.register_client_fd(a[1]);
    h.handle_command(a[1], "LOG name=alice");
    read_from_fd(a[0]);

    h.handle_command(a[1], "TCR name=");
    ASSERT_TRUE(read_from_fd(a[0]).find("name_too_long") != std::string::npos);

    h.handle_command(a[1], "TCR name=team description=d");
    std::string created = read_from_fd(a[0]);
    std::string team = extract_uuid(created);
    ASSERT_EQ(team.size(), static_cast<size_t>(36));

    h.handle_command(a[1], "TCR name=team description=d2");
    ASSERT_TRUE(read_from_fd(a[0]).find("team_exists") != std::string::npos);

    h.handle_command(a[1], "TSB");
    ASSERT_TRUE(read_from_fd(a[0]).find("missing_team") != std::string::npos);

    h.handle_command(a[1], "TUS team=11111111-1111-1111-1111-111111111111");
    ASSERT_TRUE(read_from_fd(a[0]).find("team_not_found") != std::string::npos);

    h.handle_command(a[1], "USE channel=bad");
    ASSERT_TRUE(read_from_fd(a[0]).find("invalid_context") != std::string::npos);

    h.handle_command(a[1], "USE team=not-a-uuid");
    ASSERT_TRUE(read_from_fd(a[0]).find("team_not_found") != std::string::npos);

    h.unregister_client_fd(a[1]);
    close(a[0]);
    close(a[1]);
}

TEST_CASE(command_handler_subscription_list_and_info_paths)
{
    int a[2];
    int b[2];
    pipe(a);
    pipe(b);
    CommandHandler h;
    h.register_client_fd(a[1]);
    h.register_client_fd(b[1]);

    h.handle_command(a[1], "LOG name=alice");
    std::string aliceLogin = read_from_fd(a[0]);
    std::string aliceUuid = extract_uuid(aliceLogin);
    h.handle_command(b[1], "LOG name=bob");
    std::string bobLogin = read_from_fd(b[0]);
    std::string bobUuid = extract_uuid(bobLogin);

    h.handle_command(a[1], "TCR name=teamx description=desc");
    read_from_fd(a[0]);
    h.handle_command(a[1], "USE");
    read_from_fd(a[0]);
    h.handle_command(a[1], "LST");
    std::string teamUuid = extract_uuid(read_from_fd(a[0]));

    h.handle_command(a[1], std::string("TSB team=") + teamUuid);
    ASSERT_TRUE(read_from_fd(a[0]).find("team_not_found") == std::string::npos);
    h.handle_command(b[1], std::string("TSB team=") + teamUuid);
    ASSERT_TRUE(read_from_fd(b[0]).find("OK") != std::string::npos);

    h.handle_command(a[1], "TSD");
    ASSERT_TRUE(read_from_fd(a[0]).find("count=") != std::string::npos);
    h.handle_command(a[1], std::string("TSD team=") + teamUuid);
    std::string members = read_from_fd(a[0]);
    ASSERT_TRUE(members.find(aliceUuid) != std::string::npos);
    ASSERT_TRUE(members.find(bobUuid) != std::string::npos);

    h.handle_command(b[1], std::string("TUS team=") + teamUuid);
    ASSERT_TRUE(read_from_fd(b[0]).find("OK") != std::string::npos);
    h.handle_command(b[1], std::string("TUS team=") + teamUuid);
    ASSERT_TRUE(read_from_fd(b[0]).find("not_subscribed") != std::string::npos);

    h.handle_command(a[1], "INF");
    ASSERT_TRUE(read_from_fd(a[0]).find("name=alice") != std::string::npos);

    h.unregister_client_fd(a[1]);
    h.unregister_client_fd(b[1]);
    close(a[0]); close(a[1]); close(b[0]); close(b[1]);
}

TEST_CASE(command_handler_channel_thread_comment_error_paths)
{
    int a[2];
    pipe(a);
    CommandHandler h;
    h.register_client_fd(a[1]);
    h.handle_command(a[1], "LOG name=alice");
    read_from_fd(a[0]);

    h.handle_command(a[1], "CCR name=n description=d");
    ASSERT_TRUE(read_from_fd(a[0]).find("context_required") != std::string::npos);
    h.handle_command(a[1], "HCR title=t body=b");
    ASSERT_TRUE(read_from_fd(a[0]).find("context_required") != std::string::npos);
    h.handle_command(a[1], "OCR body=b");
    ASSERT_TRUE(read_from_fd(a[0]).find("context_required") != std::string::npos);

    h.handle_command(a[1], "TCR name=team description=d");
    std::string teamResp = read_from_fd(a[0]);
    std::string teamUuid = extract_uuid(teamResp);
    h.handle_command(a[1], std::string("USE team=") + teamUuid);
    read_from_fd(a[0]);
    h.handle_command(a[1], "CCR name=chan description=d");
    std::string chanResp = read_from_fd(a[0]);
    std::string chanUuid = extract_uuid(chanResp);

    h.handle_command(a[1], "CCR name=chan description=d");
    ASSERT_TRUE(read_from_fd(a[0]).find("channel_exists") != std::string::npos);

    h.handle_command(a[1], std::string("USE team=") + teamUuid + " channel=" + chanUuid);
    read_from_fd(a[0]);
    h.handle_command(a[1], "HCR title=t body=b");
    std::string thResp = read_from_fd(a[0]);
    std::string thUuid = extract_uuid(thResp);

    h.handle_command(a[1], std::string("USE team=") + teamUuid + " channel=" + chanUuid + " thread=" + thUuid);
    read_from_fd(a[0]);
    h.handle_command(a[1], "OCR body=reply");
    ASSERT_TRUE(read_from_fd(a[0]).find("author=") != std::string::npos);

    h.handle_command(a[1], "USE team=not-a-uuid channel=bad thread=bad");
    ASSERT_TRUE(read_from_fd(a[0]).find("team_not_found") != std::string::npos);

    h.unregister_client_fd(a[1]);
    close(a[0]);
    close(a[1]);
}
