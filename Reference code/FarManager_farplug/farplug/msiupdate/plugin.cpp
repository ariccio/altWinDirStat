#include "msg.h"
#include "plugin.h"
#include <initguid.h>
#include "guids.hpp"
#include "utils.hpp"
#include "sysutils.hpp"
#include "farutils.hpp"
#include "options.hpp"
#include "ui.hpp"
#include "update.hpp"

void WINAPI GetGlobalInfoW(GlobalInfo* info) {
  info->StructSize = sizeof(GlobalInfo);
  info->MinFarVersion = FARMANAGERVERSION;
  info->Version = PLUGIN_VERSION;
  info->Guid = c_plugin_guid;
  info->Title = PLUGIN_NAME;
  info->Description = PLUGIN_DESCRIPTION;
  info->Author = PLUGIN_AUTHOR;
}

void WINAPI SetStartupInfoW(const PluginStartupInfo* info) {
  Far::init(info);
  FAR_ERROR_HANDLER_BEGIN;
  g_options.load();
  Update::init();
  FAR_ERROR_HANDLER_END(return, return, false);
}

void WINAPI GetPluginInfoW(PluginInfo* info) {
  FAR_ERROR_HANDLER_BEGIN;
  static const wchar_t* plugin_menu[1];
  static const wchar_t* config_menu[1];
  info->StructSize = sizeof(PluginInfo);
  info->Flags = PF_PRELOAD;
  plugin_menu[0] = Far::msg_ptr(MSG_PLUGIN_NAME);
  info->PluginMenu.Strings = plugin_menu;
  info->PluginMenu.Guids = &c_plugin_menu_guid;
  info->PluginMenu.Count = ARRAYSIZE(plugin_menu);
  config_menu[0] = Far::msg_ptr(MSG_PLUGIN_NAME);
  info->PluginConfig.Strings = config_menu;
  info->PluginConfig.Guids = &c_plugin_config_guid;
  info->PluginConfig.Count = ARRAYSIZE(config_menu);
  FAR_ERROR_HANDLER_END(return, return, false);
}

HANDLE WINAPI OpenW(const OpenInfo* info) {
  FAR_ERROR_HANDLER_BEGIN;
  Update::execute(true);
  return INVALID_HANDLE_VALUE;
  FAR_ERROR_HANDLER_END(return INVALID_HANDLE_VALUE, return INVALID_HANDLE_VALUE, false);
}

intptr_t WINAPI ConfigureW(const ConfigureInfo* info) {
  FAR_ERROR_HANDLER_BEGIN;
  if (config_dialog(g_options)) {
    g_options.save();
    return TRUE;
  }
  return FALSE;
  FAR_ERROR_HANDLER_END(return FALSE, return FALSE, false);
}

void WINAPI ExitFARW(const ExitInfo* info) {
  FAR_ERROR_HANDLER_BEGIN;
  Update::clean();
  FAR_ERROR_HANDLER_END(return, return, true);
}

intptr_t WINAPI ProcessSynchroEventW(const ProcessSynchroEventInfo* info) {
  FAR_ERROR_HANDLER_BEGIN;
  if (info->Event == SE_COMMONSYNCHRO) {
    switch (static_cast<Update::Command>(reinterpret_cast<int>(info->Param))) {
    case Update::cmdClean:
      Update::clean();
      break;
    case Update::cmdExecute:
      Update::execute(false);
      break;
    }
  }
  return 0;
  FAR_ERROR_HANDLER_END(return 0, return 0, false);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  g_h_instance = hinstDLL;
  return TRUE;
}
