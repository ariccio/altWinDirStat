/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __SMARTHANDLE_H__

class SmartHandle {
public:
	inline SmartHandle() : m_h(INVALID_HANDLE_VALUE) {}
    inline SmartHandle(HANDLE h) : m_h(h) {}
    inline ~SmartHandle()
    {
        close();
    }
    inline SmartHandle& operator=(const HANDLE& h) {
        m_h= h;
		return *this;
    }

    inline operator HANDLE() const {
        return m_h;
    }
    inline void close()
    {
        if (m_h==INVALID_HANDLE_VALUE || m_h==NULL)
            return;
        CloseHandle(m_h);
        m_h= INVALID_HANDLE_VALUE;
    }
private:
    HANDLE m_h;
};
#define __SMARTHANDLE_H__
#endif
