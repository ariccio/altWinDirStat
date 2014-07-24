#define _ERROR_WINDOWS
#include "error.h"

#include "msg.h"
#include "pinfo.h"
#include <InitGuid.h>
#include "guids.h"
#include "util.h"
#include "options.h"
#include "dlgapi.h"
#include "file_filters.h"
#include "file_info.h"
#include "ui.h"
#include "filepath.h"
#include "plugin.h"
#include "rapi_proc.h"

extern "C" HINSTANCE g_h_module;
HINSTANCE g_h_module;
struct PluginStartupInfo g_far;
struct FarStandardFunctions g_fsf;
Array<FarColor> g_colors;
PluginOptions g_plugin_options;
ModuleVersion g_version;
unsigned __int64 g_time_freq; // system timer resolution
Array<PluginInstance*> g_plugin_objects; // list of all open plugin instances
std::string g_plugin_format;

std::string DeviceInfo::strid() {
  const unsigned size = 64;
  std::string res;
  VERIFY(StringFromGUID2(id, res.buf(size), size) != 0);
  res.set_size();
  return res;
}

void error_dlg(Error& e, const std::string& message) {
  std::string msg;
  msg.add(far_get_msg(MSG_PLUGIN_NAME)).add('\n');
  if (message.size() != 0) msg.add(word_wrap(message, get_msg_width())).add('\n');
  std::string err_msg = word_wrap(e.message(), get_msg_width());
  if (err_msg.size() != 0) msg.add(err_msg).add('\n');
  msg.add_fmt(L"%S:%u v.%u.%u.%u.%u"PLUGIN_TYPE, &extract_file_name(oem_to_unicode(e.file)), e.line, g_version.major, g_version.minor, g_version.patch, g_version.revision);
  far_message(c_error_dialog_guid, msg, 0, FMSG_WARNING | FMSG_MB_OK);
}

void error_dlg(CustomError& e) {
  std::string msg;
  msg.add(far_get_msg(MSG_PLUGIN_NAME)).add('\n');
  if (e.object().size() != 0) msg.add(fit_str(e.object(), get_msg_width())).add('\n');
  if (e.message().size() != 0) msg.add(word_wrap(e.message(), get_msg_width())).add('\n');
  msg.add_fmt(L"%S:%u v.%u.%u.%u.%u"PLUGIN_TYPE, &extract_file_name(oem_to_unicode(e.file)), e.line, g_version.major, g_version.minor, g_version.patch, g_version.revision);
  far_message(c_error_dialog_guid, msg, 0, FMSG_WARNING | FMSG_MB_OK);
}

void error_dlg(const std::exception& e) {
  std::string msg;
  msg.add(far_get_msg(MSG_PLUGIN_NAME)).add('\n');
  std::string err_msg = word_wrap(oem_to_unicode(e.what()), get_msg_width());
  if (err_msg.size() != 0) msg.add(err_msg).add('\n');
  msg.add_fmt(L"v.%u.%u.%u.%u"PLUGIN_TYPE, g_version.major, g_version.minor, g_version.patch, g_version.revision);
  far_message(c_error_dialog_guid, msg, 0, FMSG_WARNING | FMSG_MB_OK);
}

void msg_dlg(const std::string& message) {
  std::string msg;
  msg.add(far_get_msg(MSG_PLUGIN_NAME)).add('\n');
  if (message.size() != 0) msg.add(word_wrap(message, get_msg_width())).add('\n');
  far_message(c_error_dialog_guid, msg, 0, FMSG_MB_OK);
}

void fatal_error_dlg(bool dump_saved) {
  const wchar_t* msg[3];
  msg[0] = far_msg_ptr(MSG_PLUGIN_NAME);
  msg[1] = far_msg_ptr(MSG_ERR_FATAL);
  msg[2] = far_msg_ptr(dump_saved ? MSG_ERR_FATAL_DUMP : MSG_ERR_FATAL_NO_DUMP);
  far_message(c_error_dialog_guid, msg, sizeof(msg) / sizeof(msg[0]), 0, FMSG_MB_OK);
}

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
  g_far = *info;
  g_fsf = *info->FSF;
  g_far.FSF = &g_fsf;
  // measure system timer frequency
  QueryPerformanceFrequency((PLARGE_INTEGER) &g_time_freq);
  // load Far color table
  far_load_colors();
  // load registry options
  load_plugin_options(g_plugin_options);
  g_rapi2 = g_plugin_options.access_method == amRapi2;
  g_plugin_format = g_plugin_options.prefix;
  // load version info
  g_version = get_module_version(g_h_module);
}

