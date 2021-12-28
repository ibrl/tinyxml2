
#include "XMLUtil.h"
#include <cstddef>
#include <cstdarg>
#include <cstdio>

#define TIXML_SNPRINTF	snprintf
#define TIXML_VSNPRINTF	vsnprintf
static inline int TIXML_VSCPRINTF( const char* format, va_list va )
{
    int len = vsnprintf( 0, 0, format, va );
    TIXMLASSERT( len >= 0 );
    return len;
}
#define TIXML_SSCANF   sscanf

using namespace tinyxml2;


const char* XMLUtil::writeBoolTrue  = "true";
const char* XMLUtil::writeBoolFalse = "false";

static const unsigned char TIXML_UTF_LEAD_0 = 0xefU;
static const unsigned char TIXML_UTF_LEAD_1 = 0xbbU;
static const unsigned char TIXML_UTF_LEAD_2 = 0xbfU;


void XMLUtil::SetBoolSerialization(const char* writeTrue, const char* writeFalse)
{
    static const char* defTrue  = "true";
    static const char* defFalse = "false";

    writeBoolTrue = (writeTrue) ? writeTrue : defTrue;
    writeBoolFalse = (writeFalse) ? writeFalse : defFalse;
}


const char* XMLUtil::ReadBOM( const char* p, bool* bom )
{
    TIXMLASSERT( p );
    TIXMLASSERT( bom );
    *bom = false;
    const unsigned char* pu = reinterpret_cast<const unsigned char*>(p);
    // Check for BOM:
    if (    *(pu+0) == TIXML_UTF_LEAD_0
            && *(pu+1) == TIXML_UTF_LEAD_1
            && *(pu+2) == TIXML_UTF_LEAD_2 ) {
        *bom = true;
        p += 3;
    }
    TIXMLASSERT( p );
    return p;
}


