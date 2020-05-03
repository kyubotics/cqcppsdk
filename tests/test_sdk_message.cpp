
#define CATCH_CONFIG_MAIN
#include "cqcppsdk/cqcppsdk.hpp"

#include "catch.hpp"

TEST_CASE("MessageSegment ctor", "[message segment]") {
    using cq::message::MessageSegment;
    using seg_types = MessageSegment::SegTypes;

    SECTION("default") {
        SECTION("text") {
            ::std::string src = "test";

            MessageSegment seg = MessageSegment::text(src);
            REQUIRE(seg.type() == seg_types::text);
            REQUIRE(::std::string(seg) == src);

            MessageSegment seg2 = seg;
            REQUIRE(seg2.type() == seg_types::text);
            REQUIRE(::std::string(seg2) == src);

            MessageSegment seg3 = ::std::move(seg2);
            REQUIRE(seg3.type() == seg_types::text);
            REQUIRE(::std::string(seg3) == src);
        }

        SECTION("not text") {
            ::std::string src = "[CQ:emoji,id=127838]";

            MessageSegment seg = MessageSegment::emoji(127838);
            REQUIRE(seg.type() == seg_types::emoji);
            REQUIRE(::std::string(seg) == src);

            MessageSegment seg2 = seg;
            REQUIRE(seg2.type() == seg_types::emoji);
            REQUIRE(::std::string(seg2) == src);

            MessageSegment seg3 = ::std::move(seg2);
            REQUIRE(seg3.type() == seg_types::emoji);
            REQUIRE(::std::string(seg3) == src);
        }

        SECTION("unimpl") {
            ::std::string src = "[CQ:bface,p=204112,id=2DD591BFD449F584C709D276EC472E55]";

            MessageSegment seg = MessageSegment::fromCQCode(src);
            REQUIRE(seg.type() == seg_types::unimpl);
            REQUIRE(::std::string(seg) == src);

            MessageSegment seg2 = seg;
            REQUIRE(seg2.type() == seg_types::unimpl);
            REQUIRE(::std::string(seg2) == src);

            MessageSegment seg3 = ::std::move(seg2);
            REQUIRE(seg3.type() == seg_types::unimpl);
            REQUIRE(::std::string(seg3) == src);
        }
    }

    SECTION("text") {
        ::std::string src = "test";
        MessageSegment seg = MessageSegment::text(src);
        REQUIRE(seg.type() == seg_types::text);
        REQUIRE(::std::string(seg) == src);
    }

    SECTION("emoji") {
        ::std::string src = "[CQ:emoji,id=127838]";

        MessageSegment seg = MessageSegment::fromCQCode(src);
        REQUIRE(seg.type() == seg_types::emoji);
        REQUIRE((seg.value_map().at("id") == ::std::string("127838")));
        REQUIRE(::std::string(seg) == src);

        seg = MessageSegment::emoji(127838);
        REQUIRE(seg.type() == seg_types::emoji);
        REQUIRE((seg.value_map().at("id") == ::std::string("127838")));
        REQUIRE(::std::string(seg) == src);
    }

    SECTION("face") {
        ::std::string src = "[CQ:face,id=180]";

        MessageSegment seg = MessageSegment::fromCQCode(src);
        REQUIRE(seg.type() == seg_types::face);
        REQUIRE((seg.value_map().at("id") == ::std::string("180")));
        REQUIRE(::std::string(seg) == src);

        seg = MessageSegment::face(180);
        REQUIRE(seg.type() == seg_types::face);
        REQUIRE((seg.value_map().at("id") == ::std::string("180")));
        REQUIRE(::std::string(seg) == src);
    }

    SECTION("image") {
        ::std::string src = "[CQ:image,file=AE9CE0B5A1FBA37F95718FDC547945CC.jpg]";

        MessageSegment seg = MessageSegment::fromCQCode(src);
        REQUIRE(seg.type() == seg_types::image);
        REQUIRE(seg.value_map().at("file") == "AE9CE0B5A1FBA37F95718FDC547945CC.jpg");
        REQUIRE(::std::string(seg) == src);

        seg = MessageSegment::image("AE9CE0B5A1FBA37F95718FDC547945CC.jpg");
        REQUIRE(seg.type() == seg_types::image);
        REQUIRE(::std::string(seg) == src);
    }

    SECTION("record") {
        ::std::string src = "[CQ:record,file=C5B0B7D8A515F1F652B0DEBB49335B9E.silk]";
        MessageSegment seg = MessageSegment::fromCQCode(src);
        REQUIRE(seg.type() == seg_types::record);
        REQUIRE(seg.value_map().at("file") == "C5B0B7D8A515F1F652B0DEBB49335B9E.silk");
        REQUIRE(::std::string(seg) == src);

        seg = MessageSegment::record("C5B0B7D8A515F1F652B0DEBB49335B9E.silk", true);
        REQUIRE(seg.type() == seg_types::record);
        REQUIRE(::std::string(seg) == "[CQ:record,file=C5B0B7D8A515F1F652B0DEBB49335B9E.silk,magic=true]");
    }
}