const wchar_t* disk_menu[1];
const wchar_t* plugin_menu[1];
const wchar_t* config_menu[1];
void WINAPI GetPluginInfoW(PluginInfo* info) {
  info->StructSize = sizeof(PluginInfo);

  std::vector<DeviceInfo> dev_list;
  try {
    init_if_needed();
    get_device_list(dev_list);
  }
  catch (...) {
    dev_list.clear();
  }
  if (dev_list.size() == 0) {
    if (g_plugin_options.last_dev_type == dtPDA) disk_menu[0] = far_msg_ptr(MSG_DISK_MENU_PDA);
    else if (g_plugin_options.last_dev_type == dtSmartPhone) disk_menu[0] = far_msg_ptr(MSG_DISK_MENU_SMARTPHONE);
    else disk_menu[0] = far_msg_ptr(MSG_DISK_MENU_PDA);
  }
  else {
    if (dev_list[0].platform == L"PocketPC") {
      disk_menu[0] = far_msg_ptr(MSG_DISK_MENU_PDA);
      g_plugin_options.last_dev_type = dtPDA;
    }
    else if (dev_list[0].platform == L"SmartPhone") {
      disk_menu[0] = far_msg_ptr(MSG_DISK_MENU_SMARTPHONE);
      g_plugin_options.last_dev_type = dtSmartPhone;
    }
    else {
      disk_menu[0] = far_msg_ptr(MSG_DISK_MENU_PDA);
      g_plugin_options.last_dev_type = dtPDA;
    }
    save_plugin_options(g_plugin_options);
  }

  if (g_plugin_options.add_to_disk_menu) {
    info->DiskMenu.Strings = disk_menu;
    info->DiskMenu.Guids = &c_disk_menu_guid;
    info->DiskMenu.Count = ARRAYSIZE(disk_menu);
  }

  if (g_plugin_options.add_to_plugin_menu) {
    plugin_menu[0] = far_msg_ptr(MSG_PLUGIN_NAME);
    info->PluginMenu.Strings = plugin_menu;
    info->PluginMenu.Guids = &c_plugin_menu_guid;
    info->PluginMenu.Count = ARRAYSIZE(plugin_menu);
  }

  config_menu[0] = far_msg_ptr(MSG_PLUGIN_NAME);
  info->PluginConfig.Strings = config_menu;
  info->PluginConfig.Guids = &c_config_menu_guid;
  info->PluginConfig.Count = ARRAYSIZE(config_menu);

  info->CommandPrefix = g_plugin_format.data();
}

#define HANDLE_ERROR(err_ret, break_ret) \
  catch (Break&) { \
    return break_ret; \
  } \
  catch (CustomError& e) { \
    if (show_error) error_dlg(e); \
    return err_ret; \
  } \
  catch (MsgError& e) { \
    if (show_error) msg_dlg(e.message()); \
    return err_ret; \
  } \
  catch (ComError& e) { \
    if (e.error() == HRESULT_FROM_WIN32(ERROR_NOT_READY)) { \
      far_control_ptr(plugin, FCTL_CLOSEPANEL, nullptr); \
      if (show_error) msg_dlg(far_get_msg(MSG_ERR_DEVICE_DISCONNECTED)); \
    } \
    else { \
      LOG_ERR(e); \
      if (show_error) error_dlg(e, fit_str(plugin->last_object, get_msg_width())); \
    } \
    return err_ret; \
  } \
  catch (Error& e) { \
    LOG_ERR(e); \
    if (show_error) error_dlg(e, fit_str(plugin->last_object, get_msg_width())); \
    return err_ret; \
  } \
  catch (std::exception& e) { \
    if (show_error) error_dlg(e); \
    return err_ret; \
  } \
  catch (...) { \
    far_message(c_error_dialog_guid, L"\nFailure!", 0, FMSG_WARNING | FMSG_MB_OK); \
    return err_ret; \
  }

int set_dir(PluginInstance* plugin, const wchar_t* dir, bool show_error); // forward

HANDLE WINAPI OpenW(const OpenInfo* info) {
  try {
    if (info->OpenFrom == OPEN_COMMANDLINE) {
      // test if plugin is already open on active panel
      for (unsigned i = 0; i < g_plugin_objects.size(); i++) {
        PanelInfo pi;
        if (far_control_ptr(g_plugin_objects[i], FCTL_GETPANELINFO, &pi)) {
          if (pi.Flags & PFLAGS_FOCUS) {
            // change current directory of active plugin
            set_dir(g_plugin_objects[i], reinterpret_cast<OpenCommandLineInfo*>(info->Data)->CommandLine, true);
            far_control_int(INVALID_HANDLE_VALUE, FCTL_UPDATEPANEL, 0);
            PanelRedrawInfo pri = { sizeof(PanelRedrawInfo) };
            pri.CurrentItem = 0;
            pri.TopPanelItem = 0;
            far_control_ptr(INVALID_HANDLE_VALUE, FCTL_REDRAWPANEL, &pri);
            // do not create new plugin instance
            return nullptr;
          }
        }
      }
    }
    init_if_needed();
    std::vector<DeviceInfo> dev_list;
    get_device_list(dev_list);
    if (dev_list.size() == 0) FAIL(MsgError(far_get_msg(MSG_ERR_NO_DEVICE)));
    intptr_t mi = 0;
    if (dev_list.size() > 1) {
      std::vector<std::string> menu_str;
      for (unsigned i = 0; i < dev_list.size(); i++) {
        menu_str += std::string::format(L"%S %S (%S)", &dev_list[i].name, &dev_list[i].platform, &dev_list[i].con_type);
      }
      mi = far_menu(c_device_list_menu_guid, far_get_msg(MSG_DEVICE_LIST_TITLE), menu_str);
      if (mi == -1) BREAK;
    }
    PluginInstance* plugin = new PluginInstance();
    try {
      plugin->device_info = dev_list[mi];
      create_session(plugin->device_info.id, plugin);
      if (info->OpenFrom == OPEN_COMMANDLINE) {
        // directory is specified on command line
        plugin->current_dir = reinterpret_cast<OpenCommandLineInfo*>(info->Data)->CommandLine;
        if (!dir_exists(plugin->current_dir, plugin->session)) plugin->current_dir = L"\\";
      }
      else if (g_plugin_options.save_last_dir) {
        // restore last directory
        plugin->current_dir = load_last_dir(plugin->device_info.strid());
        if (!dir_exists(plugin->current_dir, plugin->session)) plugin->current_dir = L"\\";
      }
      else {
        plugin->current_dir = L"\\";
      }
      NOFAIL(refresh_system_info(plugin));
      g_plugin_objects += plugin;
    }
    catch (...) {
      delete plugin;
      throw;
    }
    return plugin;
  }
  catch (Break&) {
  }
  catch (MsgError& e) {
    msg_dlg(e.message());
  }
  catch (Error& e) {
    LOG_ERR(e);
    error_dlg(e, far_get_msg(MSG_ERR_PLUGIN_INIT));
  }
  catch (...) {
    far_message(c_error_dialog_guid, L"\nFailure!", 0, FMSG_WARNING | FMSG_MB_OK);
  }
  return nullptr;
}

