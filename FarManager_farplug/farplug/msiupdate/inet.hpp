#pragma once

class LoadUrlProgress: public ProgressMonitor, private CriticalSection {
private:
  unsigned completed;
  unsigned total;
public:
  LoadUrlProgress(): ProgressMonitor(), completed(0), total(0) {
  }
  virtual void do_update_ui();
  void set(unsigned completed, unsigned total) {
    if (this) {
      CriticalSectionLock cs_lock(*this);
      this->completed = completed;
      this->total = total;
    }
  }
};

string load_url(const wstring& url, const HttpOptions& options, HANDLE h_abort, LoadUrlProgress* progress = NULL);
string load_url(const wstring& url, const HttpOptions& options);