TEST_CASE("MessageSegment equal", "[message segment]") {
    using cq::message::MessageSegment;
    using seg_types = MessageSegment::SegTypes;

    REQUIRE(MessageSegment::text("test") == MessageSegment::text("test"));
    REQUIRE(MessageSegment::text("test") != MessageSegment::text("text"));
    REQUIRE(MessageSegment::text("test") != MessageSegment::emoji(127838));
    REQUIRE(MessageSegment::emoji(127838) == MessageSegment::fromCQCode("[CQ:emoji,id=127838]"));
}

TEST_CASE("MessageSegment bad construction", "[message segment]") {
    using cq::message::MessageSegment;
    ::std::string src = "[CQ:record,file=C5B0B7D8A515F1F652B0DEBB49335B9E.silk";
    try {
        MessageSegment seg = MessageSegment::fromCQCode(src);
    } catch (std::exception& err) {
        REQUIRE(::std::string(err.what()) == "Invalid CQCode");
    }
}

TEST_CASE("Message ctor", "[message]") {
    using namespace cq::message;
    using seg_types = cq::message::MessageSegment::SegTypes;

    SECTION("from string source") {
        ::std::string src =
            "[CQ:emoji,id=127838]"
            "test"
            "[CQ:face,id=180]"
            "[CQ:image,file=AE9CE0B5A1FBA37F95718FDC547945CC.jpg]"
            "[CQ:bface,p=204112,id=2DD591BFD449F584C709D276EC472E55]";

        cq::message::Message msg;
        msg = src;
        REQUIRE(msg.size() == 5);
        REQUIRE(::std::string(msg) == src);

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

    SECTION("from MessageSegment") {
        Message msg = MessageSegment::emoji(127838);
        msg += MessageSegment::text("test");
        REQUIRE(msg.size() == 2);

        auto i = msg.begin();
        REQUIRE(i->type() == seg_types::emoji);
        REQUIRE(i->value_map().at("id") == "127838");
        std::advance(i, 1);
        REQUIRE(i->type() == seg_types::text);
        REQUIRE(i->plain_text() == "test");
    }

    SECTION("from Message") {
        Message msg = MessageSegment::emoji(127838);
        REQUIRE(msg.size() == 1);
        REQUIRE(msg.front().type() == seg_types::emoji);
        REQUIRE(msg.front().value_map().at("id") == "127838");

        SECTION("copy ctor") {
            Message msg2 = msg;
            REQUIRE(msg2.size() == 1);
            REQUIRE(msg2.front().type() == seg_types::emoji);
            REQUIRE(msg2.front().value_map().at("id") == "127838");
        }

        SECTION("move ctor") {
            Message msg2 = msg;
            Message msg3 = ::std::move(msg2);
            REQUIRE(msg3.size() == 1);
            REQUIRE(msg3.front().type() == seg_types::emoji);
            REQUIRE(msg3.front().value_map().at("id") == "127838");
        }
    }
}

TEST_CASE("Message reduce", "[message]") {
    using namespace cq::message;

    SECTION("Message reduce all text") {
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
    }

    SECTION("Message reduce ends with text") {
        cq::message::Message msg;
        msg.push_back(MessageSegment::text("ABC"));
        msg.push_back(MessageSegment::face(180));
        msg.push_back(MessageSegment::text("D"));
        msg.push_back(MessageSegment::text("EF"));
        REQUIRE(msg.size() == 4);
        msg.reduce();
        REQUIRE(msg.size() == 3);
        REQUIRE(msg.extract_plain_text() == "ABCDEF");
    }

    SECTION("Message reduce ends with non-text") {
        cq::message::Message msg;
        msg.push_back(MessageSegment::text("ABC"));
        msg.push_back(MessageSegment::text("D"));
        msg.push_back(MessageSegment::dice());
        msg.push_back(MessageSegment::text("EF"));
        msg.push_back(MessageSegment::face(180));
        REQUIRE(msg.size() == 5);
        msg.reduce();
        REQUIRE(msg.size() == 4);
        REQUIRE(msg.extract_plain_text() == "ABCDEF");
    }
}

TEST_CASE("Message concat", "[message]") {
    using namespace cq::message;
    SECTION("concat string literal") {
        SECTION("with text reduce") {
            cq::message::Message msg =
                "ABC"
                "[CQ:emoji,id=127838]"
                "[CQ:shake]"
                "DEF";
            REQUIRE(msg.size() == 4);
            REQUIRE(msg.extract_plain_text() == "ABCDEF");
            msg += "GHIJK";
            REQUIRE(msg.size() == 4);
            REQUIRE(msg.extract_plain_text() == "ABCDEFGHIJK");
        }

        SECTION("no text reduce") {
            cq::message::Message msg =
                "ABCDEF"
                "[CQ:emoji,id=127838]"
                "[CQ:shake]";
            REQUIRE(msg.size() == 3);
            REQUIRE(msg.extract_plain_text() == "ABCDEF");
            msg += "GHIJK";
            REQUIRE(msg.size() == 4);
            REQUIRE(msg.extract_plain_text() == "ABCDEFGHIJK");
        }
    }

    SECTION("string on the left") {
        SECTION("with text reduce") {
            Message msg1 = "GHIJK";
            REQUIRE(msg1.size() == 1);
            REQUIRE(msg1.extract_plain_text() == "GHIJK");

            ::std::string src =
                "ABC"
                "[CQ:emoji,id=127838]"
                "[CQ:shake]"
                "DEF";

            Message msg = src + msg1;
            REQUIRE(msg.size() == 4);
            REQUIRE(msg.extract_plain_text() == "ABCDEFGHIJK");
        }

        SECTION("no text reduce") {
            Message msg1 = "GHIJK";
            REQUIRE(msg1.size() == 1);
            REQUIRE(msg1.extract_plain_text() == "GHIJK");

            ::std::string src =
                "ABCDEF"
                "[CQ:emoji,id=127838]"
                "[CQ:shake]";

            Message msg = src + msg1;
            REQUIRE(msg.size() == 4);
            REQUIRE(msg.extract_plain_text() == "ABCDEFGHIJK");
        }
    }

    SECTION("concat MessageSegment") {
        SECTION("concat lvalue") {
            cq::message::Message msg =
                "ABC"
                "[CQ:emoji,id=127838]"
                "DEF";
            REQUIRE(msg.size() == 3);
            REQUIRE(msg.extract_plain_text() == "ABCDEF");

            auto seg1 = MessageSegment::text("GHIJK");
            msg += seg1;
            REQUIRE(msg.size() == 4);
            REQUIRE(msg.extract_plain_text() == "ABCDEFGHIJK");

            auto seg2 = MessageSegment::dice();
            msg += seg2;
            REQUIRE(msg.size() == 5);
            REQUIRE(msg.extract_plain_text() == "ABCDEFGHIJK");
        }

        SECTION("concat rvalue") {
            cq::message::Message msg =
                "ABC"
                "[CQ:emoji,id=127838]"
                "DEF";
            REQUIRE(msg.size() == 3);
            REQUIRE(msg.extract_plain_text() == "ABCDEF");

            msg += MessageSegment::text("GHIJK");
            REQUIRE(msg.size() == 4);
            REQUIRE(msg.extract_plain_text() == "ABCDEFGHIJK");

            msg += MessageSegment::dice();
            REQUIRE(msg.size() == 5);
            REQUIRE(msg.extract_plain_text() == "ABCDEFGHIJK");
        }
    }

    SECTION("MessageSegment on the left") {
        SECTION("concat string literal") {
            MessageSegment seg = MessageSegment::emoji(127838);
            REQUIRE(seg.type() == MessageSegment::SegTypes::emoji);
            REQUIRE(seg.value_map().at("id") == "127838");

            auto msg = seg + "ABCDEFGHIJK";
            REQUIRE(msg.size() == 2);
            REQUIRE(msg.extract_plain_text() == "ABCDEFGHIJK");
        }

        SECTION("concat MessageSegment") {
            MessageSegment seg = MessageSegment::emoji(127838);
            REQUIRE(seg.type() == MessageSegment::SegTypes::emoji);
            REQUIRE(seg.value_map().at("id") == "127838");

            auto msg = seg + MessageSegment::text("ABCDEFGHIJK");
            REQUIRE(msg.size() == 2);
            REQUIRE(msg.extract_plain_text() == "ABCDEFGHIJK");
        }
    }

    SECTION("concat Message") {
        cq::message::Message msg =
            "ABC"
            "[CQ:emoji,id=127838]"
            "DEF";
        REQUIRE(msg.size() == 3);
        REQUIRE(msg.extract_plain_text() == "ABCDEF");

        cq::message::Message msg2 =
            "[CQ:emoji,id=127838]"
            "GHIJKLMN"
            "[CQ:face,id=180]"
            "[CQ:image,file=AE9CE0B5A1FBA37F95718FDC547945CC.jpg]"
            "[CQ:bface,p=204112,id=2DD591BFD449F584C709D276EC472E55]";
        REQUIRE(msg2.size() == 5);
        REQUIRE(msg2.extract_plain_text() == "GHIJKLMN");

        msg += msg2;
        REQUIRE(msg.size() == 8);
        REQUIRE(msg.extract_plain_text() == "ABCDEFGHIJKLMN");
    }
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

TEST_CASE("Message bad construction", "[message]") {
    using namespace cq::message;
    ::std::string src =
        "[CQ:emoji,id=127838]"
        "test"
        "[CQ:face,id="
        "[CQ:image,file=AE9CE0B5A1FBA37F95718FDC547945CC.jpg]";

    cq::message::Message msg;

    try {
        msg = src;
    } catch (std::exception& err) {
        REQUIRE(::std::string(err.what()) == "Invalid CQCode");
        REQUIRE(msg.empty());
    }
}