// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// An iterator yields a sequence of key/value pairs from a source.
// The following class defines the interface.  Multiple implementations
// are provided by this library.  In particular, iterators are provided
// to access the contents of a Table or a DB.
//
// Multiple threads can invoke const methods on an Iterator without
// external synchronization, but if any of the threads may call a
// non-const method, all threads accessing the same Iterator must use
// external synchronization.

#ifndef STORAGE_LEVELDB_INCLUDE_ITERATOR_H_
#define STORAGE_LEVELDB_INCLUDE_ITERATOR_H_

#include "leveldb/export.h"
#include "leveldb/slice.h"
#include "leveldb/status.h"

namespace leveldb {

class LEVELDB_EXPORT Iterator {
 public:
  Iterator();

  Iterator(const Iterator&) = delete;
  Iterator& operator=(const Iterator&) = delete;

  virtual ~Iterator();

  // An iterator is either positioned at a key/value pair, or
  // not valid.  This method returns true iff the iterator is valid.
  virtual bool Valid() const = 0;

  // Position at the first key in the source.  The iterator is Valid()
  // after this call iff the source is not empty.
  virtual void SeekToFirst() = 0;

  // Position at the last key in the source.  The iterator is
  // Valid() after this call iff the source is not empty.
  virtual void SeekToLast() = 0;

  // Position at the first key in the source that is at or past target.
  // The iterator is Valid() after this call iff the source contains
  // an entry that comes at or past target.
  virtual void Seek(const Slice& target) = 0;

  // Moves to the next entry in the source.  After this call, Valid() is
  // true iff the iterator was not positioned at the last entry in the source.
  // REQUIRES: Valid()
  virtual void Next() = 0;

  // Moves to the previous entry in the source.  After this call, Valid() is
  // true iff the iterator was not positioned at the first entry in source.
  // REQUIRES: Valid()
  virtual void Prev() = 0;

  // Return the key for the current entry.  The underlying storage for
  // the returned slice is valid only until the next modification of
  // the iterator.
  // REQUIRES: Valid()
  virtual Slice key() const = 0;

  // Return the value for the current entry.  The underlying storage for
  // the returned slice is valid only until the next modification of
  // the iterator.
  // REQUIRES: Valid()
  virtual Slice value() const = 0;

  // If an error has occurred, return it.  Else return an ok status.
  virtual Status status() const = 0;

  // Clients are allowed to register function/arg1/arg2 triples that
  // will be invoked when this iterator is destroyed.
  //
  // Note that unlike all of the preceding methods, this method is
  // not abstract and therefore clients should not override it.
  using CleanupFunction = void (*)(void* arg1, void* arg2);
  // 我们允许客户端注册 CleanupFunction 类型的回调函数, 在迭代器被销毁的时候会调用它们(可以注册多个). 
  // 注意, 跟前面的方法不同, RegisterCleanup 不是抽象的, 客户端不应该覆写他们. 
  void RegisterCleanup(CleanupFunction function, void* arg1, void* arg2);

 private:
  // Cleanup functions are stored in a single-linked list.
  // The list's head node is inlined in the iterator.
  // 该类用于保存用户注册的清理函数, 一个清理函数对应一个该类对象, 全部对象被维护在一个单向链表上. 
  struct CleanupNode {
    // The head node is used if the function pointer is not null.
    CleanupFunction function;
    void* arg1;
    void* arg2;
    CleanupNode* next;

    // True if the node is not used. Only head nodes might be unused.
    bool IsEmpty() const { return function == nullptr; }
    // Invokes the cleanup function.
    // 运行之前调用 Iterator::RegisterCleanup 注册的清理函数
    void Run() { assert(function != nullptr); (*function)(arg1, arg2); }
  };
  CleanupNode cleanup_head_;
};

// Return an empty iterator (yields nothing).
LEVELDB_EXPORT Iterator* NewEmptyIterator();

// Return an empty iterator with the specified status.
// 基于指定状态返回一个空迭代器
LEVELDB_EXPORT Iterator* NewErrorIterator(const Status& status);

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_INCLUDE_ITERATOR_H_
