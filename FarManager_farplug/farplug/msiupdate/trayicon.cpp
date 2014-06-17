#include "utils.hpp"
#include "sysutils.hpp"
#include "trayicon.hpp"

#define ICON_MESSAGE_ID (WM_USER + 1)

LRESULT TrayIcon::window_proc(UINT msg, WPARAM w_param, LPARAM l_param) {
  if (msg == ICON_MESSAGE_ID) {
    if (win2k) {
      if (l_param == WM_LBUTTONUP) end_message_loop(TRUE);
      else if (l_param == WM_RBUTTONUP) end_message_loop(FALSE);
    }
    else {
      if (l_param == NIN_BALLOONUSERCLICK) end_message_loop(TRUE);
      else if (l_param == NIN_BALLOONTIMEOUT) end_message_loop(FALSE);
    }
  }
  return DefWindowProcW(h_wnd, msg, w_param, l_param);
}

TrayIcon::TrayIcon(const wstring& window_name, const wstring& title, const wstring& text): MessageWindow(window_name), Icon(g_h_instance, c_icon_id, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON)) {
  win2k = LOWORD(GetVersion()) == 5;
  memset(&nid, 0, sizeof(nid));
  nid.cbSize = sizeof(nid);
  nid.hWnd = h_wnd;
  nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_INFO;
  if (win2k) nid.uFlags |= NIF_TIP;
  nid.uCallbackMessage = ICON_MESSAGE_ID;
  wcscpy(nid.szTip, text.c_str());
  nid.uTimeout = c_timeout;
  wcscpy(nid.szInfo, text.c_str());
  wcscpy(nid.szInfoTitle, title.c_str());
  nid.dwInfoFlags = NIIF_INFO;
  nid.hIcon = h_icon;
  CHECK_SYS(Shell_NotifyIconW(NIM_ADD, &nid));
}

TrayIcon::~TrayIcon() {
  CHECK_SYS(Shell_NotifyIconW(NIM_DELETE, &nid));
}
