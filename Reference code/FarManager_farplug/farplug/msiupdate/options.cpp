#include "utils.hpp"
#include "sysutils.hpp"
#include "farutils.hpp"
#include "options.hpp"

Options g_options;

class OptionsKey: public Far::Settings {
public:
  template<class Integer>
  Integer get_int(const wchar_t* name, Integer def_value) {
    unsigned __int64 value;
    if (get(name, value))
      return static_cast<Integer>(value);
    else
      return def_value;
  }

  bool get_bool(const wchar_t* name, bool def_value) {
    unsigned __int64 value;
    if (get(name, value))
      return value != 0;
    else
      return def_value;
  }

  wstring get_str(const wchar_t* name, const wstring& def_value) {
    wstring value;
    if (get(name, value))
      return value;
    else
      return def_value;
  }

  void set_int(const wchar_t* name, unsigned value, unsigned def_value) {
    if (value == def_value)
      del(name);
    else
      set(name, value);
  }

  void set_bool(const wchar_t* name, bool value, bool def_value) {
    if (value == def_value)
      del(name);
    else
      set(name, value ? 1 : 0);
  }

  void set_str(const wchar_t* name, const wstring& value, const wstring& def_value) {
    if (value == def_value)
      del(name);
    else
      set(name, value);
  }
};


HttpOptions::HttpOptions():
  use_proxy(false),
  proxy_server(),
  proxy_port(0),
  proxy_auth_scheme(0),
  proxy_user_name(),
  proxy_password() {
}

Options::Options():
  use_full_install_ui(false),
  update_stable_builds(false),
  logged_install(false),
  install_properties(),
  http(),
  cache_enabled(false),
  cache_max_size(2),
  cache_dir(L"%TEMP%"),
  open_changelog(true) {
}

const wchar_t* c_param_use_full_install_ui = L"use_full_install_ui";
const wchar_t* c_param_update_stable_builds = L"update_stable_builds";
const wchar_t* c_param_logged_install = L"logged_install";
const wchar_t* c_param_install_properties = L"install_properties";
const wchar_t* c_param_use_proxy = L"use_proxy";
const wchar_t* c_param_proxy_server = L"proxy_server";
const wchar_t* c_param_proxy_port = L"proxy_port";
const wchar_t* c_param_proxy_auth_scheme = L"proxy_auth_scheme";
const wchar_t* c_param_proxy_user_name = L"proxy_user_name";
const wchar_t* c_param_proxy_password = L"proxy_password";
const wchar_t* c_param_cache_enabled = L"cache_enabled";
const wchar_t* c_param_cache_max_size = L"cache_max_size";
const wchar_t* c_param_cache_dir = L"cache_dir";
const wchar_t* c_param_open_changelog = L"open_changelog";

void Options::load() {
  OptionsKey key;
  if (!key.create())
    return;
  Options defaults;
  use_full_install_ui = key.get_bool(c_param_use_full_install_ui, defaults.use_full_install_ui);
  update_stable_builds = key.get_bool(c_param_update_stable_builds, defaults.update_stable_builds);
  logged_install = key.get_bool(c_param_logged_install, defaults.logged_install);
  install_properties = key.get_str(c_param_install_properties, defaults.install_properties);
  http.use_proxy = key.get_bool(c_param_use_proxy, defaults.http.use_proxy);
  http.proxy_server = key.get_str(c_param_proxy_server, defaults.http.proxy_server);
  http.proxy_port = key.get_int(c_param_proxy_port, defaults.http.proxy_port);
  http.proxy_auth_scheme = key.get_int(c_param_proxy_auth_scheme, defaults.http.proxy_auth_scheme);
  http.proxy_user_name = key.get_str(c_param_proxy_user_name, defaults.http.proxy_user_name);
  http.proxy_password = key.get_str(c_param_proxy_password, defaults.http.proxy_password);
  cache_enabled = key.get_bool(c_param_cache_enabled, defaults.cache_enabled);
  cache_max_size = key.get_int(c_param_cache_max_size, defaults.cache_max_size);
  cache_dir = key.get_str(c_param_cache_dir, defaults.cache_dir);
  open_changelog = key.get_bool(c_param_open_changelog, defaults.open_changelog);
};

void Options::save() {
  OptionsKey key;
  if (!key.create())
    return;
  Options defaults;
  key.set_bool(c_param_use_full_install_ui, use_full_install_ui, defaults.use_full_install_ui);
  key.set_bool(c_param_update_stable_builds, update_stable_builds, defaults.update_stable_builds);
  key.set_bool(c_param_logged_install, logged_install, defaults.logged_install);
  key.set_str(c_param_install_properties, install_properties, defaults.install_properties);
  key.set_bool(c_param_use_proxy, http.use_proxy, defaults.http.use_proxy);
  key.set_str(c_param_proxy_server, http.proxy_server, defaults.http.proxy_server);
  key.set_int(c_param_proxy_port, http.proxy_port, defaults.http.proxy_port);
  key.set_int(c_param_proxy_auth_scheme, http.proxy_auth_scheme, defaults.http.proxy_auth_scheme);
  key.set_str(c_param_proxy_user_name, http.proxy_user_name, defaults.http.proxy_user_name);
  key.set_str(c_param_proxy_password, http.proxy_password, defaults.http.proxy_password);
  key.set_bool(c_param_cache_enabled, cache_enabled, defaults.cache_enabled);
  key.set_int(c_param_cache_max_size, cache_max_size, defaults.cache_max_size);
  key.set_str(c_param_cache_dir, cache_dir, defaults.cache_dir);
  key.set_bool(c_param_open_changelog, open_changelog, defaults.open_changelog);
}

