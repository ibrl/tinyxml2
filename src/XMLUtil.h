
#ifndef XMLUTIL_H
#define XMLUTIL_H

#include <assert.h>
#include <cctype>
#include <cstring>
#include <cstdint>
#include <climits>

#define TIXMLASSERT                assert

namespace tinyxml2 {

class XMLUtil {
public:
    static const char* SkipWhiteSpace( const char* p, int* curLineNumPtr )	{
        TIXMLASSERT( p );

        while( IsWhiteSpace(*p) ) {
            if (curLineNumPtr && *p == '\n') {
                ++(*curLineNumPtr);
            }
            ++p;
        }
        TIXMLASSERT( p );
        return p;
    }
    static char* SkipWhiteSpace( char* const p, int* curLineNumPtr ) {
        return const_cast<char*>( SkipWhiteSpace( const_cast<const char*>(p), curLineNumPtr ) );
    }

    // Anything in the high order range of UTF-8 is assumed to not be whitespace. This isn't
    // correct, but simple, and usually works.
    static bool IsWhiteSpace( char p )					{
        return !IsUTF8Continuation(p) && isspace( static_cast<unsigned char>(p) );
    }

    inline static bool IsNameStartChar( unsigned char ch ) {
        if ( ch >= 128 ) {
            // This is a heuristic guess in attempt to not implement Unicode-aware isalpha()
            return true;
        }
        if ( isalpha( ch ) ) {
            return true;
        }
        return ch == ':' || ch == '_';
    }

    inline static bool IsNameChar( unsigned char ch ) {
        return IsNameStartChar( ch )
               || isdigit( ch )
               || ch == '.'
               || ch == '-';
    }

    inline static bool IsPrefixHex( const char* p) {
        p = SkipWhiteSpace(p, 0);
        return p && *p == '0' && ( *(p + 1) == 'x' || *(p + 1) == 'X');
    }

    inline static bool StringEqual( const char* p, const char* q, int nChar=INT_MAX )  {
        if ( p == q ) {
            return true;
        }
        TIXMLASSERT( p );
        TIXMLASSERT( q );
        TIXMLASSERT( nChar >= 0 );
        return strncmp( p, q, nChar ) == 0;
    }

    inline static bool IsUTF8Continuation( const char p ) {
        return ( p & 0x80 ) != 0;
    }

    static const char* ReadBOM( const char* p, bool* hasBOM );
    // p is the starting location,
    // the UTF-8 value of the entity will be placed in value, and length filled in.
    static const char* GetCharacterRef( const char* p, char* value, int* length );
    static void ConvertUTF32ToUTF8( unsigned long input, char* output, int* length );

    // converts primitive types to strings
    static void ToStr( int v, char* buffer, int bufferSize );
    static void ToStr( unsigned v, char* buffer, int bufferSize );
    static void ToStr( bool v, char* buffer, int bufferSize );
    static void ToStr( float v, char* buffer, int bufferSize );
    static void ToStr( double v, char* buffer, int bufferSize );
    static void ToStr(int64_t v, char* buffer, int bufferSize);
    static void ToStr(uint64_t v, char* buffer, int bufferSize);

    // converts strings to primitive types
    static bool	ToInt( const char* str, int* value );
    static bool ToUnsigned( const char* str, unsigned* value );
    static bool	ToBool( const char* str, bool* value );
    static bool	ToFloat( const char* str, float* value );
    static bool ToDouble( const char* str, double* value );
    static bool ToInt64(const char* str, int64_t* value);
    static bool ToUnsigned64(const char* str, uint64_t* value);
    // Changes what is serialized for a boolean value.
    // Default to "true" and "false". Shouldn't be changed
    // unless you have a special testing or compatibility need.
    // Be careful: static, global, & not thread safe.
    // Be sure to set static const memory as parameters.
    static void SetBoolSerialization(const char* writeTrue, const char* writeFalse);

private:
    static const char* writeBoolTrue;
    static const char* writeBoolFalse;

};

}


#endif // XMLUTIL_H
