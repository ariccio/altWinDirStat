/* (C) 2003 XDA Developers
 * Author: Willem Jan Hengeveld <itsme@xs4all.nl>
 * Web: http://www.xda-developers.com/
 *
 * $Header$
 */
#include "stdafx.h"
#include "ComboBoxControl.h"
#include <string>
#include "stringutils.h"

ComboBoxControl::ComboBoxControl(CComboBox* cb)
{
    m_cb= cb;
}

void ComboBoxControl::Initialize(const StringList& list)
{
    for (size_t i=0 ; i<list.size() ; i++)
    {
        m_cb->SetItemData(m_cb->AddString(list[i].c_str()), i);
    }
}

void ComboBoxControl::SetSelection(int index)
{
	for (int i=0 ; i<m_cb->GetCount() ; i++)
	{
		if (m_cb->GetItemData(i)==index)
		{
			m_cb->SetCurSel(i);
			return;
		}
	}
}

void ComboBoxControl::SetText(const std::string& text)
{
    m_cb->SetWindowText(CString(text.c_str()));
}

bool ComboBoxControl::GetSelection(int& index) const
{
	int sel= m_cb->GetCurSel();

    if (sel==CB_ERR)
        return false;

    index= (int)m_cb->GetItemData(sel);
    return true;
}

bool ComboBoxControl::GetText(std::string& text) const
{
	int sel= m_cb->GetCurSel();
    if (sel!=CB_ERR)
        return false;

    CString dst;
    m_cb->GetWindowText(dst);

    if (dst.IsEmpty())
        return false;

    text= std::string(dst.GetBuffer());
    return true;
}