void XMLUtil::ConvertUTF32ToUTF8( unsigned long input, char* output, int* length )
{
    const unsigned long BYTE_MASK = 0xBF;
    const unsigned long BYTE_MARK = 0x80;
    const unsigned long FIRST_BYTE_MARK[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

    if (input < 0x80) {
        *length = 1;
    }
    else if ( input < 0x800 ) {
        *length = 2;
    }
    else if ( input < 0x10000 ) {
        *length = 3;
    }
    else if ( input < 0x200000 ) {
        *length = 4;
    }
    else {
        *length = 0;    // This code won't convert this correctly anyway.
        return;
    }

    output += *length;

    // Scary scary fall throughs are annotated with carefully designed comments
    // to suppress compiler warnings such as -Wimplicit-fallthrough in gcc
    switch (*length) {
        case 4:
            --output;
            *output = static_cast<char>((input | BYTE_MARK) & BYTE_MASK);
            input >>= 6;
            //fall through
        case 3:
            --output;
            *output = static_cast<char>((input | BYTE_MARK) & BYTE_MASK);
            input >>= 6;
            //fall through
        case 2:
            --output;
            *output = static_cast<char>((input | BYTE_MARK) & BYTE_MASK);
            input >>= 6;
            //fall through
        case 1:
            --output;
            *output = static_cast<char>(input | FIRST_BYTE_MARK[*length]);
            break;
        default:
            TIXMLASSERT( false );
    }
}


const char* XMLUtil::GetCharacterRef( const char* p, char* value, int* length )
{
    // Presume an entity, and pull it out.
    *length = 0;

    if ( *(p+1) == '#' && *(p+2) ) {
        unsigned long ucs = 0;
        TIXMLASSERT( sizeof( ucs ) >= 4 );
        ptrdiff_t delta = 0;
        unsigned mult = 1;
        static const char SEMICOLON = ';';

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
        else {
            // Decimal.
            const char* q = p+2;
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

            while ( *q != '#' ) {
                if ( *q >= '0' && *q <= '9' ) {
                    const unsigned int digit = *q - '0';
                    TIXMLASSERT( digit < 10 );
                    TIXMLASSERT( digit == 0 || mult <= UINT_MAX / digit );
                    const unsigned int digitScaled = mult * digit;
                    TIXMLASSERT( ucs <= ULONG_MAX - digitScaled );
                    ucs += digitScaled;
                }
                else {
                    return 0;
                }
                TIXMLASSERT( mult <= UINT_MAX / 10 );
                mult *= 10;
                --q;
            }
        }
        // convert the UCS to UTF-8
        ConvertUTF32ToUTF8( ucs, value, length );
        return p + delta + 1;
    }
    return p+1;
}


void XMLUtil::ToStr( int v, char* buffer, int bufferSize )
{
    TIXML_SNPRINTF( buffer, bufferSize, "%d", v );
}


void XMLUtil::ToStr( unsigned v, char* buffer, int bufferSize )
{
    TIXML_SNPRINTF( buffer, bufferSize, "%u", v );
}


void XMLUtil::ToStr( bool v, char* buffer, int bufferSize )
{
    TIXML_SNPRINTF( buffer, bufferSize, "%s", v ? writeBoolTrue : writeBoolFalse);
}

/*
    ToStr() of a number is a very tricky topic.
    https://github.com/leethomason/tinyxml2/issues/106
*/
void XMLUtil::ToStr( float v, char* buffer, int bufferSize )
{
    TIXML_SNPRINTF( buffer, bufferSize, "%.8g", v );
}


void XMLUtil::ToStr( double v, char* buffer, int bufferSize )
{
    TIXML_SNPRINTF( buffer, bufferSize, "%.17g", v );
}


void XMLUtil::ToStr( int64_t v, char* buffer, int bufferSize )
{
    // horrible syntax trick to make the compiler happy about %lld
    TIXML_SNPRINTF(buffer, bufferSize, "%lld", static_cast<long long>(v));
}

void XMLUtil::ToStr( uint64_t v, char* buffer, int bufferSize )
{
    // horrible syntax trick to make the compiler happy about %llu
    TIXML_SNPRINTF(buffer, bufferSize, "%llu", (long long)v);
}

bool XMLUtil::ToInt(const char* str, int* value)
{
    if (IsPrefixHex(str)) {
        unsigned v;
        if (TIXML_SSCANF(str, "%x", &v) == 1) {
            *value = static_cast<int>(v);
            return true;
        }
    }
    else {
        if (TIXML_SSCANF(str, "%d", value) == 1) {
            return true;
        }
    }
    return false;
}

bool XMLUtil::ToUnsigned(const char* str, unsigned* value)
{
    if (TIXML_SSCANF(str, IsPrefixHex(str) ? "%x" : "%u", value) == 1) {
        return true;
    }
    return false;
}

bool XMLUtil::ToBool( const char* str, bool* value )
{
    int ival = 0;
    if ( ToInt( str, &ival )) {
        *value = (ival==0) ? false : true;
        return true;
    }
    static const char* TRUE_VALS[] = { "true", "True", "TRUE", 0 };
    static const char* FALSE_VALS[] = { "false", "False", "FALSE", 0 };

    for (int i = 0; TRUE_VALS[i]; ++i) {
        if (StringEqual(str, TRUE_VALS[i])) {
            *value = true;
            return true;
        }
    }
    for (int i = 0; FALSE_VALS[i]; ++i) {
        if (StringEqual(str, FALSE_VALS[i])) {
            *value = false;
            return true;
        }
    }
    return false;
}


bool XMLUtil::ToFloat( const char* str, float* value )
{
    if ( TIXML_SSCANF( str, "%f", value ) == 1 ) {
        return true;
    }
    return false;
}


bool XMLUtil::ToDouble( const char* str, double* value )
{
    if ( TIXML_SSCANF( str, "%lf", value ) == 1 ) {
        return true;
    }
    return false;
}


bool XMLUtil::ToInt64(const char* str, int64_t* value)
{
    if (IsPrefixHex(str)) {
        unsigned long long v = 0;	// horrible syntax trick to make the compiler happy about %llx
        if (TIXML_SSCANF(str, "%llx", &v) == 1) {
            *value = static_cast<int64_t>(v);
            return true;
        }
    }
    else {
        long long v = 0;	// horrible syntax trick to make the compiler happy about %lld
        if (TIXML_SSCANF(str, "%lld", &v) == 1) {
            *value = static_cast<int64_t>(v);
            return true;
        }
    }
    return false;
}


bool XMLUtil::ToUnsigned64(const char* str, uint64_t* value) {
    unsigned long long v = 0;	// horrible syntax trick to make the compiler happy about %llu
    if(TIXML_SSCANF(str, IsPrefixHex(str) ? "%llx" : "%llu", &v) == 1) {
        *value = (uint64_t)v;
        return true;
    }
    return false;
}
