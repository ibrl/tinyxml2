#ifndef DYNARRAY_H
#define DYNARRAY_H

#include <assert.h>
#include <climits>
#include <tuple>
#define TIXMLASSERT                assert

namespace tinyxml2 {
/*
    A dynamic array of Plain Old Data. Doesn't support constructors, etc.
    Has a small initial memory pool, so that low or no usage will not
    cause a call to new/delete
*/
template <class T, int INITIAL_SIZE>
class DynArray
{
public:
    DynArray() :
        _mem( _pool ),
        _allocated( INITIAL_SIZE ),
        _size( 0 )
    {
    }

    ~DynArray() {
        if ( _mem != _pool ) {
            delete [] _mem;
        }
    }

    void Clear() {
        _size = 0;
    }

    void Push( T t ) {
        TIXMLASSERT( _size < INT_MAX );
        EnsureCapacity( _size+1 );
        _mem[_size] = t;
        ++_size;
    }

    T* PushArr( int count ) {
        TIXMLASSERT( count >= 0 );
        TIXMLASSERT( _size <= INT_MAX - count );
        EnsureCapacity( _size+count );
        T* ret = &_mem[_size];
        _size += count;
        return ret;
    }

    T Pop() {
        TIXMLASSERT( _size > 0 );
        --_size;
        return _mem[_size];
    }

    void PopArr( int count ) {
        TIXMLASSERT( _size >= count );
        _size -= count;
    }

    bool Empty() const					{
        return _size == 0;
    }

    T& operator[](int i)				{
        TIXMLASSERT( i>= 0 && i < _size );
        return _mem[i];
    }

    const T& operator[](int i) const	{
        TIXMLASSERT( i>= 0 && i < _size );
        return _mem[i];
    }

    const T& PeekTop() const            {
        TIXMLASSERT( _size > 0 );
        return _mem[ _size - 1];
    }

    int Size() const					{
        TIXMLASSERT( _size >= 0 );
        return _size;
    }

    int Capacity() const				{
        TIXMLASSERT( _allocated >= INITIAL_SIZE );
        return _allocated;
    }

    void SwapRemove(int i) {
        TIXMLASSERT(i >= 0 && i < _size);
        TIXMLASSERT(_size > 0);
        _mem[i] = _mem[_size - 1];
        --_size;
    }

    const T* Mem() const				{
        TIXMLASSERT( _mem );
        return _mem;
    }

    T* Mem() {
        TIXMLASSERT( _mem );
        return _mem;
    }

    std::tuple<T*, int, int> GetState() const {
        return std::make_tuple(_mem, _allocated, _size);
    }

private:
    DynArray( const DynArray& ); // not supported
    void operator=( const DynArray& ); // not supported

    void EnsureCapacity( int cap ) {
        TIXMLASSERT( cap > 0 );
        if ( cap > _allocated ) {
            TIXMLASSERT( cap <= INT_MAX / 2 );
            const int newAllocated = cap * 2;
            T* newMem = new T[newAllocated];
            TIXMLASSERT( newAllocated >= _size );
            memcpy( newMem, _mem, sizeof(T)*_size );	// warning: not using constructors, only works for PODs
            if ( _mem != _pool ) {
                delete [] _mem;
            }
            _mem = newMem;
            _allocated = newAllocated;
        }
    }

    T*  _mem;
    T   _pool[INITIAL_SIZE];
    int _allocated;		// objects allocated
    int _size;			// number objects in use
};

}

#endif // DYNARRAY_H
