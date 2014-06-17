/* class is a string */
#if defined(__A_ANSI_STRING__) || defined(__A_UNICODE_STRING__)
#  define __A_STRING__
#endif /* defined(__A_ANSI_STRING__) || defined(__A_UNICODE_STRING__) */

/* means need to use object copy constructors and desctructors */
#if defined(__A_OBJECT_ARRAY__)
#  define __A_OBJECT__
#endif /* defined(__A_OBJECT_ARRAY__) */

/* determine class name */
#ifdef __A_PLAIN_ARRAY__
#  define __A_CLASS__ Array
#endif /* __A_PLAIN_ARRAY__ */
#ifdef __A_OBJECT_ARRAY__
#  define __A_CLASS__ ObjectArray
#endif /* __A_OBJECT_ARRAY__ */
#ifdef __A_ANSI_STRING__
#  define __A_CLASS__ AnsiString
#endif /* __A_ANSI_STRING__ */
#ifdef __A_UNICODE_STRING__
#  define __A_CLASS__ UnicodeString
#endif /* __A_UNICODE_STRING__ */

/* type for indices and sizes */
#ifndef __A_IDXSZ_TYPE__
#  define __A_IDXSZ_TYPE__ size_t
#endif /* __A_IDXSZ_TYPE__ */

/* class declaration */
#ifdef __A_STRING__
class __A_CLASS__ {
public:
#  ifdef __A_ANSI_STRING__
  typedef char Char;
#  endif /* __A_ANSI_STRING__ */
#  ifdef __A_UNICODE_STRING__
  typedef wchar_t Char;
#  endif /* __A_UNICODE_STRING__ */
  typedef Char Item;
private:
  typedef __A_CLASS__ A;
#endif /* __A_STRING__ */

#if defined(__A_PLAIN_ARRAY__) || defined(__A_OBJECT_ARRAY__)
template<class Type> class __A_CLASS__ {
public:
  typedef Type Item;
private:
  typedef __A_CLASS__<Item> A;
#endif /* defined(__A_PLAIN_ARRAY__) || defined(__A_OBJECT_ARRAY__) */

public:
  typedef __A_IDXSZ_TYPE__ IdxSz;
private:
  struct Buffer {
    IdxSz size;
    IdxSz limit;
    IdxSz inc;
    mutable unsigned ref;
#ifdef DEBUG
    Item* d;
#endif /* DEBUG */
    Item* data() throw() {
      return (Item*) (this + 1);
    }
    void set_size(IdxSz new_size) throw() {
      assert(new_size <= limit);
      size = new_size;
#ifdef __A_STRING__
      data()[size] = 0;
#endif /* __A_STRING__ */
    }
  };
  Buffer* b;
public:
  __A_CLASS__(): b(create(NULL, 0)) {
  }
  __A_CLASS__(const A& arr) throw(): b(ref_create(arr)) {
  }
  explicit __A_CLASS__(const Item& item): b(create(&item, 1)) {
  }
  __A_CLASS__(const Item* data, IdxSz size): b(create(data, size)) {
  }
#ifdef __A_STRING__
  __A_CLASS__(const Item* str): b(create(str, strlen(str))) {
  }
#endif /* __A_STRING__ */
  ~__A_CLASS__() throw() {
    clean();
  }
private:
  /* calculate default buffer increment */
  static IdxSz auto_inc() throw() {
    if (sizeof(Item) == 1) return 64;
    else if (sizeof(Item) <= 2) return 32;
    else if (sizeof(Item) <= 4) return 16;
    else if (sizeof(Item) <= 8) return 8;
    else if (sizeof(Item) <= 16) return 4;
    else if (sizeof(Item) <= 32) return 2;
    else return 1;
  }
#ifdef __A_OBJECT__
  /* copy item */
  static void copy_item(Item* dst, const Item* src) {
    ::new(dst) Item(*src);
  }
  /* free item */
  static void clean_item(Item* dst) throw() {
    dst->~Item();
  }
#endif /* __A_OBJECT__ */
  /* create internal buffer and copy initial data */
  static Buffer* create(const Item* d, IdxSz dsize) {
    IdxSz new_limit = dsize;
#ifdef __A_STRING__
    Buffer* new_b = (Buffer*) ::malloc(sizeof(Buffer) + (new_limit + 1) * sizeof(Item));
#else /* __A_STRING__ */
    Buffer* new_b = (Buffer*) ::malloc(sizeof(Buffer) + new_limit * sizeof(Item));
#endif /* __A_STRING__ */
    if (new_b == NULL) throw std::bad_alloc();
    new_b->limit = new_limit;
    new_b->inc = auto_inc();
    new_b->ref = 0;
#ifdef DEBUG
    new_b->d = new_b->data();
#endif /* DEBUG */
#ifdef __A_OBJECT__
    IdxSz i;
    try {
      for (i = 0; i < dsize; i++) copy_item(new_b->data() + i, d + i);
    }
    catch (...) {
      while (i-- > 0) clean_item(new_b->data() + i);
      ::free(new_b);
      throw;
    }
#else /* __A_OBJECT__ */
    ::memcpy(new_b->data(), d, dsize * sizeof(Item));
#endif /* __A_OBJECT__ */
    new_b->set_size(dsize);
    return new_b;
  }
  /* create reference of the internal buffer */
  static Buffer* ref_create(const A& arr) throw() {
    Buffer* new_b = arr.b;
    new_b->ref++;
    return new_b;
  }
  /* copy object by reference */
  void ref_copy(const A& arr) {
    if (b != arr.b) { /* do not copy self */
      clean(); /* free existing items and internal buffer */
      b = arr.b;
      b->ref++;
    }
  }
  /* reallocate internal buffer */
  Buffer* alloc(IdxSz new_limit) {
    assert(b->ref == 0);
    assert(b->size <= new_limit);
#ifdef __A_STRING__
    Buffer* new_b = (Buffer*) ::realloc(b, sizeof(Buffer) + (new_limit + 1) * sizeof(Item));
#else /* __A_STRING__ */
    Buffer* new_b = (Buffer*) ::realloc(b, sizeof(Buffer) + new_limit * sizeof(Item));
#endif /* __A_STRING__ */
    if (new_b == NULL) throw std::bad_alloc();
    new_b->limit = new_limit;
#ifdef DEBUG
    new_b->d = new_b->data();
#endif /* DEBUG */
    return new_b;
  }
  /* allocate a copy of referenced internal buffer */
  Buffer* ref_alloc(IdxSz new_limit) {
    assert(b->ref != 0);
#ifdef __A_STRING__
    Buffer* new_b = (Buffer*) ::malloc(sizeof(Buffer) + (new_limit + 1) * sizeof(Item));
#else /* __A_STRING__ */
    Buffer* new_b = (Buffer*) ::malloc(sizeof(Buffer) + new_limit * sizeof(Item));
#endif /* __A_STRING__ */
    if (new_b == NULL) throw std::bad_alloc();
    new_b->size = 0;
    new_b->limit = new_limit;
    b->ref--;
    new_b->ref = 0;
    new_b->inc = b->inc;
#ifdef DEBUG
    new_b->d = new_b->data();
#endif /* DEBUG */
    return new_b;
  }
  /* change size of internal buffer */
  void resize(IdxSz new_limit) {
    b = alloc(new_limit);
  }
  /* change size of referenced internal buffer */
  void ref_resize(IdxSz new_limit) {
    assert(b->size <= new_limit);
    Buffer* new_b = ref_alloc(new_limit);
#ifdef __A_OBJECT__
    IdxSz i;
    try {
      for (i = 0; i < b->size; i++) copy_item(new_b->data() + i, b->data() + i);
    }
    catch (...) {
      while (i-- > 0) clean_item(new_b->data() + i);
      ::free(new_b);
      b->ref++;
      throw;
    }
#else /* __A_OBJECT__ */
    ::memcpy(new_b->data(), b->data(), b->size * sizeof(Item));
#endif /* __A_OBJECT__ */
    new_b->set_size(b->size);
    b = new_b;
  }
  /* make own copy of referenced buffer */
  void copy_if_ref() {
    if (b->ref != 0) ref_resize(b->limit);
  }
  /* free items and internal buffer */
  void clean() throw() {
    if (b->ref != 0) b->ref--;
    else {
#ifdef __A_OBJECT__
      for (IdxSz i = 0; i < b->size; i++) clean_item(b->data() + i);
#endif /* __A_OBJECT__ */
      ::free(b);
    }
  }
public:
  /* extend internal buffer */
  A& extend(IdxSz limit) {
    if (limit > b->limit) {
      if (b->ref != 0) ref_resize(limit);
      else resize(limit);
    }
    return *this;
  }
  /* compact internal buffer */
  A& compact() {
    if (b->size < b->limit) {
      if (b->ref != 0) ref_resize(b->size);
      else resize(b->size);
    }
    return *this;
  }
  /* item access */
  const Item& operator[](IdxSz index) const throw() {
    return citem(index);
  }
  const Item& citem(IdxSz index) const throw() {
    assert(index < b->size);
    return (const Item&) b->data()[index];
  }
  Item& item(IdxSz index) {
    assert(index < b->size);
    copy_if_ref();
    return (Item&) b->data()[index];
  }
  const Item& last() const throw() {
    return citem(b->size - 1);
  }
  Item& last_item() {
    return item(b->size - 1);
  }
  /* obtain direct access to internal buffer */
  Item* buf(IdxSz limit) {
    if (b->ref != 0) ref_resize(limit > b->limit ? limit : b->limit);
    else if (limit > b->limit) resize(limit);
    return b->data();
  }
  Item* buf() {
    return buf(b->size);
  }
  const Item* data() const throw() {
    return b->data();
  }
  /* size manipulation */
  IdxSz size() const throw() {
    return b->size;
  }
  A& set_size(IdxSz size) {
    if (size != b->size) {
      copy_if_ref();
      b->set_size(size);
    }
    return *this;
  }
#ifdef __A_STRING__
  /* special version for string buffers */
  A& set_size() throw() {
    return set_size(strlen(b->data()));
  }
#endif /* __A_STRING__ */
  /* set/get automatic buffer increment */
  IdxSz inc() const throw() {
    return b->inc;
  }
  A& set_inc(IdxSz inc) {
    if (inc != b->inc) {
      copy_if_ref();
      b->inc = inc;
    }
    return *this;
  }
  /* replace */
  A& replace(IdxSz pos, IdxSz size, const Item* d, IdxSz dsize) {
    assert(pos <= b->size);
    assert(size <= b->size);
    assert(pos + size <= b->size);
    if ((size == 0) && (dsize == 0)) return *this;
    if (b->ref != 0) {
      Buffer* new_b;
      IdxSz new_limit = b->size + dsize - size;
      if (new_limit > b->limit) {
        if (new_limit < b->limit + b->inc) new_limit = b->limit + b->inc;
        new_b = ref_alloc(new_limit);
      }
      else new_b = ref_alloc(b->limit);
#ifdef __A_OBJECT__
      /* complex arrays, referenced */
      IdxSz i = 0;
      try {
        for (; i < pos; i++) copy_item(new_b->data() + i, b->data() + i);
        for (; i < pos + dsize; i++) copy_item(new_b->data() + i, d + i - pos);
        for (; i < b->size + dsize - size; i++) copy_item(new_b->data() + i, b->data() + i + size - dsize);
      }
      catch (...) {
        while (i-- > 0) clean_item(new_b->data() + i);
        ::free(new_b);
        b->ref++;
        throw;
      }
#else /* __A_OBJECT__ */
      /* plain arrays and strings, referenced */
      ::memcpy(new_b->data(), b->data(), pos * sizeof(Item));
      ::memcpy(new_b->data() + pos, d, dsize * sizeof(Item));
      ::memcpy(new_b->data() + pos + dsize, b->data() + pos + size, (b->size - pos - size) * sizeof(Item));
#endif /* __A_OBJECT__ */
      new_b->set_size(b->size + dsize - size);
      b = new_b;
    }
    else {
#ifdef __A_OBJECT__
      /* complex arrays, not referenced */
      /* true if we copy data from self */
      bool self_copy = ((d >= b->data()) && (d < b->data() + b->size));
      IdxSz dpos;
      if (self_copy) {
        assert(d + dsize <= b->data() + b->size);
        dpos = (IdxSz) (d - b->data());
      }
      IdxSz new_limit = b->size + dsize;
      if (new_limit > b->limit) {
        if (new_limit < b->limit + b->inc) new_limit = b->limit + b->inc;
        b = alloc(new_limit);
      }
      if (dsize != 0) ::memmove(b->data() + pos + dsize, b->data() + pos, (b->size - pos) * sizeof(Item));
      IdxSz i = 0;
      try {
        if (self_copy) {
          for (; (i < dsize) && (i + dpos < pos); i++) copy_item(b->data() + pos + i, b->data() + dpos + i);
          for (; i < dsize; i++) copy_item(b->data() + pos + i, b->data() + dpos + dsize + i);
        }
        else {
          for (; i < dsize; i++) copy_item(b->data() + pos + i, d + i);
        }
      }
      catch (...) {
        while (i-- > 0) clean_item(b->data() + pos + i);
        if (dsize != 0) ::memmove(b->data() + pos, b->data() + pos + dsize, (b->size - pos) * sizeof(Item));
        throw;
      }
      for (i = 0; i < size; i++) clean_item(b->data() + pos + dsize + i);
      if (size != 0) ::memmove(b->data() + pos + dsize, b->data() + pos + dsize + size, (b->size - pos - size) * sizeof(Item));
#else /* __A_OBJECT__ */
      if ((d >= b->data()) && (d < b->data() + b->size)) { /* true if we copy data from self */
        /* plain arrays and strings, not referenced, self copy */
        assert(d + dsize <= b->data() + b->size);
        /* special case - no need to copy data */
        if ((pos + size == b->size) && (d == b->data() + pos)) {
        }
        else {
          IdxSz dpos = (IdxSz) (d - b->data());
          IdxSz new_limit = b->size + dsize;
          if (new_limit > b->limit) {
            if (new_limit < b->limit + b->inc) new_limit = b->limit + b->inc;
            b = alloc(new_limit);
          }
          if (dsize != 0) ::memmove(b->data() + pos + dsize, b->data() + pos, (b->size - pos) * sizeof(Item));
          if (dpos < pos) {
            if (dpos + dsize <= pos) ::memcpy(b->data() + pos, b->data() + dpos, dsize * sizeof(Item));
            else {
              ::memcpy(b->data() + pos, b->data() + dpos, (pos - dpos) * sizeof(Item));
              ::memcpy(b->data() + pos + (pos - dpos), b->data() + pos + dsize, (dsize - (pos - dpos)) * sizeof(Item));
            }
          }
          else ::memcpy(b->data() + pos, b->data() + dpos + dsize, dsize * sizeof(Item));
          if (size != 0) ::memmove(b->data() + pos + dsize, b->data() + pos + dsize + size, (b->size - pos - size) * sizeof(Item));
        }
      }
      else {
        /* plain arrays and strings, not referenced, normal copy */
        IdxSz new_limit = b->size + dsize - size;
        if (new_limit > b->limit) {
          if (new_limit < b->limit + b->inc) new_limit = b->limit + b->inc;
          b = alloc(new_limit);
        }
        if (dsize != size) ::memmove(b->data() + pos + dsize, b->data() + pos + size, (b->size - pos - size) * sizeof(Item));
        ::memcpy(b->data() + pos, d, dsize * sizeof(Item));
      }
#endif /* __A_OBJECT__ */
      b->set_size(b->size + dsize - size);
    }
    return *this;
  }
  A& replace(IdxSz pos, IdxSz size, const A& arr) {
    return replace(pos, size, arr.b->data(), arr.b->size);
  }
  A& replace(IdxSz pos, IdxSz size, const Item& item) {
    return replace(pos, size, &item, 1);
  }
#ifdef __A_STRING__
  A& replace(IdxSz pos, IdxSz size, const Item* str) {
    return replace(pos, size, str, strlen(str));
  }
#endif /* __A_STRING__ */
  /* add */
  A& add(const Item* d, IdxSz dsize) {
    return replace(b->size, 0, d, dsize);
  }
  A& add(const A& arr) {
    return add(arr.b->data(), arr.b->size);
  }
  A& add(const Item& item) {
    return add(&item, 1);
  }
#ifdef __A_STRING__
  A& add(const Item* str) {
    return add(str, strlen(str));
  }
#endif /* __A_STRING__ */
  /* operator += */
  A& operator+=(const Item& item) {
    return add(item);
  }
  A& operator+=(const A& arr) {
    return add(arr);
  }
#ifdef __A_STRING__
  A& operator+=(const Item* str) {
    return add(str);
  }
#endif /* __A_STRING__ */
  /* insert */
  A& insert(IdxSz pos, const Item* d, IdxSz dsize) {
    return replace(pos, 0, d, dsize);
  }
  A& insert(IdxSz pos, const A& arr) {
    return insert(pos, arr.b->data(), arr.b->size);
  }
  A& insert(IdxSz pos, const Item& item) {
    return insert(pos, &item, 1);
  }
#ifdef __A_STRING__
  A& insert(IdxSz pos, const Item* str) {
    return insert(pos, str, strlen(str));
  }
#endif /* __A_STRING__ */
  /* copy */
  A& copy(const Item* d, IdxSz dsize) {
    return replace(0, b->size, d, dsize);
  }
  A& copy(const A& arr) throw() {
    ref_copy(arr);
    return *this;
  }
  A& copy(const Item& item) {
    return copy(&item, 1);
  }
#ifdef __A_STRING__
  A& copy(const Item* str) {
    return copy(str, strlen(str));
  }
#endif /* __A_STRING__ */
  A& operator=(const A& arr) throw() {
    return copy(arr);
  }
  A& operator=(const Item& item) {
    return copy(item);
  }
#ifdef __A_STRING__
  A& operator=(const Item* str) {
    return copy(str);
  }
#endif /* __A_STRING__ */
  /* delete part of the array */
  A& remove(IdxSz pos, IdxSz size) {
    return replace(pos, size, NULL, 0);
  }
  A& remove(IdxSz pos) {
    return remove(pos, 1);
  }
  A& clear() {
    return remove(0, b->size);
  }
  /* sorting & binary search */
private:
  template<class Compare, class Value> static int __cdecl _cmp(const void* item1, const void* item2) throw() {
    return Compare()(*(const Value*) item1, *(const Item*) item2);
  }
  static int __cdecl _cmp_def(const void* item1, const void* item2) throw() {
    if (*(const Item*) item1 > *(const Item*) item2) return 1;
    else if (*(const Item*) item1 == *(const Item*) item2) return 0;
    else return -1;
  }
public:
  template<class Compare, class Value> IdxSz bsearch(const Value& item) const throw() {
    const Item* found = (const Item*) ::bsearch(&item, b->data(), b->size, sizeof(Item), _cmp<Compare, Value>);
    if (found != NULL) return (IdxSz) (found - b->data());
    else return (IdxSz) -1;
  }
  IdxSz bsearch(const Item& item) const throw() {
    const Item* found = (const Item*) ::bsearch(&item, b->data(), b->size, sizeof(Item), _cmp_def);
    if (found != NULL) return (IdxSz) (found - b->data());
    else return (IdxSz) -1;
  }
  template<class Compare> void sort() throw() {
    copy_if_ref();
    ::qsort(b->data(), b->size, sizeof(Item), _cmp<Compare, Item>);
  }
  void sort() throw() {
    copy_if_ref();
    ::qsort(b->data(), b->size, sizeof(Item), _cmp_def);
  }
  /* search */
  IdxSz search(IdxSz pos, IdxSz size, const Item* d, IdxSz dsize) const throw() {
    if (pos >= b->size) size = 0;
    else if ((size >= b->size) || (pos + size >= b->size)) size = b->size - pos;
    if (dsize > size) return (IdxSz) -1;
    for (IdxSz p = pos; p <= pos + size - dsize; p++) {
#ifdef __A_OBJECT_ARRAY__
      IdxSz i;
      for (i = 0; i < dsize; i++) {
        if (!(b->data()[p + i] == d[i])) break;
      }
      if (i == dsize) return p;
#else /* __A_OBJECT_ARRAY__ */
      if (memcmp(b->data() + p, d, dsize * sizeof(Item)) == 0) return p;
#endif /* __A_OBJECT_ARRAY__ */
    }
    return (IdxSz) -1;
  }
  IdxSz search(IdxSz pos, IdxSz size, const A& arr) const throw() {
    return search(pos, size, arr.b->data(), arr.b->size);
  }
  IdxSz search(IdxSz pos, IdxSz size, const Item& item) const throw() {
    return search(pos, size, &item, 1);
  }
#ifdef __A_STRING__
  IdxSz search(IdxSz pos, IdxSz size, const Item* str) const throw() {
    return search(pos, size, str, strlen(str));
  }
#endif /* __A_STRING__ */
  IdxSz search(IdxSz pos, const Item* d, IdxSz dsize) const throw() {
    return search(pos, b->size - pos, d, dsize);
  }
  IdxSz search(IdxSz pos, const A& arr) const throw() {
    return search(pos, b->size - pos, arr.b->data(), arr.b->size);
  }
  IdxSz search(IdxSz pos, const Item& item) const throw() {
    return search(pos, b->size - pos, &item, 1);
  }
#ifdef __A_STRING__
  IdxSz search(IdxSz pos, const Item* str) const throw() {
    return search(pos, b->size - pos, str, strlen(str));
  }
#endif /* __A_STRING__ */
  IdxSz search(const Item* d, IdxSz dsize) const throw() {
    return search(0, b->size, d, dsize);
  }
  IdxSz search(const A& arr) const throw() {
    return search(0, b->size, arr.b->data(), arr.b->size);
  }
  IdxSz search(const Item& item) const throw() {
    return search(0, b->size, &item, 1);
  }
#ifdef __A_STRING__
  IdxSz search(const Item* str) const throw() {
    return search(0, b->size, str, strlen(str));
  }
#endif /* __A_STRING__ */
  /* reverse search */
  IdxSz rsearch(IdxSz pos, IdxSz size, const Item* d, IdxSz dsize) const throw() {
    if (pos >= b->size) size = 0;
    else if ((size >= b->size) || (pos + size >= b->size)) size = b->size - pos;
    if (dsize > size) return (IdxSz) -1;
    for (IdxSz p = pos + size - dsize; p + 1 > pos; p--) {
#ifdef __A_OBJECT_ARRAY__
      IdxSz i;
      for (i = 0; i < dsize; i++) {
        if (!(b->data()[p + i] == d[i])) break;
      }
      if (i == dsize) return p;
#else /* __A_OBJECT_ARRAY__ */
      if (memcmp(b->data() + p, d, dsize * sizeof(Item)) == 0) return p;
#endif /* __A_OBJECT_ARRAY__ */
    }
    return (IdxSz) -1;
  }
  IdxSz rsearch(IdxSz pos, IdxSz size, const A& arr) const throw() {
    return rsearch(pos, size, arr.b->data(), arr.b->size);
  }
  IdxSz rsearch(IdxSz pos, IdxSz size, const Item& item) const throw() {
    return rsearch(pos, size, &item, 1);
  }
#ifdef __A_STRING__
  IdxSz rsearch(IdxSz pos, IdxSz size, const Item* str) const throw() {
    return rsearch(pos, size, str, strlen(str));
  }
#endif /* __A_STRING__ */
  IdxSz rsearch(IdxSz pos, const Item* d, IdxSz dsize) const throw() {
    return rsearch(pos, b->size - pos, d, dsize);
  }
  IdxSz rsearch(IdxSz pos, const A& arr) const throw() {
    return rsearch(pos, b->size - pos, arr.b->data(), arr.b->size);
  }
  IdxSz rsearch(IdxSz pos, const Item& item) const throw() {
    return rsearch(pos, b->size - pos, &item, 1);
  }
#ifdef __A_STRING__
  IdxSz rsearch(IdxSz pos, const Item* str) const throw() {
    return rsearch(pos, b->size - pos, str, strlen(str));
  }
#endif /* __A_STRING__ */
  IdxSz rsearch(const Item* d, IdxSz dsize) const throw() {
    return rsearch(0, b->size, d, dsize);
  }
  IdxSz rsearch(const A& arr) const throw() {
    return rsearch(0, b->size, arr.b->data(), arr.b->size);
  }
  IdxSz rsearch(const Item& item) const throw() {
    return rsearch(0, b->size, &item, 1);
  }
#ifdef __A_STRING__
  IdxSz rsearch(const Item* str) const throw() {
    return rsearch(0, b->size, str, strlen(str));
  }
#endif /* __A_STRING__ */
  /* equality */
  bool equal(IdxSz pos, IdxSz size, const Item* d, IdxSz dsize) const throw() {
    if (pos >= b->size) size = 0;
    else if ((size >= b->size) || (pos + size >= b->size)) size = b->size - pos;
    if (size != dsize) return false;
#ifdef __A_OBJECT_ARRAY__
    for (IdxSz p = 0; p < size; p++) {
      if (!(b->data()[pos + p] == d[p])) return false;
    }
    return true;
#else /* __A_OBJECT_ARRAY__ */
    return memcmp(b->data() + pos, d, size * sizeof(Item)) == 0;
#endif /* __A_OBJECT_ARRAY__ */
  }
  bool equal(IdxSz pos, IdxSz size, const A& arr) const throw() {
    return equal(pos, size, arr.b->data(), arr.b->size);
  }
  bool equal(IdxSz pos, IdxSz size, const Item& item) const throw() {
    return equal(pos, size, &item, 1);
  }
#ifdef __A_STRING__
  bool equal(IdxSz pos, IdxSz size, const Item* str) const throw() {
    return equal(pos, size, str, strlen(str));
  }
#endif /* __A_STRING__ */
  bool equal(IdxSz pos, const Item* d, IdxSz dsize) const throw() {
    return equal(pos, dsize, d, dsize);
  }
  bool equal(IdxSz pos, const A& arr) const throw() {
    return equal(pos, arr.b->size, arr.b->data(), arr.b->size);
  }
  bool equal(IdxSz pos, const Item& item) const throw() {
    return equal(pos, 1, &item, 1);
  }
#ifdef __A_STRING__
  bool equal(IdxSz pos, const Item* str) const throw() {
    return equal(pos, strlen(str), str, strlen(str));
  }
#endif /* __A_STRING__ */
  bool equal(const Item* d, IdxSz dsize) const throw() {
    return equal(0, b->size, d, dsize);
  }
  bool equal(const A& arr) const throw() {
    return equal(0, b->size, arr.b->data(), arr.b->size);
  }
  bool equal(const Item& item) const throw() {
    return equal(0, b->size, &item, 1);
  }
#ifdef __A_STRING__
  bool equal(const Item* str) const throw() {
    return equal(0, b->size, str, strlen(str));
  }
#endif /* __A_STRING__ */
  bool operator==(const A& arr) const throw() {
    return equal(arr);
  }
  bool operator==(const Item& item) const throw() {
    return equal(item);
  }
#ifdef __A_STRING__
  bool operator==(const Item* str) const throw() {
    return equal(str);
  }
#endif /* __A_STRING__ */
  bool operator!=(const A& arr) const throw() {
    return !equal(arr);
  }
  bool operator!=(const Item& item) const throw() {
    return !equal(item);
  }
#ifdef __A_STRING__
  bool operator!=(const Item* str) const throw() {
    return !equal(str, strlen(str));
  }
#endif /* __A_STRING__ */
  /* comparison */
  int compare(IdxSz pos, IdxSz size, const Item* d, IdxSz dsize) const throw() {
    if (pos >= b->size) size = 0;
    else if ((size >= b->size) || (pos + size >= b->size)) size = b->size - pos;
    IdxSz min_size = size < dsize ? size : dsize;
#ifdef __A_OBJECT_ARRAY__
    for (IdxSz p = 0; p < min_size; p++) {
      if (b->data()[pos + p] > d[p]) return 1;
      else if (!(b->data()[pos + p] == d[p])) return -1;
    }
#else /* __A_OBJECT_ARRAY__ */
    int res = memcmp(b->data() + pos, d, (size < dsize ? size : dsize) * sizeof(Item));
    if (res != 0) return res;
#endif /* __A_OBJECT_ARRAY__ */
    if (size > dsize) return 1;
    else if (size == dsize) return 0;
    else return -1;
  }
  int compare(IdxSz pos, IdxSz size, const A& arr) const throw() {
    return compare(pos, size, arr.b->data(), arr.b->size);
  }
  int compare(IdxSz pos, IdxSz size, const Item& item) const throw() {
    return compare(pos, size, &item, 1);
  }
#ifdef __A_STRING__
  int compare(IdxSz pos, IdxSz size, const Item* str) const throw() {
    return compare(pos, size, str, strlen(str));
  }
#endif /* __A_STRING__ */
  int compare(IdxSz pos, const Item* d, IdxSz dsize) const throw() {
    return compare(pos, dsize, d, dsize);
  }
  int compare(IdxSz pos, const A& arr) const throw() {
    return compare(pos, arr.b->size, arr.b->data(), arr.b->size);
  }
  int compare(IdxSz pos, const Item& item) const throw() {
    return compare(pos, 1, &item, 1);
  }
#ifdef __A_STRING__
  int compare(IdxSz pos, const Item* str) const throw() {
    return compare(pos, strlen(str), str, strlen(str));
  }
#endif /* __A_STRING__ */
  int compare(const Item* d, IdxSz dsize) const throw() {
    return compare(0, b->size, d, dsize);
  }
  int compare(const A& arr) const throw() {
    return compare(0, b->size, arr.b->data(), arr.b->size);
  }
  int compare(const Item& item) const throw() {
    return compare(0, b->size, &item, 1);
  }
#ifdef __A_STRING__
  int compare(const Item* str) const throw() {
    return compare(0, b->size, str, strlen(str));
  }
#endif /* __A_STRING__ */
  bool operator>(const A& arr) const throw() {
    return compare(arr) > 0;
  }
  bool operator>(const Item& item) const throw() {
    return compare(item) > 0;
  }
#ifdef __A_STRING__
  bool operator>(const Item* str) const throw() {
    return compare(str) > 0;
  }
#endif /* __A_STRING__ */
  bool operator<(const A& arr) const throw() {
    return compare(arr) < 0;
  }
  bool operator<(const Item& item) const throw() {
    return compare(item) < 0;
  }
#ifdef __A_STRING__
  bool operator<(const Item* str) const throw() {
    return compare(str) < 0;
  }
#endif /* __A_STRING__ */
  bool operator>=(const A& arr) const throw() {
    return compare(arr) >= 0;
  }
  bool operator>=(const Item& item) const throw() {
    return compare(item) >= 0;
  }
#ifdef __A_STRING__
  bool operator>=(const Item* str) const throw() {
    return compare(str) >= 0;
  }
#endif /* __A_STRING__ */
  bool operator<=(const A& arr) const throw() {
    return compare(arr) <= 0;
  }
  bool operator<=(const Item& item) const throw() {
    return compare(item) <= 0;
  }
#ifdef __A_STRING__
  bool operator<=(const Item* str) const throw() {
    return compare(str) <= 0;
  }
#endif /* __A_STRING__ */
  /* slices */
  A slice(IdxSz pos, IdxSz size) const {
    assert(pos <= b->size);
    assert(size <= b->size);
    assert(pos + size <= b->size);
    return A(b->data() + pos, size);
  }
  A slice(IdxSz pos) const {
    return slice(pos, b->size - pos);
  }
  A left(IdxSz size) const {
    return slice(0, size);
  }
  A right(IdxSz size) const {
    return slice(b->size - size, size);
  }
#ifdef __A_STRING__
  /* string format */
private:
#ifdef _MSC_VER
  typedef signed __int64 i64;
  typedef unsigned __int64 u64;
#else /* _MSC_VER */
  typedef signed long long i64;
  typedef unsigned long long u64;
#endif /* _MSC_VER */
  /*
    format specification
    %[<width>][.<precision>][<size>]<type>
    width = field width, excess space is filled with spaces
    precision = strings: max length
                decimal integers: min number of digits
    size = B (byte), H (short), I (int), L (__int64), default = int
    type = u (unsigned integer)
           i (signed integer)
           x (hexadecimal unsigned integer)
           b (binary unsigned integer)
           s (null terminated string)
           p (pointer)
           c (character)
           S (String)
  */
  A& do_format(const Item* fmt, va_list ap) {
    IdxSz index = 0; /* index into format string */
    Item ch; /* current character */

    while (true) {

      /* read format until % sign or end of string */
      unsigned cnt = 0;
      ch = fmt[index];
      while ((ch != '%') && (ch != 0)) {
        cnt++;
        ch = fmt[index + cnt];
      }
      add(fmt + index, cnt);
      index += cnt;

      /* if end of string reached */
      if (ch == 0) break;

      /* if % sign reached */
      if (ch == '%') {

        ch = fmt[++index];

        /* read flags if present */
        bool flag_left_align = false;
        bool flag_plus_sign = false;
        bool flag_add_blank = false;
        bool flag_zero_pad = false;
        bool flag_alt_form = false;
        while (true) {
          if (ch == '-') flag_left_align = true;
          else if (ch == '+') flag_plus_sign = true;
          else if (ch == ' ') flag_add_blank = true;
          else if (ch == '0') flag_zero_pad = true;
          else if (ch == '#') flag_alt_form = true;
          else break;
          ch = fmt[++index];
        }

        /* read width if present */
        unsigned width = 0;
        if (ch == '*') { /* width is taken from argument list */
          width = va_arg(ap, unsigned);
          ch = fmt[++index];
        }
        else {
          while ((ch >= '0') && (ch <= '9')) {
            width = width * 10 + ch - '0';
            ch = fmt[++index];
          }
        }

        /* read precision if present */
        unsigned precision;
        if (ch == '.') {
          ch = fmt[++index];
          if (ch == '*') {
            precision = va_arg(ap, unsigned);
            ch = fmt[++index];
          }
          else {
            precision = 0;
            while ((ch >= '0') && (ch <= '9')) {
              precision = precision * 10 + ch - '0';
              ch = fmt[++index];
            }
          }
        }
        else precision = (unsigned) -1;

        /* read parameter size if present */
        unsigned size;
        if (ch == 'B') size = 1;
        else if (ch == 'H') size = 2;
        else if (ch == 'I') size = 4;
        else if (ch == 'L') size = 8;
        else size = 0;
        /* if size present read next char */
        if (size != 0) ch = fmt[++index];

        /* these will be set to character representation of the argument after conversion */
        A ch_val;
        Item str_val[128];
        const Item* value;
        IdxSz value_len;

        /* null terminated string */
        if (ch == 's') {
          assert(size == 0);
          assert(!flag_plus_sign);
          assert(!flag_add_blank);
          assert(!flag_alt_form);
          value = va_arg(ap, const Item*);
          assert(value != NULL);
          value_len = strlen(value);
          if ((precision != (unsigned) -1) && (value_len > precision)) value_len = precision;
        }
        /* String class */
        else if (ch == 'S') {
          assert(size == 0);
          assert(!flag_plus_sign);
          assert(!flag_add_blank);
          assert(!flag_alt_form);
          const A* a = va_arg(ap, const A*);
          value = a->b->data();
          value_len = a->b->size;
          if ((precision != (unsigned) -1) && (value_len > precision)) value_len = precision;
        }
        /* character */
        else if (ch == 'c') {
          assert(size == 0);
          assert(!flag_plus_sign);
          assert(!flag_add_blank);
          assert(!flag_alt_form);
          Item c = (Item) va_arg(ap, int);
          if (precision == (unsigned) -1) precision = 1;
          ch_val.extend(precision);
          Item* ch_val_buf = ch_val.buf();
          for (unsigned i = 0; i < precision; i++) ch_val_buf[i] = c;
          ch_val.set_size(precision);
          value = ch_val.data();
          value_len = ch_val.size();
        }
        /* unsigned integer */
        else if ((ch == 'u') || (ch == 'i') || (ch == 'x') || (ch == 'b') || (ch == 'p')) {
          assert(!flag_left_align || !flag_zero_pad);
          u64 number;
          bool negative = false; /* true for negative values */
          if (size == 0) size = 4; /* set default size */
          if (ch == 'i') {
            i64 n;
            if (size <= 4) n = va_arg(ap, int);
            else if (size == 8) n = va_arg(ap, i64);
            else assert(false);
            if (n < 0) {
              negative = true;
              number = -n;
            }
            else number = n;
          }
          /* handle pointer as hexadecimal integer */
          else if (ch == 'p') {
            assert(!flag_plus_sign);
            assert(!flag_add_blank);
            if (sizeof(void*) == sizeof(unsigned)) number = va_arg(ap, unsigned);
            else if (sizeof(void*) == sizeof(u64)) number = va_arg(ap, u64);
            else assert(false);
            ch = 'x';
          }
          else {
            assert(!flag_plus_sign);
            assert(!flag_add_blank);
            if (size <= 4) number = va_arg(ap, unsigned);
            else if (size == 8) number = va_arg(ap, u64);
            else assert(false);
          }

          /* value radix */
          u64 base = 10;
          if (ch == 'x') base = 16;
          else if (ch == 'b') base = 2;

          /* convert number into string */
          const unsigned str_size = sizeof(str_val) / sizeof(str_val[0]);
          unsigned idx = str_size;
          do {
            Item d = static_cast<Item>(number % base);
            if (d < 10) d += '0';
            else d += 'A' - 10;
            str_val[--idx] = d;
            number /= base;
          }
          while (number != 0);

          /* zero pad according to precision */
          if (precision == (unsigned) -1) {
             /* calculate default precision values */
            if (ch == 'x') precision = 2 * size;
            else if (ch == 'b') precision = 8 * size;
            else precision = 1;
          }
          while (str_size - idx < precision) str_val[--idx] = '0';

          /* add prefix */
          if (ch == 'x') {
            if (flag_alt_form) {
              str_val[--idx] = 'x';
              str_val[--idx] = '0';
            }
          }
          else assert(!flag_alt_form);

          /* add minus sign */
          if (negative) {
            str_val[--idx] = '-';
          }
          else if (flag_plus_sign) {
            str_val[--idx] = '+';
            assert(!flag_add_blank);
          }
          else if (flag_add_blank) {
            str_val[--idx] = ' ';
          }

          value = str_val + idx;
          value_len = str_size - idx;
        }
        else if (ch == '%') {
          value = &ch;
          value_len = 1;
        }
        else {
          assert(false);
          value = &ch;
          value_len = 1;
        }

        /* pad to width */
        if (value_len < width) {
          extend(b->size + width);
          if (flag_left_align) {
            assert(!flag_zero_pad);
            add(value, value_len);
            for (IdxSz i = value_len; i < width; i++) add(' ');
          }
          else {
            for (IdxSz i = value_len; i < width; i++) add(flag_zero_pad ? '0' : ' ');
            add(value, value_len);
          }
        }
        else add(value, value_len);

      }
      index++;
    }
    return *this;
  }
public:
  A& replace_fmt(IdxSz pos, IdxSz size, const Item* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    replace(pos, size, A().do_format(fmt, ap));
    va_end(ap);
    return *this;
  }
  A& insert_fmt(IdxSz pos, const Item* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    insert(pos, A().do_format(fmt, ap));
    va_end(ap);
    return *this;
  }
  A& add_fmt(const Item* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    do_format(fmt, ap);
    va_end(ap);
    return *this;
  }
  A& copy_fmt(const Item* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    clear();
    do_format(fmt, ap);
    va_end(ap);
    return *this;
  }
  static A format(const Item* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    A output;
    output.do_format(fmt, ap);
    va_end(ap);
    return output;
  }
  /* case insensitive string comparison */
  int icompare(IdxSz pos, IdxSz size, const Item* d, IdxSz dsize) const throw() {
    if (pos >= b->size) size = 0;
    else if ((size >= b->size) || (pos + size >= b->size)) size = b->size - pos;
#ifdef __A_ANSI_STRING__
    int res = _strnicmp(b->data() + pos, d, size < dsize ? size : dsize);
#endif /* __A_ANSI_STRING__ */
#ifdef __A_UNICODE_STRING__
    int res = _wcsnicmp(b->data() + pos, d, size < dsize ? size : dsize);
#endif /* __A_UNICODE_STRING__ */
    if (res != 0) return res;
    else if (size > dsize) return 1;
    else if (size == dsize) return 0;
    else return -1;
  }
  int icompare(IdxSz pos, IdxSz size, const A& arr) const throw() {
    return icompare(pos, size, arr.b->data(), arr.b->size);
  }
  int icompare(IdxSz pos, IdxSz size, const Item& item) const throw() {
    return icompare(pos, size, &item, 1);
  }
  int icompare(IdxSz pos, IdxSz size, const Item* str) const throw() {
    return icompare(pos, size, str, strlen(str));
  }
  int icompare(IdxSz pos, const Item* d, IdxSz dsize) const throw() {
    return icompare(pos, dsize, d, dsize);
  }
  int icompare(IdxSz pos, const A& arr) const throw() {
    return icompare(pos, arr.b->size, arr.b->data(), arr.b->size);
  }
  int icompare(IdxSz pos, const Item& item) const throw() {
    return icompare(pos, 1, &item, 1);
  }
  int icompare(IdxSz pos, const Item* str) const throw() {
    return icompare(pos, strlen(str), str, strlen(str));
  }
  int icompare(const Item* d, IdxSz dsize) const throw() {
    return icompare(0, b->size, d, dsize);
  }
  int icompare(const A& arr) const throw() {
    return icompare(0, b->size, arr.b->data(), arr.b->size);
  }
  int icompare(const Item& item) const throw() {
    return icompare(0, b->size, &item, 1);
  }
  int icompare(const Item* str) const throw() {
    return icompare(0, b->size, str, strlen(str));
  }
  /* encode string in C-style */
private:
  static Char hex(Char n) {
    return n < 10 ? n + '0' : n - 10 + 'A';
  }
public:
  A c_encode() const {
    A output;
    output.extend(b->size);
    for (IdxSz i = 0; i < b->size; i++) {
      if (((b->data()[i] >= 0x00) && (b->data()[i] <= 0x1F)) || (b->data()[i] == 0x7F)) {
        output.add('\\');
        switch (b->data()[i]) {
        case '\a':
          output.add('a');
          break;
        case '\b':
          output.add('b');
          break;
        case '\f':
          output.add('f');
          break;
        case '\n':
          output.add('n');
          break;
        case '\r':
          output.add('r');
          break;
        case '\t':
          output.add('t');
          break;
        case '\v':
          output.add('v');
          break;
        default:
          output.add('x');
          output.add(hex((b->data()[i] >> 4) & 0x0F));
          output.add(hex(b->data()[i] & 0x0F));
          break;
        }
      }
      else {
        output.add(b->data()[i]);
      }
    }
    return output;
  }
  /* strip */
  A& strip() {
    IdxSz hp = 0;
    IdxSz tp = b->size;
    while ((hp < b->size) && (((unsigned) b->data()[hp] <= 0x20) || (b->data()[hp] == 0x7F))) hp++;
    if (hp < b->size) while (((unsigned) b->data()[tp - 1] <= 0x20) || (b->data()[tp - 1] == 0x7F)) tp--;
    copy(b->data() + hp, tp - hp);
    return *this;
  }
  /* C-string length */
private:
  static IdxSz strlen(const Char* str) throw() {
#  ifdef __A_ANSI_STRING__
    return (IdxSz) ::strlen((const char*) str);
#  endif /* __A_ANSI_STRING__ */
#  ifdef __A_UNICODE_STRING__
    return (IdxSz) ::wcslen(str);
#  endif /* __A_UNICODE_STRING__ */
  }
#endif /* __A_STRING__ */
};

/* operators */

#ifdef __A_STRING__
inline __A_CLASS__ operator+(const __A_CLASS__& a, const __A_CLASS__& b) {
  return __A_CLASS__(a).add(b);
}
inline __A_CLASS__ operator+(const __A_CLASS__& a, const __A_CLASS__::Char& b) {
  return __A_CLASS__(a).add(b);
}
inline __A_CLASS__ operator+(const __A_CLASS__::Char& a, const __A_CLASS__& b) {
  return __A_CLASS__(a).add(b);
}
inline __A_CLASS__ operator+(const __A_CLASS__& a, const __A_CLASS__::Char* b) {
  return __A_CLASS__(a).add(b);
}
inline __A_CLASS__ operator+(const __A_CLASS__::Char* a, const __A_CLASS__& b) {
  return __A_CLASS__(a).add(b);
}
inline bool operator==(const __A_CLASS__::Char& a, const __A_CLASS__& b) throw() {
  return b == a;
}
inline bool operator!=(const __A_CLASS__::Char& a, const __A_CLASS__& b) throw() {
  return b != a;
}
inline bool operator>(const __A_CLASS__::Char& a, const __A_CLASS__& b) throw() {
  return b < a;
}
inline bool operator<(const __A_CLASS__::Char& a, const __A_CLASS__& b) throw() {
  return b > a;
}
inline bool operator>=(const __A_CLASS__::Char& a, const __A_CLASS__& b) throw() {
  return b <= a;
}
inline bool operator<=(const __A_CLASS__::Char& a, const __A_CLASS__& b) throw() {
  return b >= a;
}
inline bool operator==(const __A_CLASS__::Char* a, const __A_CLASS__& b) throw() {
  return b == a;
}
inline bool operator!=(const __A_CLASS__::Char* a, const __A_CLASS__& b) throw() {
  return b != a;
}
inline bool operator>(const __A_CLASS__::Char* a, const __A_CLASS__& b) throw() {
  return b < a;
}
inline bool operator<(const __A_CLASS__::Char* a, const __A_CLASS__& b) throw() {
  return b > a;
}
inline bool operator>=(const __A_CLASS__::Char* a, const __A_CLASS__& b) throw() {
  return b <= a;
}
inline bool operator<=(const __A_CLASS__::Char* a, const __A_CLASS__& b) throw() {
  return b >= a;
}
#endif /* __A_STRING__ */

#if defined(__A_PLAIN_ARRAY__) || defined(__A_OBJECT_ARRAY__)
template<class Item> __A_CLASS__<Item> operator+(const __A_CLASS__<Item>& a, const __A_CLASS__<Item>& b) {
  return __A_CLASS__<Item>(a).add(b);
}
template<class Item> __A_CLASS__<Item> operator+(const __A_CLASS__<Item>& a, const Item& b) {
  return __A_CLASS__<Item>(a).add(b);
}
template<class Item> __A_CLASS__<Item> operator+(const Item& a, const __A_CLASS__<Item>& b) {
  return __A_CLASS__<Item>(a).add(b);
}
template<class Item> bool operator==(const Item& a, const __A_CLASS__<Item>& b) throw() {
  return b == a;
}
template<class Item> bool operator!=(const Item& a, const __A_CLASS__<Item>& b) throw() {
  return b != a;
}
#endif /* defined(__A_PLAIN_ARRAY__) || defined(__A_OBJECT_ARRAY__) */

#ifdef __A_STRING__
#  undef __A_STRING__
#endif /* __A_STRING__ */
#ifdef __A_OBJECT__
#  undef __A_OBJECT__
#endif /* __A_OBJECT__ */
#undef __A_CLASS__
