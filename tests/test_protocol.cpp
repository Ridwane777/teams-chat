#include "../include/server/protocol/Protocol.hpp"
#include "test_helpers.hpp"

TEST_CASE(protocol_encode_decode_roundtrip)
{
    Protocol p;
    std::string raw = "hello world/42%";
    std::string encoded = p.encode(raw);
    ASSERT_TRUE(encoded.find("%20") != std::string::npos);
    ASSERT_EQ(p.decode(encoded), raw);
}

TEST_CASE(protocol_parse_and_serialise)
{
    Protocol p;
    auto f = p.parse("LOG name=john%20doe\n");
    ASSERT_TRUE(f.has_value());
    ASSERT_EQ(f->opcode, "LOG");
    ASSERT_EQ(f->get("name"), "john doe");

    Frame out;
    out.opcode = "OK";
    out.fields["name"] = "john doe";
    std::string serial = p.serialise(out);
    ASSERT_TRUE(serial.find("name=john%20doe") != std::string::npos);
}

TEST_CASE(protocol_parse_invalid)
{
    Protocol p;
    ASSERT_TRUE(!p.parse("bad name=ok").has_value());
    ASSERT_TRUE(!p.parse("TOOLONG name=ok").has_value());
    ASSERT_TRUE(!p.parse("LOG no_equal").has_value());
}

TEST_CASE(protocol_ok_err_evt)
{
    Protocol p;
    ASSERT_EQ(p.ok({}), "OK\n");
    std::string err = p.err(404, "missing");
    ASSERT_TRUE(err.find("ERR") != std::string::npos);
    ASSERT_TRUE(err.find("code=404") != std::string::npos);
    ASSERT_TRUE(err.find("reason=missing") != std::string::npos);
    std::string evt = p.evt("USER_ONLINE", {{"uuid", "u1"}});
    ASSERT_TRUE(evt.find("EVT") != std::string::npos);
    ASSERT_TRUE(evt.find("event=USER_ONLINE") != std::string::npos);
}
