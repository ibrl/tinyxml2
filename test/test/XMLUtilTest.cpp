#include <gtest/gtest.h>
#include <cstring>
#include "../../src/XMLUtil.h"

TEST(XmlUtilTest, SkipWhiteSpaceSingleLineTest) {
    char const * line = "    test44adflö  ";
    char const * skippedLine = "test44adflö  ";
    int currentLineNumber = 0;

    const char* result = tinyxml2::XMLUtil::SkipWhiteSpace(line, &currentLineNumber);
    EXPECT_EQ(strcmp(result, skippedLine), 0);
    EXPECT_EQ(currentLineNumber, currentLineNumber);

}

TEST(XmlUtilTest, SkipWhiteSpaceMultipleLineTest) {
    char const * line = "    test44a\n  dflö  ";
    char const * skippedLine = "test44a\n  dflö  ";
    int currentLineNumber = 0;

    const char* result = tinyxml2::XMLUtil::SkipWhiteSpace(line, &currentLineNumber);
    EXPECT_EQ(strcmp(result, skippedLine), 0);
    EXPECT_EQ(currentLineNumber, currentLineNumber);
}


TEST(XmlUtilTest, SkipWhiteSpaceMultipleLine2Test) {
    char const * line = "\n      test44adflö  ";
    char const * skippedLine = "test44adflö  ";
    int currentLineNumber = 0;

    const char* result = tinyxml2::XMLUtil::SkipWhiteSpace(line, &currentLineNumber);
    EXPECT_EQ(strcmp(result, skippedLine), 0);
    EXPECT_EQ(currentLineNumber, 1);
}

TEST(XmlUtilTest, SkipWhiteSpaceMultipleLine2WithOutConstTest) {
    char const * line = "\n      test44adflö  ";
    char const * skippedLine = "test44adflö  ";
    int currentLineNumber = 0;

    char charLine[strlen(line)+1];
    strncpy(charLine, line, strlen(line)+1);
    char* result = tinyxml2::XMLUtil::SkipWhiteSpace(charLine, &currentLineNumber);
    EXPECT_EQ(strcmp(result, skippedLine), 0);
    EXPECT_EQ(currentLineNumber, 1);

}

TEST(XmlUtilTest, IsWhiteSpaceTest)
{
    char const * check = " \t\nü🐱";
    bool res = tinyxml2::XMLUtil::IsWhiteSpace(check[0]);
    EXPECT_EQ(res, true);
    res = tinyxml2::XMLUtil::IsWhiteSpace(check[1]);
    EXPECT_EQ(res, true);
    res = tinyxml2::XMLUtil::IsWhiteSpace(check[2]);
    EXPECT_EQ(res, true);
    res = tinyxml2::XMLUtil::IsWhiteSpace(check[3]);
    EXPECT_EQ(res, false);
    res = tinyxml2::XMLUtil::IsWhiteSpace(check[4]);
    EXPECT_EQ(res, false);
}


TEST(XmlUnitTest, IsNameStartCharTest)
{
    std::array<unsigned char, 8> check { 'a', 'Z', '1', '9', '/', '?', ':', '_'};
    std::array<bool, 8> results { true, true, false, false, false, false, true, true};

    for (auto i=0; i < check.size(); i++) {
        bool res = tinyxml2::XMLUtil::IsNameStartChar(check[i]);
        bool expected = results[i];
        EXPECT_EQ( res , results[i]);
    }
}

TEST(XmlUnitTest, IsNameCharTest)
{
    std::array<unsigned char, 10> check { 'a', 'Z', '1', '9', '/', '?', ':', '_','.','-'};
    std::array<bool, 10> results { true, true, true, true, false, false, true, true, true, true};

    for (auto i=0; i < check.size(); i++) {
        bool res = tinyxml2::XMLUtil::IsNameChar(check[i]);
        bool expected = results[i];
        EXPECT_EQ( res , results[i]);
    }
}

