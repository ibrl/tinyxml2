#include <gtest/gtest.h>
#include <assert.h>
#include <climits>
#include <charconv>
#include <iostream>
#include "../../src/XMLUtil.h"
#include "../../src/StrPair.h"

#define SEMICOLON ';'
#define TIXMLASSERT assert

unsigned long checkIntegerValueNew(const char* p)
{
    unsigned long ucs {0};
    if (p == nullptr) {
        return 0;
    }
    if (*(p+2) == 'x') {
        const char * start = p+3;
        if (start) {
            const char * endp  = strchr(start, SEMICOLON);
            if (endp) {
                auto [ptr, ec] { std::from_chars(start, endp, ucs, 16) };
                if (ec == std::errc()) {
                    return ucs;
                }
            }
        }
    }
    return 0;
}

unsigned long checkIntegerValue(const char* p )
{
    ptrdiff_t delta = 0;
    unsigned mult = 1;
    unsigned long ucs = 0;

    if ( *(p+2) == 'x' ) {
        // Hexadecimal.
        const char* q = p+3;
        if ( !(*q) ) {
            return 0;
        }

        q = strchr( q, SEMICOLON );

        if ( !q ) {
            return 0;
        }
        TIXMLASSERT( *q == SEMICOLON );

        delta = q-p;
        --q;

        while ( *q != 'x' ) {
            unsigned int digit = 0;

            if ( *q >= '0' && *q <= '9' ) {
                digit = *q - '0';
            }
            else if ( *q >= 'a' && *q <= 'f' ) {
                digit = *q - 'a' + 10;
            }
            else if ( *q >= 'A' && *q <= 'F' ) {
                digit = *q - 'A' + 10;
            }
            else {
                return 0;
            }
            TIXMLASSERT( digit < 16 );
            TIXMLASSERT( digit == 0 || mult <= UINT_MAX / digit );
            const unsigned int digitScaled = mult * digit;
            TIXMLASSERT( ucs <= ULONG_MAX - digitScaled );
            ucs += digitScaled;
            TIXMLASSERT( mult <= UINT_MAX / 16 );
            mult *= 16;
            --q;
        }
    }
    return ucs;
}


TEST(XmlUtilTest, CheckIntegerValueOrigTest) {
    char const* testString { "&#x4acD;dfjk"};
    unsigned long ucs = checkIntegerValue(testString);
    EXPECT_EQ(ucs, 19149);
    char const * testString2 { "&#x4L"};
    ucs = checkIntegerValue(testString2);
    EXPECT_EQ(ucs, 0);
}

TEST(XmlUtilTest, CheckIntegerValueRefactoredTest) {
    char const* testString { "&#x4acD;dfjk"};
    unsigned long ucs = checkIntegerValueNew(testString);
    EXPECT_EQ(ucs, 19149);
    char const * testString2 { "&#x4L"};
    ucs = checkIntegerValueNew(testString2);
    EXPECT_EQ(ucs, 0);
}

TEST(XmlUtilTest, CheckIntegerNullPtrTest) {
    char * testString = nullptr;
    unsigned long ucs = checkIntegerValueNew(testString);
    EXPECT_EQ(ucs, 0);
    char const * testString2 { "&4244L"};
    ucs = checkIntegerValueNew(testString2);
    EXPECT_EQ(ucs, 0);
}

TEST(XmlStrPairTest, CreationTest) {
    tinyxml2::StrPair strPair;
    const auto [start, end, flags] = strPair.GetState();
    EXPECT_EQ(start, nullptr);
    EXPECT_EQ(end, nullptr);
    EXPECT_EQ(flags, 0);
}


TEST(XmlStrPairTest, SetTest)
{
    char text[] { "1234567890"};
    tinyxml2::StrPair strPair;
    strPair.Set(text, text + strlen(text), tinyxml2::StrPair::NEEDS_ENTITY_PROCESSING);
    const auto [start, end, flags] = strPair.GetState();
    auto myFlags = flags & tinyxml2::StrPair::NEEDS_ENTITY_PROCESSING;
    EXPECT_EQ(start, text);
    EXPECT_EQ(end, text+strlen(text));
    EXPECT_EQ(*end, 0);
    EXPECT_NE(myFlags, 0);
}

