#include "../include/client/ClientCommand.hpp"
#include "test_helpers.hpp"
#include <fcntl.h>

static std::string exec_and_capture(const std::string &command, bool *ok = nullptr)
{
    int pipefd[2];
    pipe(pipefd);
    bool result = UserCommand::execute(pipefd[1], command);
    close(pipefd[1]);
    std::string out = read_all_from_fd(pipefd[0]);
    close(pipefd[0]);
    if (ok)
        *ok = result;
    return out;
}

TEST_CASE(client_command_unknown_and_empty)
{
    bool ok = true;
    exec_and_capture("   ", &ok);
    ASSERT_TRUE(ok);
    exec_and_capture("/unknown", &ok);
    ASSERT_TRUE(!ok);
}

TEST_CASE(client_command_basic_commands_write_expected_opcodes)
{
    ASSERT_EQ(exec_and_capture("/logout"), "OUT\n");
    ASSERT_EQ(exec_and_capture("/users"), "ULS\n");
    ASSERT_EQ(exec_and_capture("/messages user-id"), "MLS with=user-id\n");
    ASSERT_EQ(exec_and_capture("/subscribe team-id"), "TSB team=team-id\n");
    ASSERT_EQ(exec_and_capture("/unsubscribe team-id"), "TUS team=team-id\n");
}

TEST_CASE(client_command_contextual_commands)
{
    ASSERT_EQ(exec_and_capture("/use team1"), "USE team=team1\n");
    ASSERT_EQ(exec_and_capture("/list"), "LST\n");
    ASSERT_EQ(exec_and_capture("/info"), "INF\n");
    ASSERT_EQ(exec_and_capture("/create name desc"), "CCR name=name description=desc\n");
    ASSERT_EQ(exec_and_capture("/use team1 chan1"), "USE team=team1 channel=chan1\n");
    ASSERT_EQ(exec_and_capture("/create title body"), "HCR title=title body=body\n");
    ASSERT_EQ(exec_and_capture("/use team1 chan1 th1"), "USE team=team1 channel=chan1 thread=th1\n");
    ASSERT_EQ(exec_and_capture("/create reply"), "OCR body=reply\n");
}

TEST_CASE(client_command_additional_commands_and_argument_validation)
{
    ASSERT_EQ(exec_and_capture("/help"), "");
    ASSERT_EQ(exec_and_capture("/login john"), "LOG name=john\n");
    ASSERT_EQ(exec_and_capture("/user user-id"), "UGT uuid=user-id\n");
    ASSERT_EQ(exec_and_capture("/send user-id hello"), "SND to=user-id body=hello\n");
    ASSERT_EQ(exec_and_capture("/subscribed"), "TSD\n");
    ASSERT_EQ(exec_and_capture("/subscribed team-id"), "TSD team=team-id\n");

    ASSERT_EQ(exec_and_capture("/login"), "");
    ASSERT_EQ(exec_and_capture("/user"), "");
    ASSERT_EQ(exec_and_capture("/send user-only"), "");
    ASSERT_EQ(exec_and_capture("/messages"), "");
    ASSERT_EQ(exec_and_capture("/subscribe"), "");
    ASSERT_EQ(exec_and_capture("/unsubscribe"), "");
}
