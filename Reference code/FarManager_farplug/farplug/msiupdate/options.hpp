#pragma once

struct HttpOptions {
  bool use_proxy;
  wstring proxy_server;
  unsigned proxy_port;
  unsigned proxy_auth_scheme;
  wstring proxy_user_name;
  wstring proxy_password;
  HttpOptions();
};

class Options {
public:
  bool use_full_install_ui;
  bool update_stable_builds;
  bool logged_install;
  wstring install_properties;
  HttpOptions http;
  bool cache_enabled;
  unsigned cache_max_size;
  wstring cache_dir;
  bool open_changelog;
  Options();
  void load();
  void save();
};

extern Options g_options;
