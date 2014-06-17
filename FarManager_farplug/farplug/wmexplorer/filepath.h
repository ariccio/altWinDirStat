#ifndef _FILEPATH_H
#define _FILEPATH_H

class FilePath: public std::vector<std::string> {
private:
  void convert_path(const std::string& _path);
  void normalize();
public:
  bool is_absolute;
  std::string root;
  FilePath(const std::string& path) {
    convert_path(path);
  }
  FilePath(const FilePath& fp) {
    *this = fp;
  }
  FilePath& operator=(const FilePath& fp) {
    (*this).std::vector<std::string>::operator=(fp);
    is_absolute = fp.is_absolute;
    root = fp.root;
    return *this;
  }
  FilePath& operator=(const std::string& path) {
    convert_path(path);
    return *this;
  }
  bool is_root_path() const {
    return is_absolute && (size() == 0);
  }
  std::string get_dir_path() const {
    if (size() != 0) return get_partial_path(size() - 1);
    else return get_partial_path(0);
  }
  std::string get_file_name() const {
    if (size() != 0) return last();
    else return std::string();
  }
  std::string get_partial_path(unsigned num_part) const {
    std::string path;
    if (is_absolute) path = root + L'\\';
    for (unsigned i = 0; i < num_part; i++) {
      path += (*this)[i];
      if (i + 1 < num_part) path += L'\\';
    }
    return path;
  }
  std::string get_full_path() const {
    return get_partial_path(size());
  }
  FilePath& combine(const FilePath& fp) {
    if (fp.is_absolute) *this = fp;
    else {
      add(fp);
      normalize();
    }
    return *this;
  }
};

#endif // _FILEPATH_H
