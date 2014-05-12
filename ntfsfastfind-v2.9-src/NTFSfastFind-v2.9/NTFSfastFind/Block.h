// ------------------------------------------------------------------------------------------------
// Memory Block object used like a vector with bounds checking.
//
// Project: NTFSfastFind
// Author:  Dennis Lang   Apr-2011
// http://home.comcast.net/~lang.dennis/
// ------------------------------------------------------------------------------------------------


#pragma once

#include <exception>
#include <vector>
#include <assert.h>


struct BlockException : public std::exception
{
    const char* what() const throw() { return "Block"; }
};

// Readonly memory region.
class Block
{
public:
    Block(void) :
        m_ptr(0), m_len(0)
    { }

    Block(const void* ptr, size_t len) :
        m_ptr((char*)ptr), m_len(len)
    { }

    template <typename T>
    Block(const std::vector<T>& buffer) :
        m_ptr((const char*)&buffer[0]), m_len(buffer.size())
    {
    }

    void Set(const void* ptr, size_t len)
    { 
        m_ptr = (char*)ptr; 
        m_len = len; 
    }

    void* Copy(void* pDst, size_t offset, size_t len) const
    {
        if (offset + len > m_len)
            throw BlockException();

        memcpy(pDst, m_ptr + offset, len);
        return pDst;
    }

    template <typename T>
    const T& OutRef(size_t offset, size_t len) const
    {
        if (offset + len > m_len)
            throw BlockException();
        return *(T*)(m_ptr + offset);
    }

    template <typename T>
    const T* OutPtr(size_t offset, size_t len = sizeof(T)) const
    {
        if (offset + len > m_len)
            throw BlockException();
        return (T*)(m_ptr + offset);
    }

    const void* OutVPtr(size_t offset) const
    {
        return (m_ptr + offset);
    }

    size_t size() const
    { return m_len; }

private:
    const char* m_ptr;
    size_t      m_len;
};