TEST(XmlUnitTest, IsPrefixHexTest)
{
    char const * firstCheck = {" \n0xab" };
    char const * secondCheck = {"Xa 24"};
    char const * thirdCheck = {"0Xa 24"};
    char const * emptyCheck = {};  // won't work as IsPrefixHex cannot handle nullptrs
    char const * oneCharCheck = {"0"};

    bool res = tinyxml2::XMLUtil::IsPrefixHex(firstCheck);
    EXPECT_EQ( res , true);
    res = tinyxml2::XMLUtil::IsPrefixHex(secondCheck);
    EXPECT_EQ( res , false);
    res = tinyxml2::XMLUtil::IsPrefixHex(thirdCheck);
    EXPECT_EQ( res , true);
    res = tinyxml2::XMLUtil::IsPrefixHex(oneCharCheck);
    EXPECT_EQ( res , false);
}

TEST(XmlUnitTest, StringEqualTest)
{
    char const * firstPart = {"abcDef"};
    char const * secondPart = {"abcIDF"};
    bool res = tinyxml2::XMLUtil::StringEqual(firstPart, firstPart);
    EXPECT_EQ( res , true);
    res = tinyxml2::XMLUtil::StringEqual(firstPart, secondPart, 3);
    EXPECT_EQ( res , true);
    res = tinyxml2::XMLUtil::StringEqual(firstPart, secondPart);
    EXPECT_EQ( res , false);
}

TEST(XmlUnitTest, IsUTF8ContinuationTest)
{
    const char c1 = 'A';
    const char c2 = static_cast<const char>('A' | 0x80);
    bool res = tinyxml2::XMLUtil::IsUTF8Continuation(c1);
    EXPECT_EQ( res , false);
    res = tinyxml2::XMLUtil::IsUTF8Continuation(c2);
    EXPECT_EQ( res , true);
}

TEST(XmlUnitTest, ReadBOMTest) {

    static const unsigned char TIXML_UTF_LEAD_0 = 0xefU;
    static const unsigned char TIXML_UTF_LEAD_1 = 0xbbU;
    static const unsigned char TIXML_UTF_LEAD_2 = 0xbfU;

    std::array<unsigned char, 10> check { TIXML_UTF_LEAD_0, TIXML_UTF_LEAD_1, TIXML_UTF_LEAD_2, '9', '/', '?', ':', '_','.','-'};
    char * ptr = reinterpret_cast<char*>(check.data());
    bool boom = false;
    const char * resPtr = tinyxml2::XMLUtil::ReadBOM(ptr, &boom);
    EXPECT_EQ(boom , true);
    EXPECT_EQ(resPtr, ptr+3);
    resPtr = tinyxml2::XMLUtil::ReadBOM(ptr+1, &boom);
    EXPECT_EQ(boom , false);
    EXPECT_EQ(resPtr, ptr+1);
}



TEST(XmlUnitTest, ConvertUTF32ToUTF8Test)
{
    unsigned long astonishedFace = 0x1f632;
    unsigned char astonishedFaceUtf8[4] { 0xF0, 0x9F, 0x98, 0xB2 };
    unsigned long PHAGS_PA_LETTER_HA = 0xA85C;
    unsigned char PHAGS_PA_LETTER_HAUtf8[3] { 0xEA, 0xA1, 0x9c };
    unsigned long scandinavianAE = 0xc6;
    unsigned char scandinavianAEUtf8[2] { 0xc3, 0x86 };
    unsigned long normalAscii = '@';

    char buffer[8];
    int length=0;
    memset(buffer, 0, 8);

    tinyxml2::XMLUtil::ConvertUTF32ToUTF8(astonishedFace, &buffer[0], &length);
    EXPECT_EQ(length, 4);
    for (int i=0; i < length; i++) {
        EXPECT_EQ(astonishedFaceUtf8[i], static_cast<unsigned char>(buffer[i]));
    }
    memset(buffer,0,8);
    tinyxml2::XMLUtil::ConvertUTF32ToUTF8(PHAGS_PA_LETTER_HA, &buffer[0], &length);
    EXPECT_EQ(length, 3);
    for (int i=0; i < length; i++) {
        EXPECT_EQ(PHAGS_PA_LETTER_HAUtf8[i], static_cast<unsigned char>(buffer[i]));
    }

    memset(buffer,0,8);
    tinyxml2::XMLUtil::ConvertUTF32ToUTF8(scandinavianAE, &buffer[0], &length);
    EXPECT_EQ(length, 2);
    for (int i=0; i < length; i++) {
        EXPECT_EQ(scandinavianAEUtf8[i], static_cast<unsigned char>(buffer[i]));
    }

    memset(buffer,0,8);
    tinyxml2::XMLUtil::ConvertUTF32ToUTF8(normalAscii, &buffer[0], &length);
    EXPECT_EQ(length, 1);
    EXPECT_EQ('@', buffer[0]);

}

