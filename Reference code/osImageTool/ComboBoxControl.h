/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#ifndef __COMBOBOXCONTROL_H__

#include <string>
#include "stringutils.h"

class ComboBoxControl {
public:
    ComboBoxControl(CComboBox* cb);

    void Initialize(const StringList& list);

    void SetSelection(int index);
	void SetText(const std::string& text);
    bool GetSelection(int& index) const;
	bool GetText(std::string& text) const;

private:
    CComboBox* m_cb;
};

#define __COMBOBOXCONTROL_H__
#endif