void WINAPI GetOpenPanelInfoW(OpenPanelInfo* info) {
  PluginInstance* plugin = (PluginInstance*) info->hPanel;

  info->StructSize = sizeof(OpenPanelInfo);

  info->Flags = OPIF_ADDDOTS | OPIF_SHORTCUT;
  info->CurDir = plugin->current_dir.data();
  info->Format = g_plugin_format.data();

  if (g_plugin_options.last_dev_type == dtPDA) plugin->panel_title = far_msg_ptr(MSG_DISK_MENU_PDA);
  else if (g_plugin_options.last_dev_type == dtSmartPhone) plugin->panel_title = far_msg_ptr(MSG_DISK_MENU_SMARTPHONE);
  else plugin->panel_title = far_msg_ptr(MSG_DISK_MENU_PDA);
  plugin->panel_title.add(L":").add(plugin->current_dir);

  if (g_plugin_options.show_free_space) plugin->panel_title.add(L":" + format_data_size(plugin->free_space, get_size_suffixes()));
  info->PanelTitle = plugin->panel_title.data();
  info->InfoLines = plugin->sys_info.data();
  info->InfoLinesNumber = plugin->sys_info.size();
}

void WINAPI ClosePanelW(const ClosePanelInfo* info) {
  PluginInstance* plugin = (PluginInstance*) info->hPanel;
  end_session(plugin);
  // save last directory
  if (g_plugin_options.save_last_dir) {
    save_last_dir(plugin->device_info.strid(), plugin->current_dir);
  }
  g_plugin_objects.remove(g_plugin_objects.search(plugin));
  delete plugin;
}

intptr_t WINAPI GetFindDataW(GetFindDataInfo* info) {
  PluginInstance* plugin = (PluginInstance*) info->hPanel;
  bool show_error = (info->OpMode & (OPM_SILENT | OPM_FIND | OPM_QUICKVIEW)) == 0;
  try {
    UiLink ui((info->OpMode & OPM_SILENT) != 0);
    PluginItemList file_list;
    plugin->last_object = plugin->current_dir;
    FileListOptions options;
    options.hide_rom_files = g_plugin_options.hide_rom_files;
    gen_file_list(plugin->current_dir, plugin->session, file_list, options, ui);
    plugin->file_lists += file_list;
    info->PanelItem = (PluginPanelItem*) plugin->file_lists.last().data();
    info->ItemsNumber = plugin->file_lists.last().size();
    if (!(info->OpMode & OPM_FIND)) NOFAIL(refresh_system_info(plugin));
    return TRUE;
  }
  HANDLE_ERROR(FALSE, TRUE);
}

void WINAPI FreeFindDataW(const FreeFindDataInfo* info) {
  PluginInstance* plugin = (PluginInstance*) info->hPanel;
  for (unsigned i = 0; i < plugin->file_lists.size(); i++) {
    if (plugin->file_lists[i].data() == info->PanelItem) {
      plugin->file_lists.remove(i);
      return;
    }
  }
  assert(false);
}

intptr_t WINAPI SetDirectoryW(const SetDirectoryInfo* info) {
  PluginInstance* plugin = (PluginInstance*) info->hPanel;
  bool show_error = (info->OpMode & (OPM_SILENT | OPM_FIND | OPM_QUICKVIEW)) == 0;
  try {
    return set_dir(plugin, info->Dir, show_error);
  }
  HANDLE_ERROR(FALSE, TRUE);
}

int set_dir(PluginInstance* plugin, const wchar_t* dir, bool show_error) {
  FilePath fp_dir(dir);
  FilePath current_dir(plugin->current_dir);
  bool parent_dir = wcscmp(dir, L"..") == 0;
  if (current_dir.is_root_path() && parent_dir) {
    if (g_plugin_options.exit_on_dot_dot) far_control_ptr(plugin, FCTL_CLOSEPANEL, nullptr);
    return TRUE;
  }
  else if (parent_dir) {
    current_dir = current_dir.get_partial_path(current_dir.size() - 1);
  }
  else {
    current_dir.combine(fp_dir);
  }
  std::string cd = current_dir.get_full_path();
  if (!current_dir.is_root_path()) {
    plugin->last_object = cd;
    if (!dir_exists(cd, plugin->session)) FAIL(SystemError(ERROR_PATH_NOT_FOUND));
  }
  plugin->current_dir = cd;
  return TRUE;
}