TEST(XmlUnitTest, GetCharacterRefHexTest) {

    char const * astonishedFace { "&#x1f632;@"};
    char const * astonishedFaceWrong1 { "&#x1f632"};
    char const * astonishedFaceWrong2 { "&#x1G632;@"};
    char const * astonishedFaceWrong3 { "&A"};


    unsigned char astonishedFaceUtf8[4] { 0xF0, 0x9F, 0x98, 0xB2 };
    int length = 0;
    char result[8];
    memset(result,0,8);

    const char * out = tinyxml2::XMLUtil::GetCharacterRef(astonishedFace, &result[0], &length);
    EXPECT_EQ(length, 4);
    EXPECT_EQ(*out,'@');
    for (int i=0; i < length; i++) {
        EXPECT_EQ(astonishedFaceUtf8[i], static_cast<unsigned char>(result[i]));
    }

    memset(result,0,8);
    out = tinyxml2::XMLUtil::GetCharacterRef(astonishedFaceWrong1, &result[0], &length);
    EXPECT_EQ(out, nullptr);

    memset(result,0,8);
    out = tinyxml2::XMLUtil::GetCharacterRef(astonishedFaceWrong2, &result[0], &length);
    EXPECT_EQ(out, nullptr);

    memset(result,0,8);
    out = tinyxml2::XMLUtil::GetCharacterRef(astonishedFaceWrong3, &result[0], &length);
    EXPECT_EQ(*out, 'A');

}

TEST(XmlUnitTest, GetCharacterRefDecimalTest) {

    char const * astonishedFace { "&#128562;@"};
    char const * astonishedFaceWrong1 { "&#128562"};
    char const * astonishedFaceWrong2 { "&#1A8562;@"};
    char const * astonishedFaceWrong3 { "&A"};


    unsigned char astonishedFaceUtf8[4] { 0xF0, 0x9F, 0x98, 0xB2 };
    int length = 0;
    char result[8];
    memset(result,0,8);

    const char * out = tinyxml2::XMLUtil::GetCharacterRef(astonishedFace, &result[0], &length);
    EXPECT_EQ(length, 4);
    EXPECT_EQ(*out,'@');
    for (int i=0; i < length; i++) {
        EXPECT_EQ(astonishedFaceUtf8[i], static_cast<unsigned char>(result[i]));
    }

    memset(result,0,8);
    out = tinyxml2::XMLUtil::GetCharacterRef(astonishedFaceWrong1, &result[0], &length);
    EXPECT_EQ(out, nullptr);

    memset(result,0,8);
    out = tinyxml2::XMLUtil::GetCharacterRef(astonishedFaceWrong2, &result[0], &length);
    EXPECT_EQ(out, nullptr);

    memset(result,0,8);
    out = tinyxml2::XMLUtil::GetCharacterRef(astonishedFaceWrong3, &result[0], &length);
    EXPECT_EQ(*out, 'A');


}