TEST(XmlStrPairTest, ResetTest)
{
    char text[] { "1234567890"};
    tinyxml2::StrPair strPair;
    strPair.Set(text, text + strlen(text), tinyxml2::StrPair::NEEDS_ENTITY_PROCESSING);

    strPair.Reset();
    const auto [start, end, flags] = strPair.GetState();
    EXPECT_EQ(start, nullptr);
    EXPECT_EQ(end, nullptr);
    EXPECT_EQ(flags, 0);

}

TEST(XmlStrPairTest, EmptyTest)
{
    char text[] { "1234567890"};
    tinyxml2::StrPair strPair;
    strPair.Set(text, text + strlen(text), tinyxml2::StrPair::NEEDS_ENTITY_PROCESSING);
    bool res = strPair.Empty();
    EXPECT_EQ(res, false);

    strPair.Reset();
    res = strPair.Empty();
    EXPECT_EQ(res, true);
}

TEST(XmlStrPairTest, SetInternedStrTest)
{
    char text[] { "1234567890"};
    tinyxml2::StrPair strPair;
    strPair.SetInternedStr(text);
    const auto [start, end, flags] = strPair.GetState();
    EXPECT_EQ(start, &text[0]);
    EXPECT_EQ(end, nullptr);
    EXPECT_EQ(flags, 0);
}

TEST(XmlStrPairTest, SetStrTest)
{
    const int NEEDS_DELETE = 0x200;  // check in StrPair.h
    char *text = new char[] { "1234567890"};
    tinyxml2::StrPair strPair;
    strPair.SetStr(text, tinyxml2::StrPair::NEEDS_ENTITY_PROCESSING);
    const auto [start, end, flags] = strPair.GetState();
    auto myFlags = flags & (tinyxml2::StrPair::NEEDS_ENTITY_PROCESSING |  NEEDS_DELETE);
    EXPECT_EQ(*start, *text);
    EXPECT_EQ(end, start + strlen(text));
    EXPECT_EQ(myFlags, (tinyxml2::StrPair::NEEDS_ENTITY_PROCESSING |  NEEDS_DELETE));
    char *text2 = new char[] { "1234567890"};
    strPair.SetStr(text2, tinyxml2::StrPair::NEEDS_ENTITY_PROCESSING);
    const auto [start2, end2, flags2] = strPair.GetState();
    EXPECT_EQ(*start2, *text2);
    EXPECT_EQ(end2, start2 + strlen(text2));
    EXPECT_EQ(flags2, (tinyxml2::StrPair::NEEDS_ENTITY_PROCESSING |  NEEDS_DELETE));


}

TEST(XmlStrPairTest, GetStrTest) {
    {
        char testString[] { "&#160;\0"};
        tinyxml2::StrPair strPair;
        strPair.Set(testString, testString+6, tinyxml2::StrPair::NEEDS_ENTITY_PROCESSING);
        const char* result = strPair.GetStr();
        EXPECT_EQ(static_cast<unsigned char>(result[0]), static_cast<unsigned char>(194));
        EXPECT_EQ(static_cast<unsigned char>(result[1]), static_cast<unsigned char> (160));
    }
    {
        char testString[] { "&#x4e2d;"};
        tinyxml2::StrPair strPair;
        strPair.Set(testString, testString+8, tinyxml2::StrPair::NEEDS_ENTITY_PROCESSING);
        const char* result = strPair.GetStr();
        EXPECT_EQ(static_cast<unsigned char>(result[0]), static_cast<unsigned char>(228));
        EXPECT_EQ(static_cast<unsigned char>(result[1]), static_cast<unsigned char> (184));
        EXPECT_EQ(static_cast<unsigned char>(result[2]), static_cast<unsigned char> (173));

    }
}