intptr_t get_files(HANDLE hPlugin, struct PluginPanelItem *PanelItem, size_t ItemsNumber, int Move, std::string& DestPath, OPERATION_MODES OpMode) {
  if ((ItemsNumber == 0) || (wcscmp(PanelItem[0].FileName, L"..") == 0)) return 1;
  PluginInstance* plugin = (PluginInstance*) hPlugin;
  bool show_dialog = (OpMode & (OPM_SILENT | OPM_FIND | OPM_VIEW | OPM_EDIT | OPM_QUICKVIEW)) == 0;
  bool show_error = (OpMode & (OPM_FIND | OPM_QUICKVIEW)) == 0;
  try {
    CopyFilesOptions options;
    options.show_dialog = show_dialog;
    options.show_error = show_error;
    options.dst_dir = DestPath;
    options.move_files = Move != 0;
    options.copy_shared = get_app_option(FSSF_SYSTEM, c_copy_opened_files_option, true);
    options.use_file_filters = false;
    options.use_tmp_files = (OpMode & (OPM_FIND | OPM_VIEW | OPM_QUICKVIEW)) != 0;
    if (show_dialog) {
      options.ignore_errors = g_plugin_options.ignore_errors;
      options.overwrite = g_plugin_options.overwrite;
      options.show_stats = g_plugin_options.show_stats;
      if (!show_copy_files_dlg(options, false)) BREAK;
      if (g_plugin_options.save_def_values) {
        g_plugin_options.ignore_errors = options.ignore_errors;
        g_plugin_options.overwrite = options.overwrite;
        g_plugin_options.show_stats = options.show_stats;
        save_def_option_values(g_plugin_options);
      }
      DestPath = options.dst_dir;
    }
    else {
      options.ignore_errors = false;
      options.overwrite = ooOverwrite;
      options.show_stats = ssoNever;
    }
    CopyFilesStats stats;
    Log log;
    {
      UiLink ui((OpMode & OPM_SILENT) != 0);
      if (ui.update_needed()) {
        draw_progress_msg(far_get_msg(MSG_PROGRESS_PREPARE));
      }

      // source directory
      std::string src_dir_path = plugin->current_dir;

      // distination directory and file name (if renaming)
      std::string dst_dir_path, dst_new_name;
      FilePath dst_fp(options.dst_dir);
      bool dst_is_remote = !dst_fp.is_absolute || (dst_fp.root.size() == 0);
      if (dst_is_remote) {
        // ensure that file name case is correct in source and destinations paths
        // it will be used later in comparison
        FilePath src_dir_fp(src_dir_path);
        find_real_file_path(src_dir_fp, plugin->session);
        src_dir_path = src_dir_fp.get_full_path();
        dst_fp = FilePath(plugin->current_dir).combine(dst_fp);
        find_real_file_path(dst_fp, plugin->session);
      }
      if ((dst_is_remote && dir_exists(dst_fp.get_full_path(), plugin->session)) || (!dst_is_remote && dir_exists(dst_fp.get_full_path()))) {
        dst_dir_path = dst_fp.get_full_path();
      }
      else {
        if (ItemsNumber != 1) {
          dst_dir_path = dst_fp.get_full_path();
        }
        else {
          dst_dir_path = dst_fp.get_dir_path();
          dst_new_name = dst_fp.get_file_name();
        }
      }

      std::string src_file_name, dst_file_name; // store source / destination file names
      std::string src_path, dst_path; // store source / destination file paths

      // list of selected files
      FileList panel_file_list;
      panel_items_to_file_list(PanelItem, ItemsNumber, panel_file_list);

      // verify that no file is copied into self
      if (dst_is_remote) {
        for (unsigned i = 0; i < ItemsNumber; i++) {
          src_file_name = panel_file_list[i].file_name;
          COMPOSE_PATH2(src_path, src_dir_path, src_file_name);
          if (dst_new_name.size() != 0) dst_file_name = dst_new_name; else dst_file_name = src_file_name;
          COMPOSE_PATH2(dst_path, dst_dir_path, dst_file_name);
          if (dst_path == src_path) FAIL(CustomError(far_get_msg(MSG_ERR_SELF_COPY), src_path));
        }
      }

      // make sure destination path exists
      if (dst_is_remote) prepare_target_path(dst_dir_path, plugin->session, plugin);
      else prepare_target_path(dst_dir_path, plugin);

      Array<unsigned> finished_idx; // indices of processed files

      // try to move files remotely
      // mark files that were processed successfully
      if (options.move_files && dst_is_remote) {
        // prepare progress data
        CopyFilesProgress progress;
        QueryPerformanceCounter((PLARGE_INTEGER) &progress.start_time);
        ui.force_update();
        // iterate through selected files
        for (unsigned i = 0; i < ItemsNumber; i++) {
          src_file_name = panel_file_list[i].file_name; // source file name
          COMPOSE_PATH2(src_path, src_dir_path, src_file_name); // source file path
          if (dst_new_name.size() != 0) dst_file_name = dst_new_name; else dst_file_name = src_file_name; // destination file name
          COMPOSE_PATH2(dst_path, dst_dir_path, dst_file_name); // destination file path
          // update progress bar if needed
          if (ui.update_needed()) {
            progress.src_path = src_path;
            progress.dst_path = dst_path;
            draw_move_remote_files_progress(progress, stats);
          }
          // try to move file remotely
          if (move_remote_file(src_path, dst_path, plugin->session)) {
            // update stats
            if (panel_file_list[i].is_dir()) stats.dirs++;
            else stats.files++;
            // add finished file to list
            finished_idx += i;
          }
        }
      }

      // scan source directories and prepare lists of files to process
      std::vector<FileList> file_lists;
      ui.force_update();
      CreateListStats list_stats;
      CreateListOptions list_options;
      list_options.ignore_errors = options.ignore_errors;
      list_options.show_error = options.show_error;
      try {
        for (unsigned i = 0; i < ItemsNumber; i++) {
          if (finished_idx.bsearch(i) == -1) {
            file_lists += create_file_list(src_dir_path, panel_file_list[i].file_name, list_stats, list_options, ui, log, plugin->session, plugin);
          }
          else file_lists += FileList(); // skip already moved objects
        }
      }
      finally (stats.errors = list_stats.errors);

      // show file filters dialog if needed
      std::vector<FilterInterface> filters;
      if (options.use_file_filters && !dst_is_remote && show_dialog) {
        load_file_filters();
        if (export_filter_list.size() != 0) {
          Array<FilterSelection> selection;
          if (!show_filters_dlg(export_filter_list, selection)) BREAK;
          for (unsigned i = 0; i < selection.size(); i++) {
            filters += FilterInterface(export_filter_list[selection[i].src_idx].src_ext, export_filter_list[selection[i].src_idx][selection[i].dst_idx].dst_ext, export_filter_list[selection[i].src_idx][selection[i].dst_idx].guid);
          }
        }
      }

      // perform copy
      CopyFilesProgress progress;
      progress.total_size = list_stats.size;
      progress.processed_total_size = progress.copied_total_size = 0;
      QueryPerformanceCounter((PLARGE_INTEGER) &progress.start_time);
      ui.force_update();
      AutoBuffer buffer(g_plugin_options.copy_buf_size);
      for (unsigned i = 0; i < ItemsNumber; i++) {
        if (finished_idx.bsearch(i) == -1) {
          copy_files(true, src_dir_path, file_lists[i], dst_is_remote, dst_dir_path, dst_new_name, stats, progress, options, ui, buffer, log, filters, plugin->session, plugin);
        }
        PanelItem[i].Flags &= ~PPIF_SELECTED;
      }

      // delete source files if moving (only if no errors or skipped files to prevent data loss)
      if (options.move_files && (stats.errors == 0) && (stats.skipped == 0)) {
        DeleteFilesStats del_stats;
        DeleteFilesOptions del_options;
        del_options.ignore_errors = options.ignore_errors;
        del_options.show_stats = options.show_stats;
        del_options.show_error = options.show_error;
        del_options.show_dialog = options.show_dialog;
        DeleteFilesProgress del_progress;
        del_progress.objects = 0;
        del_progress.total_objects = list_stats.files + list_stats.dirs;
        QueryPerformanceCounter((PLARGE_INTEGER) &del_progress.start_time);
        ui.force_update();
        try {
          for (unsigned i = 0; i < ItemsNumber; i++) {
            delete_files(true, src_dir_path, file_lists[i], del_stats, del_progress, del_options, ui, log, plugin->session, plugin);
          }
        }
        finally (stats.errors += del_stats.errors);
      }

      // set cursor to new file name after rename
      if (dst_is_remote && options.move_files && (src_dir_path == dst_dir_path)) {
        assert(dst_new_name.size() != 0);
        far_control_int(plugin, FCTL_UPDATEPANEL, 1);
        PanelInfo panel_info;
        far_control_ptr(plugin, FCTL_GETPANELINFO, &panel_info);
        PanelRedrawInfo redraw_info = { sizeof(PanelRedrawInfo) };
        redraw_info.TopPanelItem = panel_info.TopPanelItem;
        redraw_info.CurrentItem = panel_info.CurrentItem;
        for (size_t i = 0; i < panel_info.ItemsNumber; i++) {
          PluginPanelItem* ppi = far_get_panel_item(plugin, i, panel_info);
          std::string file_name = ppi->FileName;
          if (file_name == dst_new_name) {
            redraw_info.CurrentItem = i;
            break;
          }
        }
        far_control_ptr(INVALID_HANDLE_VALUE, FCTL_REDRAWPANEL, &redraw_info);
      }
    }

    if (show_dialog && ((options.show_stats == ssoAlways) || ((options.show_stats == ssoIfError) && (stats.errors != 0)))) show_copy_files_results_dlg(stats, log);
    return 1;
  }
  HANDLE_ERROR(0, -1);
}