TEST(XmlUnitTest, ToIntTest)
{
    char const * intDecimal { "1234"};
    char const * intNegativeDecimal { "-1234"};
    char const * intHex {"0x1234"};
    char const * intWrong("A4");

    int value;
    bool res = tinyxml2::XMLUtil::ToInt(intDecimal, &value);
    EXPECT_TRUE(res);
    EXPECT_EQ(value, 1234);

    res = tinyxml2::XMLUtil::ToInt(intNegativeDecimal, &value);
    EXPECT_TRUE(res);
    EXPECT_EQ(value, -1234);

    res = tinyxml2::XMLUtil::ToInt(intHex, &value);
    EXPECT_TRUE(res);
    EXPECT_EQ(value, 0x1234);

    res = tinyxml2::XMLUtil::ToInt(intWrong, &value);
    EXPECT_FALSE(res);

}


TEST(XmlUnitTest, ToUIntTest)
{
    char const * intDecimal { "1234"};
    char const * intNegativeDecimal { "-1234"};
    char const * intHex {"0x1234"};
    char const * intWrong("A4");

    unsigned value;
    bool res = tinyxml2::XMLUtil::ToUnsigned(intDecimal, &value);
    EXPECT_TRUE(res);
    EXPECT_EQ(value, 1234);

    res = tinyxml2::XMLUtil::ToUnsigned(intNegativeDecimal, &value);
    EXPECT_TRUE(res);
    EXPECT_EQ(value, 0xFFFFFB2E);

    res = tinyxml2::XMLUtil::ToUnsigned(intHex, &value);
    EXPECT_TRUE(res);
    EXPECT_EQ(value, 0x1234);

    res = tinyxml2::XMLUtil::ToUnsigned(intWrong, &value);
    EXPECT_FALSE(res);
}

TEST(XmlUnitTest, ToBoolTest) {

    struct TestValues {
        std::string valueString;
        bool result;
        bool boolValue;
    };

    std::array<TestValues,10> testValues { TestValues{"true", true, true}, TestValues{"True", true, true},
                                          TestValues{"TRUE", true, true}, TestValues{"TRue", false, false},
                                          TestValues{"false", true, false}, TestValues{"False",true,false},
                                          TestValues{"FALSE", true, false}, TestValues{"FAlse", false, false},
                                          TestValues{"0", true, false}, TestValues{"1", true, true}};
    for (const auto& item: testValues) {
        bool resultValue;
        bool res = tinyxml2::XMLUtil::ToBool(item.valueString.c_str(), &resultValue);
        EXPECT_EQ(item.result, res);
        if (res) {
            EXPECT_EQ(item.boolValue, resultValue);
        }
    }
}

TEST(XmlUnitTest, ToFloatTest) {
    std::string sf = "2.349493";
    float resultValue;
    bool res = tinyxml2::XMLUtil::ToFloat(sf.c_str(), &resultValue);
    EXPECT_TRUE(res);
    EXPECT_FLOAT_EQ(2.349493, resultValue);
}

TEST(XmlUnitTest, ToDoubleTest) {
    std::string sf = "2.34949343443";
    double resultValue;
    bool res = tinyxml2::XMLUtil::ToDouble(sf.c_str(), &resultValue);
    EXPECT_TRUE(res);
    EXPECT_DOUBLE_EQ(2.34949343443, resultValue);
}

TEST(XmlUnitTest, ToInt64Test) {
    std::string s64 = "9223372036854775807";
    int64_t resultValue;
    bool res = tinyxml2::XMLUtil::ToInt64(s64.c_str(), &resultValue);
    EXPECT_TRUE(res);
    EXPECT_EQ(9223372036854775807, resultValue);
}

TEST(XmlUnitTest, ToUnsigned64Test) {
    std::string s64 = "18446744073709551615";
    uint64_t resultValue;
    bool res = tinyxml2::XMLUtil::ToUnsigned64(s64.c_str(), &resultValue);
    EXPECT_TRUE(res);
    EXPECT_EQ(18446744073709551615, resultValue);
}

TEST(XmlUnitTest, ToStringIntTest) {

}

