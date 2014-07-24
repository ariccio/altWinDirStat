#ifndef _DLGAPI_H
#define _DLGAPI_H

#define AUTO_SIZE (-1)

extern const wchar_t c_pb_black;
extern const wchar_t c_pb_white;

unsigned get_label_len(const std::string& str);

class FarDialog {
private:
  const GUID& guid;
  unsigned client_xs;
  unsigned client_ys;
  unsigned x;
  unsigned y;
  Array<FarDialogItem> items;
  std::vector<std::string> values;
  struct ListData {
    Array<FarList> list;
    Array<FarListItem> items;
  };
  std::vector<ListData> lists;
  // set text for dialog element
  void set_text(FarDialogItem& di, const std::string& text);
  // prepare buffer for EDIT controls
  void set_buf(FarDialogItem& di, const std::string& text);
  // set text for list items
  void set_list_text(FarListItem& li, const std::string& text);
  void frame(const std::string& text);
  void calc_frame_size();
public:
  FarDialog(const GUID& guid, const std::string& title, unsigned width);
  FarDialogItem& item() {
    return items.last_item();
  }
  void new_line();
  void spacer(unsigned size);
  void pad(unsigned pos);
  unsigned separator();
  unsigned label(const std::string& text);
  unsigned var_edit_box(const std::string& text, unsigned boxsize = AUTO_SIZE, DWORD flags = 0);
  unsigned mask_edit_box(const std::string& text, const std::string& mask, unsigned boxsize = AUTO_SIZE, DWORD flags = 0);
  unsigned button(const std::string& text, DWORD flags = 0, bool def = false);
  unsigned def_button(const std::string& text, DWORD flags = 0) {
    return button(text, flags, true);
  }
  unsigned check_box(const std::string& text, int value, DWORD flags = 0);
  unsigned check_box(const std::string& text, bool value, DWORD flags = 0) {
    return check_box(text, value ? 1 : 0, flags);
  }
  unsigned radio_button(const std::string& text, bool value, DWORD flags = 0);
  unsigned combo_box(const std::vector<std::string>& items, unsigned sel_idx, unsigned boxsize = AUTO_SIZE, DWORD flags = 0);
  intptr_t show(FARWINDOWPROC dlg_proc = NULL, void* dlg_data = NULL, const wchar_t* help = NULL);
};

#endif // _DLGAPI_H