intptr_t WINAPI GetFilesW(GetFilesInfo* info) {
  PluginInstance* plugin = (PluginInstance*) info->hPanel;
  std::string dest_path = info->DestPath;
  intptr_t res = get_files(info->hPanel, info->PanelItem, info->ItemsNumber, info->Move, dest_path, info->OpMode);
  plugin->dest_path_buf = dest_path;
  info->DestPath = plugin->dest_path_buf.data();
  return res;
}

intptr_t WINAPI PutFilesW(const PutFilesInfo* info) {
  if ((info->ItemsNumber == 0) || (wcscmp(info->PanelItem[0].FileName, L"..") == 0)) return 1;
  PluginInstance* plugin = (PluginInstance*) info->hPanel;
  bool show_dialog = (info->OpMode & (OPM_SILENT | OPM_FIND | OPM_VIEW | OPM_EDIT | OPM_QUICKVIEW)) == 0;
  bool show_error = (info->OpMode & (OPM_FIND | OPM_QUICKVIEW)) == 0;
  try {
    CopyFilesOptions options;
    options.show_dialog = show_dialog;
    options.show_error = show_error;
    options.dst_dir = plugin->current_dir;
    options.move_files = info->Move != 0;
    options.copy_shared = get_app_option(FSSF_SYSTEM, c_copy_opened_files_option, true);
    options.use_tmp_files = false;
    if (show_dialog) {
      options.ignore_errors = g_plugin_options.ignore_errors;
      options.overwrite = g_plugin_options.overwrite;
      options.show_stats = g_plugin_options.show_stats;
      options.use_file_filters = g_plugin_options.use_file_filters;
      if (!g_plugin_options.hide_copy_dlg) {
        if (!show_copy_files_dlg(options, true)) BREAK;
        if (g_plugin_options.save_def_values) {
          g_plugin_options.ignore_errors = options.ignore_errors;
          g_plugin_options.overwrite = options.overwrite;
          g_plugin_options.show_stats = options.show_stats;
          g_plugin_options.use_file_filters = options.use_file_filters;
          save_def_option_values(g_plugin_options);
        }
      }
    }
    else {
      options.ignore_errors = false;
      options.overwrite = ooOverwrite;
      options.show_stats = ssoNever;
      options.use_file_filters = false;
    }
    CopyFilesStats stats;
    Log log;
    {
      UiLink ui((info->OpMode & OPM_SILENT) != 0);
      if (ui.update_needed()) {
        draw_progress_msg(far_get_msg(MSG_PROGRESS_PREPARE));
      }

      // distination directory and file name (if renaming)
      std::string dst_dir_path, dst_new_name;
      FilePath dst_fp(plugin->current_dir);
      if (plugin->current_dir != options.dst_dir) {
        dst_fp.combine(options.dst_dir);
      }
      if (dir_exists(dst_fp.get_full_path(), plugin->session)) {
        dst_dir_path = dst_fp.get_full_path();
      }
      else {
        if (info->ItemsNumber != 1) {
          dst_dir_path = dst_fp.get_full_path();
        }
        else {
          dst_dir_path = dst_fp.get_dir_path();
          dst_new_name = dst_fp.get_file_name();
        }
        // make sure destination path exists
        prepare_target_path(dst_dir_path, plugin->session, plugin);
      }

      // list of selected files
      PanelFileList panel_file_list;
      file_panel_items_to_file_list(del_trailing_slash(info->SrcPath), info->PanelItem, info->ItemsNumber, panel_file_list, ui, plugin);

      // scan source directories and prepare lists of files to process
      std::vector<FileList> file_lists;
      ui.force_update();
      CreateListStats list_stats;
      CreateListOptions list_options;
      list_options.ignore_errors = options.ignore_errors;
      list_options.show_error = options.show_error;
      try {
        for (size_t i = 0; i < info->ItemsNumber; i++) {
          file_lists += create_file_list(panel_file_list[i].file_dir, panel_file_list[i].file_name, list_stats, list_options, ui, log, plugin);
        }
      }
      finally (stats.errors = list_stats.errors);

      // show file filters dialog if needed
      std::vector<FilterInterface> filters;
      if (options.use_file_filters && !g_plugin_options.hide_copy_dlg && show_dialog) {
        load_file_filters();
        if (import_filter_list.size() != 0) {
          Array<FilterSelection> selection;
          if (!show_filters_dlg(import_filter_list, selection)) BREAK;
          for (unsigned i = 0; i < selection.size(); i++) {
            filters += FilterInterface(import_filter_list[selection[i].src_idx].src_ext, import_filter_list[selection[i].src_idx][selection[i].dst_idx].dst_ext, import_filter_list[selection[i].src_idx][selection[i].dst_idx].guid);
          }
        }
      }

      // perform copy
      CopyFilesProgress progress;
      progress.total_size = list_stats.size;
      progress.processed_total_size = progress.copied_total_size = 0;
      QueryPerformanceCounter((PLARGE_INTEGER) &progress.start_time);
      ui.force_update();
      AutoBuffer buffer(g_plugin_options.copy_buf_size);
      for (size_t i = 0; i < info->ItemsNumber; i++) {
        copy_files(false, panel_file_list[i].file_dir, file_lists[i], true, dst_dir_path, dst_new_name, stats, progress, options, ui, buffer, log, filters, plugin->session, plugin);
        info->PanelItem[i].Flags &= ~PPIF_SELECTED;
      }

      // delete source files if moving (only if no errors or skipped files to prevent data loss)
      if (options.move_files && (stats.errors == 0) && (stats.skipped == 0)) {
        DeleteFilesStats del_stats;
        DeleteFilesOptions del_options;
        del_options.ignore_errors = options.ignore_errors;
        del_options.show_stats = options.show_stats;
        del_options.show_error = options.show_error;
        del_options.show_dialog = options.show_dialog;
        DeleteFilesProgress del_progress;
        del_progress.objects = 0;
        del_progress.total_objects = list_stats.files + list_stats.dirs;
        QueryPerformanceCounter((PLARGE_INTEGER) &del_progress.start_time);
        ui.force_update();
        try {
          for (size_t i = 0; i < info->ItemsNumber; i++) {
            delete_files(false, panel_file_list[i].file_dir, file_lists[i], del_stats, del_progress, del_options, ui, log, plugin->session, plugin);
          }
        }
        finally (stats.errors += del_stats.errors);
      }

    }
    if (show_dialog && ((options.show_stats == ssoAlways) || ((options.show_stats == ssoIfError) && (stats.errors != 0)))) show_copy_files_results_dlg(stats, log);
    return 1;
  }
  HANDLE_ERROR(0, -1);
}

