#include <gtest/gtest.h>
#include <assert.h>
#include <climits>
#include <charconv>
#include <iostream>

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
    char* testString { "&#x4acD;dfjk"};
    unsigned long ucs = checkIntegerValue(testString);
    EXPECT_EQ(ucs, 19149);
    char* testString2 { "&#x4L"};
    ucs = checkIntegerValue(testString2);
    EXPECT_EQ(ucs, 0);
}

TEST(XmlUtilTest, CheckIntegerValueRefactoredTest) {
    char* testString { "&#x4acD;dfjk"};
    unsigned long ucs = checkIntegerValueNew(testString);
    EXPECT_EQ(ucs, 19149);
    char* testString2 { "&#x4L"};
    ucs = checkIntegerValueNew(testString2);
    EXPECT_EQ(ucs, 0);
}

TEST(XmlUtilTest, CheckIntegerNullPtrTest) {
    char* testString = nullptr;
    unsigned long ucs = checkIntegerValueNew(testString);
    EXPECT_EQ(ucs, 0);
    char* testString2 { "&4244L"};
    ucs = checkIntegerValueNew(testString2);
    EXPECT_EQ(ucs, 0);
}

TEST(XmlUtilTest, CheckMaxUintTest) {
    char* testString = nullptr;
    unsigned long ucs = checkIntegerValueNew(testString);
    EXPECT_EQ(ucs, 0);
    char* testString2 { "&#xffffffffffffffff;"};
    ucs = checkIntegerValueNew(testString2);
    EXPECT_EQ(ucs, 18446744073709551615);

    ucs = checkIntegerValue(testString2);
    EXPECT_EQ(ucs, 18446744073709551615);
}