TEST(XmlStrPairTest, ParseTextTest) {
    {
        char text[] { " as> ab"};
        char textApp[] {" ab"};
        char endTag[] {">"};
        const int NEEDS_FLUSH = 0x100;
        int strFlags = tinyxml2::StrPair::NEEDS_ENTITY_PROCESSING;
        int currentLineNumber = 0;
        tinyxml2::StrPair strPair;
        char* result = strPair.ParseText(text, endTag, strFlags, &currentLineNumber);
        const auto [start, end, flags] = strPair.GetState();

        EXPECT_EQ(*end, *endTag);
        EXPECT_EQ(*result, *textApp);
        EXPECT_EQ(*start, *text);
        EXPECT_EQ(flags, tinyxml2::StrPair::NEEDS_ENTITY_PROCESSING | NEEDS_FLUSH);
    }
    {
        char text[] { " \n\nas> ab"};
        char textApp[] {" ab"};
        char endTag[] {">"};
        const int NEEDS_FLUSH = 0x100;
        int strFlags = tinyxml2::StrPair::NEEDS_ENTITY_PROCESSING;
        int currentLineNumber = 0;
        tinyxml2::StrPair strPair;
        char* result = strPair.ParseText(text, endTag, strFlags, &currentLineNumber);
        const auto [start, end, flags] = strPair.GetState();

        EXPECT_EQ(*end, *endTag);
        EXPECT_EQ(*result, *textApp);
        EXPECT_EQ(*start, *text);
        EXPECT_EQ(flags, tinyxml2::StrPair::NEEDS_ENTITY_PROCESSING | NEEDS_FLUSH);
        EXPECT_EQ(currentLineNumber, 2);
    }

    {
        char text[] { " \n\nas ab"};
        char textApp[] {" ab"};
        char endTag[] {">"};
        const int NEEDS_FLUSH = 0x100;
        int strFlags = tinyxml2::StrPair::NEEDS_ENTITY_PROCESSING;
        int currentLineNumber = 0;
        tinyxml2::StrPair strPair;
        char* result = strPair.ParseText(text, endTag, strFlags, &currentLineNumber);
        const auto [start, end, flags] = strPair.GetState();

        EXPECT_EQ(result, nullptr);

    }
}

TEST(XmlStrPairTest, ParseNameTest)
{
    struct TestItem {
        std::string text;
        bool result;
    };

    std::array<TestItem,9> tests { TestItem{"Test", true}, TestItem{"4Test", false}, TestItem{"_4Test", true},
                                   TestItem{":Test", true}, TestItem{"ÄTest", true}, TestItem{"T4est", true},
                                   TestItem{"Te-st", true}, TestItem{"Te.st", true},
                                   TestItem{"T/st", false}};

    for (const auto& item: tests) {
        tinyxml2::StrPair strPair;
        char buffer[item.text.length()+1];
        memcpy(buffer, item.text.c_str(), item.text.length());
        char* result = strPair.ParseName(buffer);
        const auto [start, end, flags] = strPair.GetState();
        if (item.result) {
            EXPECT_EQ(*start, *item.text.c_str());
        }
        else {

            if (item.text == "T/st") {
                EXPECT_EQ(start[0], 'T' );
            }
            else {
                EXPECT_EQ(start, nullptr);
            }
        }
    }
}

TEST(XmlStrPairTest, TransferToTest)
{
    char start[] {"start"};
    char end[] {"end"};
    int flags {42};
    const int NEEDS_FLUSH = 0x100;
    tinyxml2::StrPair strPair1;
    strPair1.Set(start, end, flags);

    tinyxml2::StrPair strPair2;
    strPair1.TransferTo(&strPair2);
    const auto [startcheck, endcheck, flagscheck] = strPair2.GetState();
    EXPECT_EQ(*end, *endcheck);
    EXPECT_EQ(*start, *startcheck);
    EXPECT_EQ(flags, 42);
    const auto [startcheck1, endcheck1, flagscheck1] = strPair1.GetState();
    EXPECT_EQ(nullptr, endcheck1);
    EXPECT_EQ(nullptr, startcheck1);
    EXPECT_EQ(flagscheck1, 0);

}