intptr_t WINAPI DeleteFilesW(const DeleteFilesInfo* info) {
  if ((info->ItemsNumber == 0) || (wcscmp(info->PanelItem[0].FileName, L"..") == 0)) return TRUE;
  PluginInstance* plugin = (PluginInstance*) info->hPanel;
  bool show_dialog = (info->OpMode & (OPM_SILENT | OPM_FIND | OPM_VIEW | OPM_EDIT | OPM_QUICKVIEW)) == 0;
  bool show_error = (info->OpMode & (OPM_FIND | OPM_QUICKVIEW)) == 0;
  try {
    DeleteFilesOptions options;
    options.show_dialog = show_dialog;
    options.show_error = show_error;
    if (show_dialog) {
      options.ignore_errors = g_plugin_options.ignore_errors;
      options.show_stats = g_plugin_options.show_stats;
      if (!show_delete_files_dlg(options)) BREAK;
      if (g_plugin_options.save_def_values) {
        g_plugin_options.ignore_errors = options.ignore_errors;
        g_plugin_options.show_stats = options.show_stats;
        save_def_option_values(g_plugin_options);
      }
    }
    else {
      options.ignore_errors = false;
      options.show_stats = ssoNever;
    }
    DeleteFilesStats stats;
    Log log;
    {
      UiLink ui((info->OpMode & OPM_SILENT) != 0);
      if (ui.update_needed()) {
        draw_progress_msg(far_get_msg(MSG_PROGRESS_PREPARE));
      }

      std::string dst_dir_path = plugin->current_dir;

      // list of selected files
      FileList panel_file_list;
      panel_items_to_file_list(info->PanelItem, info->ItemsNumber, panel_file_list);

      // scan source directories and prepare lists of files to process
      std::vector<FileList> file_lists;
      ui.force_update();
      CreateListStats list_stats;
      CreateListOptions list_options;
      list_options.ignore_errors = options.ignore_errors;
      list_options.show_error = options.show_error;
      try {
        for (size_t i = 0; i < info->ItemsNumber; i++) {
          file_lists += create_file_list(dst_dir_path, panel_file_list[i].file_name, list_stats, list_options, ui, log, plugin->session, plugin);
        }
      }
      finally (stats.errors = list_stats.errors);

      // init progress data
      DeleteFilesProgress progress;
      progress.objects = 0;
      progress.total_objects = list_stats.files + list_stats.dirs;
      QueryPerformanceCounter((PLARGE_INTEGER) &progress.start_time);
      ui.force_update();
      // delete files
      for (size_t i = 0; i < info->ItemsNumber; i++) {
        delete_files(true, dst_dir_path, file_lists[i], stats, progress, options, ui, log, plugin->session, plugin);
        info->PanelItem[i].Flags &= ~PPIF_SELECTED;
      }
    }
    if (show_dialog && ((options.show_stats == ssoAlways) || ((options.show_stats == ssoIfError) && (stats.errors != 0)))) show_delete_files_results_dlg(stats, log);
    return TRUE;
  }
  HANDLE_ERROR(FALSE, TRUE);
}

