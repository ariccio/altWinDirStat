#include "filepath.h"

void FilePath::convert_path(const std::string& _path) {
  std::string path = _path;
  root.clear();
  clear();
  if (path.size() == 0) {
    path = '.'; // assume current directory
  }
  unsigned i = 0;
  while (i < path.size()) {
    // convert forward slashes into backslashes
    if (path[i] == L'/') path.item(i) = L'\\';
    // remove duplicate slashes (except \\ at path start)
    if ((path[i] == L'\\') && (i > 1) && (path[i - 1] == L'\\')) path.remove(i);
    else i++;
  }
  // determine if absoulte path or relative path
  if ((path.size() >= 2) && (path[1] == L':')) is_absolute = true;
  else if ((path.size() != 0) && (path[0] == L'\\')) is_absolute = true;
  else is_absolute = false;
  // remove slash from the end of path
  if ((path.size() > 0) && (path.last() == L'\\')) path.remove(path.size() - 1);
  unsigned p1 = 0;
  if (is_absolute) {
    // extract path root
    if ((path.size() >= 2) && (path[0] == L'\\') && (path[1] == L'\\')) {
      // if network path
      p1 = path.search(2, L'\\'); // share
      if (p1 == -1) p1 = path.size();
      p1 = path.search(p1 + 1, L'\\'); // server path
      if (p1 == -1) p1 = path.size();
    }
    else {
      p1 = path.search(L'\\');
      if (p1 == -1) p1 = path.size();
    }
    root = path.left(p1);
    p1++;
  }
  // split rest of the path into parts
  while (p1 < path.size()) {
    unsigned p2 = path.search(p1, L'\\');
    if (p2 == -1) p2 = path.size();
    add(path.slice(p1, p2 - p1));
    p1 = p2 + 1;
  }
  normalize();
}

void FilePath::normalize() {
  unsigned i = 0;
  while (i < size()) {
    if ((*this)[i] == L".") {
      if (i != 0) remove(i);
      else i++;
    }
    else if ((*this)[i] == L"..") {
      if ((i != 0) && ((*this)[i - 1] != L"..")) {
        remove(i);
        i--;
        remove(i);
      }
      else i++;
    }
    else i++;
  }
}
