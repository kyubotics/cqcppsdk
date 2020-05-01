
#define CATCH_CONFIG_MAIN
#include "cqcppsdk/cqcppsdk.hpp"

#include "catch.hpp"

TEST_CASE("MessageSegment ctor", "[message segment]") {
    using cq::message::MessageSegment;
    using seg_types = MessageSegment::SegTypes;

    ::std::string src = "test";
    MessageSegment seg = MessageSegment::text(src);
    MessageSegment seg1 = seg;
    seg = std::move(seg1);
    REQUIRE(seg.type() == seg_types::text);
    REQUIRE(::std::string(seg) == src);

    src = "[CQ:emoji,id=127838]";
    seg = MessageSegment::fromCQCode(src);
    seg1 = seg;
    seg = std::move(seg1);
    REQUIRE(seg.type() == seg_types::emoji);
    REQUIRE((seg.value_map().at("id") == ::std::string("127838")));
    REQUIRE(::std::string(seg) == src);
    seg = MessageSegment::emoji(127838);
    REQUIRE(seg.type() == seg_types::emoji);
    REQUIRE((seg.value_map().at("id") == ::std::string("127838")));
    REQUIRE(::std::string(seg) == src);

    src = "[CQ:face,id=180]";
    seg = MessageSegment::fromCQCode(src);
    seg1 = seg;
    seg = std::move(seg1);
    REQUIRE(seg.type() == seg_types::face);
    REQUIRE((seg.value_map().at("id") == ::std::string("180")));
    REQUIRE(::std::string(seg) == src);
    seg = MessageSegment::face(180);
    REQUIRE(seg.type() == seg_types::face);
    REQUIRE((seg.value_map().at("id") == ::std::string("180")));
    REQUIRE(::std::string(seg) == src);

    src = "[CQ:image,file=AE9CE0B5A1FBA37F95718FDC547945CC.jpg]";
    seg = MessageSegment::fromCQCode(src);
    seg1 = seg;
    seg = std::move(seg1);
    REQUIRE(seg.type() == seg_types::image);
    REQUIRE(seg.value_map().at("file") == "AE9CE0B5A1FBA37F95718FDC547945CC.jpg");
    REQUIRE(::std::string(seg) == src);
    seg = MessageSegment::image("AE9CE0B5A1FBA37F95718FDC547945CC.jpg");
    REQUIRE(seg.type() == seg_types::image);
    REQUIRE(::std::string(seg) == src);

    src = "[CQ:record,file=C5B0B7D8A515F1F652B0DEBB49335B9E.silk]";
    seg = MessageSegment::fromCQCode(src);
    seg1 = seg;
    seg = std::move(seg1);
    REQUIRE(seg.type() == seg_types::record);
    REQUIRE(seg.value_map().at("file") == "C5B0B7D8A515F1F652B0DEBB49335B9E.silk");
    REQUIRE(::std::string(seg) == src);
    seg = MessageSegment::record("C5B0B7D8A515F1F652B0DEBB49335B9E.silk", true);
    REQUIRE(seg.type() == seg_types::record);
    REQUIRE(::std::string(seg) == "[CQ:record,file=C5B0B7D8A515F1F652B0DEBB49335B9E.silk,magic=true]");
}
TEST_CASE("Message ctor", "[message]") {
    ::std::string src =
        "[CQ:emoji,id=127838]"
        "test"
        "[CQ:face,id=180]"
        "[CQ:image,file=AE9CE0B5A1FBA37F95718FDC547945CC.jpg]"
        "[CQ:bface,p=204112,id=2DD591BFD449F584C709D276EC472E55]";

    cq::message::Message msg;
    msg = src;
    REQUIRE(msg.size() == 5);

    using seg_types = cq::message::MessageSegment::SegTypes;
    auto i = msg.begin();
    REQUIRE(i->type() == seg_types::emoji);
    REQUIRE((i->value_map().at("id") == ::std::string("127838")));
    std::advance(i, 1);
    REQUIRE(i->type() == seg_types::text);
    REQUIRE(i->plain_text() == "test");
    std::advance(i, 1);
    REQUIRE(i->type() == seg_types::face);
    REQUIRE(i->value_map().at("id") == "180");
    std::advance(i, 1);
    REQUIRE(i->type() == seg_types::image);
    REQUIRE(i->value_map().at("file") == "AE9CE0B5A1FBA37F95718FDC547945CC.jpg");
    std::advance(i, 1);
    REQUIRE(i->type() == seg_types::unimpl);
    REQUIRE(i->plain_text() == "[CQ:bface,p=204112,id=2DD591BFD449F584C709D276EC472E55]");
}