intptr_t WINAPI MakeDirectoryW(MakeDirectoryInfo* info) {
  PluginInstance* plugin = (PluginInstance*) info->hPanel;
  bool show_dialog = (info->OpMode & (OPM_SILENT | OPM_FIND | OPM_VIEW | OPM_EDIT | OPM_QUICKVIEW)) == 0;
  bool show_error = (info->OpMode & (OPM_FIND | OPM_QUICKVIEW)) == 0;
  try {
    CreateDirOptions options;
    if (show_dialog) {
      if (!show_create_dir_dlg(options)) BREAK;
      FilePath fp_dir(options.file_name);
      std::string dir;
      if (fp_dir.is_absolute) {
        FilePath fp_cur_dir(plugin->current_dir);
        if ((fp_dir.size() > fp_cur_dir.size()) && fp_dir.equal(0, fp_cur_dir)) dir = fp_dir[fp_cur_dir.size()];
      }
      else {
        dir = fp_dir[0];
      }
      plugin->directory_name_buf = dir;
      info->Name = plugin->directory_name_buf.data();
    }
    create_dir(plugin->current_dir, options.file_name, plugin->session, plugin);
    return 1;
  }
  HANDLE_ERROR(0, -1);
}

intptr_t WINAPI ConfigureW(const ConfigureInfo* info) {
  if (show_plugin_options_dlg(g_plugin_options)) {
    save_plugin_options(g_plugin_options);
    return TRUE;
  }
  else return FALSE;
}

