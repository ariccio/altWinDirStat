#ifndef _FILE_FILTERS_H
#define _FILE_FILTERS_H

struct Filter {
  std::string dst_ext;
  std::string description;
  std::string guid;
};

struct FileFilters: public std::vector<Filter> {
  std::string src_ext;
  bool operator==(const FileFilters& filters) const {
    return src_ext == filters.src_ext;
  }
  bool operator>(const FileFilters& filters) const {
    return src_ext > filters.src_ext;
  }
};

struct FilterInterface {
  std::string src_ext;
  std::string dst_ext;
  ICeFileFilter* itf;
  FilterInterface(const std::string& src_ext, const std::string& dst_ext, const std::string& guid_str);
  FilterInterface(const FilterInterface& filter_itf);
  ~FilterInterface();
  FilterInterface& operator=(const FilterInterface& filter_itf);
};

extern std::vector<FileFilters> export_filter_list;
extern std::vector<FileFilters> import_filter_list;

void load_file_filters();
void convert_file(ICeFileFilter* filter_itf, const std::string& src_file_name, const std::string& dst_file_name, bool import);

#endif // _FILE_FILTERS_H