TEST_CASE("Message reduce and concat", "[message]") {
    using namespace cq::message;
    cq::message::Message msg;
    msg.push_back(MessageSegment::text("1"));
    msg.push_back(MessageSegment::text("2"));
    msg.push_back(MessageSegment::text("34"));
    msg.push_back(MessageSegment::text("56"));
    msg.push_back(MessageSegment::text("78"));
    msg.push_back(MessageSegment::text("9"));
    REQUIRE(msg.size() == 6);
    msg.reduce();
    REQUIRE(msg.size() == 1);
    REQUIRE(msg.extract_plain_text() == "123456789");

    msg.push_back(MessageSegment::text("A"));
    msg.push_back(MessageSegment::text("BC"));
    msg.push_back(MessageSegment::face(180));
    msg.push_back(MessageSegment::text("D"));
    msg.push_back(MessageSegment::text("EF"));
    REQUIRE(msg.size() == 6);
    msg.reduce();
    REQUIRE(msg.size() == 3);
    REQUIRE(msg.extract_plain_text() == "123456789ABCDEF");

    msg.push_back(MessageSegment::text("GH"));
    msg.push_back(MessageSegment::face(180));
    REQUIRE(msg.size() == 5);
    msg.reduce();
    REQUIRE(msg.size() == 4);
    REQUIRE(msg.extract_plain_text() == "123456789ABCDEFGH");

    msg += "IJK";
    REQUIRE(msg.size() == 5);
    REQUIRE(msg.extract_plain_text() == "123456789ABCDEFGHIJK");

    msg += "LMN";
    REQUIRE(msg.size() == 5);
    REQUIRE(msg.extract_plain_text() == "123456789ABCDEFGHIJKLMN");

    cq::message::Message msg2 =
        "[CQ:emoji,id=127838]"
        "test"
        "[CQ:face,id=180]"
        "[CQ:image,file=AE9CE0B5A1FBA37F95718FDC547945CC.jpg]"
        "[CQ:bface,p=204112,id=2DD591BFD449F584C709D276EC472E55]";
    REQUIRE(msg2.size() == 5);
    msg += msg2;
    REQUIRE(msg.size() == 10);
    REQUIRE(msg.extract_plain_text() == "123456789ABCDEFGHIJKLMNtest");
}

TEST_CASE("Message equal", "[message]") {
    using namespace cq::message;
    ::std::string src =
        "[CQ:emoji,id=127838]"
        "test"
        "[CQ:face,id=180]"
        "[CQ:image,file=AE9CE0B5A1FBA37F95718FDC547945CC.jpg]"
        "[CQ:bface,p=204112,id=2DD591BFD449F584C709D276EC472E55]";
    cq::message::Message msg1 = src;
    cq::message::Message msg2 = src;
    REQUIRE(msg1 == msg2);
    msg2.pop_back();
    REQUIRE(msg1 != msg2);
    msg2.push_back(MessageSegment::face(180));
    REQUIRE(msg1 != msg2);
    msg2.pop_back();
    REQUIRE(msg1 != msg2);
    msg2.push_back(MessageSegment::fromCQCode("[CQ:bface,p=204112,id=2DD591BFD449F584C709D276EC472E55]"));
    REQUIRE(msg1 == msg2);
}