intptr_t WINAPI ProcessPanelInputW(const ProcessPanelInputInfo* info) {
  PluginInstance* plugin = (PluginInstance*) info->hPanel;
  bool show_error = true;
  try {
    if (info->Rec.EventType != KEY_EVENT)
      return FALSE;
    const KEY_EVENT_RECORD& key_event = info->Rec.Event.KeyEvent;
    std::string key_name;
    size_t sz = g_fsf.FarInputRecordToName(&info->Rec, nullptr, 0);
    if (sz != 0) {
      sz = g_fsf.FarInputRecordToName(&info->Rec, key_name.buf(sz), sz);
      key_name.set_size(sz - 1);
    }
    if (key_name == g_plugin_options.key_attr) {
      PanelInfo panel_info;
      if (far_control_ptr(info->hPanel, FCTL_GETPANELINFO, &panel_info)) {
        if (panel_info.SelectedItemsNumber != 0) {
          FileAttrOptions options;
          options.single_file = panel_info.SelectedItemsNumber == 1;
          options.attr_and = 0xFFFFFFFF;
          options.attr_or = 0;
          for (size_t i = 0; i < panel_info.SelectedItemsNumber; i++) {
            PluginPanelItem* ppi = far_get_selected_panel_item(info->hPanel, i, panel_info);
            options.attr_and &= ppi->FileAttributes;
            options.attr_or |= ppi->FileAttributes;
          }
          if (show_file_attr_dlg(options)) {
            if ((options.attr_and != 0xFFFFFFFF) || (options.attr_or != 0)) {
              std::string file_name;
              std::string path;
              g_far.PanelControl(info->hPanel, FCTL_BEGINSELECTION, 0, 0);
              try {
                PluginPanelItem* ppi = far_get_selected_panel_item(info->hPanel, 0, panel_info);
                bool sel_flag = (ppi->Flags & PPIF_SELECTED) != 0;
                for (size_t i = 0; i < panel_info.ItemsNumber; i++) {
                  PluginPanelItem* ppi = far_get_panel_item(info->hPanel, i, panel_info);
                  if ((sel_flag && ((ppi->Flags & PPIF_SELECTED) != 0)) || (!sel_flag && (i == panel_info.CurrentItem))) {
                    file_name = ppi->FileName;
                    DWORD attr = ppi->FileAttributes & options.attr_and | options.attr_or;
                    COMPOSE_PATH2(path, plugin->current_dir, file_name);
                    set_file_attr(path, attr, plugin->session, plugin);
                    g_far.PanelControl(info->hPanel, FCTL_SETSELECTION, i, FALSE);
                  }
                }
              }
              finally (
                g_far.PanelControl(info->hPanel, FCTL_ENDSELECTION, 0, nullptr);
                far_control_int(info->hPanel, FCTL_UPDATEPANEL, 1);
                far_control_ptr(info->hPanel, FCTL_REDRAWPANEL, nullptr);
              );
            }
          }
        }
      }
      return TRUE;
    }
    if (((key_event.wVirtualKeyCode == VK_F5) || (key_event.wVirtualKeyCode == VK_F6)) && (key_event.dwControlKeyState & SHIFT_PRESSED)) {
      PanelInfo panel_info;
      if (far_control_ptr(info->hPanel, FCTL_GETPANELINFO, &panel_info)) {
        if (panel_info.SelectedItemsNumber != 0) {
          PluginPanelItem* ppi = far_get_panel_item(info->hPanel, panel_info.CurrentItem, panel_info);
          std::string file_name = ppi->FileName;
          get_files(info->hPanel, ppi, 1, key_event.wVirtualKeyCode == VK_F6, file_name, 0);
          far_control_int(info->hPanel, FCTL_UPDATEPANEL, 1);
          far_control_ptr(info->hPanel, FCTL_REDRAWPANEL, nullptr);
        }
      }
      return TRUE;
    }
    if (key_name == g_plugin_options.key_execute) {
      PanelInfo panel_info;
      if (far_control_ptr(info->hPanel, FCTL_GETPANELINFO, &panel_info)) {
        if (panel_info.SelectedItemsNumber != 0) {
          PluginPanelItem* ppi = far_get_panel_item(info->hPanel, panel_info.CurrentItem, panel_info);
          std::string file_name = ppi->FileName;
          std::string cmd_line;
          try {
            cmd_line = get_open_command(plugin->current_dir, file_name, plugin->session);
          }
          catch (Error&) {
            cmd_line = COMPOSE_PATH(plugin->current_dir, file_name);
            if (cmd_line.search(L' ') != -1) cmd_line = L'"' + cmd_line + L'"';
          }
          RunOptions options;
          options.cmd_line = cmd_line;
          if (!show_run_dlg(options)) BREAK;
          if (options.cmd_line.size() == 0) FAIL(CustomError(far_get_msg(MSG_ERR_INVALID_CMD_LINE)));
          if (options.cmd_line != cmd_line) {
            cmd_line = options.cmd_line;
          }

          // extract app_name and params
          std::string app_name, params;
          if (cmd_line[0] == L'"') {
            unsigned pos = cmd_line.search(1, L'"');
            if (pos == -1) FAIL(CustomError(far_get_msg(MSG_ERR_INVALID_CMD_LINE)));
            app_name = cmd_line.slice(1, pos - 1).strip();
            params = cmd_line.slice(pos + 1, cmd_line.size() - pos - 1).strip();
          }
          else {
            unsigned pos = cmd_line.search(L' ');
            if (pos == -1) pos = cmd_line.size();
            app_name = cmd_line.slice(0, pos).strip();
            params = cmd_line.slice(pos, cmd_line.size() - pos).strip();
          }

          plugin->last_object = app_name;
          create_process(app_name, params, plugin->session);

          far_control_int(info->hPanel, FCTL_UPDATEPANEL, 1);
        }
      }
      return TRUE;
    }
    // toggle 'hide_rom_files' option
    if (key_name == g_plugin_options.key_hide_rom_files) {
      g_plugin_options.hide_rom_files = !g_plugin_options.hide_rom_files;
      far_control_int(info->hPanel, FCTL_UPDATEPANEL, 1);
      far_control_ptr(info->hPanel, FCTL_REDRAWPANEL, nullptr);
      return TRUE;
    }
    // open selected directory in Windows Explorer
    if ((key_event.wVirtualKeyCode == VK_RETURN) && (key_event.dwControlKeyState & SHIFT_PRESSED)) {
      PanelInfo panel_info;
      if (far_control_ptr(info->hPanel, FCTL_GETPANELINFO, &panel_info)) {
        std::string path;
        if (panel_info.SelectedItemsNumber == 0) {
          path = plugin->current_dir;
        }
        else {
          PluginPanelItem* ppi = far_get_panel_item(info->hPanel, panel_info.CurrentItem, panel_info);
          if ((ppi->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {
            path = COMPOSE_PATH(plugin->current_dir, ppi->FileName);
          }
          else {
            path = plugin->current_dir;
          }
        }
        std::string cmd_line = L"explorer.exe /root,,::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{49BF5420-FA7F-11cf-8011-00A0C90A8F78}\\" + path;
        PROCESS_INFORMATION pi;
        STARTUPINFOW si;
        memset(&si, 0, sizeof(STARTUPINFOW));
        si.cb = sizeof(STARTUPINFOW);
        CHECK_API(CreateProcessW(NULL, (LPWSTR) cmd_line.data(), NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &si, &pi) != 0);
        VERIFY(CloseHandle(pi.hProcess) != 0);
        VERIFY(CloseHandle(pi.hThread) != 0);
      }
      return TRUE;
    }
    return FALSE;
  }
  HANDLE_ERROR(TRUE, TRUE);
}

void WINAPI ExitFARW(const ExitInfo* info) {
  cleanup();
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  if (fdwReason == DLL_PROCESS_ATTACH) {
    g_h_module = hinstDLL;
  }
  return TRUE;
}
