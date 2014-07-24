#pragma once

class ProgressMonitor {
private:
  HANDLE h_scr;
  wstring con_title;
  unsigned __int64 t_start;
  unsigned __int64 t_curr;
  unsigned __int64 t_next;
  unsigned __int64 t_freq;
protected:
  unsigned __int64 time_elapsed() const {
    return (t_curr - t_start) / t_freq;
  }
  virtual void do_update_ui() = 0;
public:
  ProgressMonitor(bool lazy = true);
  virtual ~ProgressMonitor();
  void update_ui(bool force = false);
};

const wchar_t** get_size_suffixes();
const wchar_t** get_speed_suffixes();

bool config_dialog(Options& options);
