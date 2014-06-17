#ifndef _PLUGIN_H
#define _PLUGIN_H

struct DeviceInfo {
  RAPIDEVICEID id;
  std::string name;
  std::string platform;
  std::string con_type;
  std::string strid();
};

struct PluginItemList: public Array<PluginPanelItem> {
  std::vector<std::string> names;
};

class InfoPanel: private Array<InfoPanelLine> {
private:
  std::vector<std::string> info_lines;
public:
  void clear();
  void add_separator(const std::string& text);
  void add_info(const std::string& name, const std::string& value);
  const InfoPanelLine* data() const { return Array<InfoPanelLine>::data(); }
  unsigned size() const { return Array<InfoPanelLine>::size(); }
};

struct PluginInstance {
  IRAPISession* session;
  IRAPIDevice* device;
  std::string current_dir;
  std::vector<PluginItemList> file_lists;
  DeviceInfo device_info;
  std::string last_object;
  InfoPanel sys_info;
  std::string panel_title;
  unsigned __int64 free_space;
  std::string directory_name_buf;
  std::string dest_path_buf;
  PluginInstance(): session(NULL), device(NULL) {
  }
  ~PluginInstance() {
    if (session != NULL) session->Release();
    if (device != NULL) device->Release();
  }
};

extern struct PluginStartupInfo g_far;
extern ModuleVersion g_version;
extern unsigned __int64 g_time_freq;

#endif // _PLUGIN_H
