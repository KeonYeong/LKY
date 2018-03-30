//  Copyright (c) 2011-present, Facebook, Inc.  All rights reserved.
//  This source code is licensed under both the GPLv2 (found in the
//  COPYING file in the root directory) and Apache 2.0 License
//  (found in the LICENSE.Apache file in the root directory).

#ifndef ROCKSDB_LITE

#include "utilities/transactions/write_prepared_txn.h"

#include <map>

#include "db/column_family.h"
#include "db/db_impl.h"
#include "rocksdb/db.h"
#include "rocksdb/status.h"
#include "rocksdb/utilities/transaction_db.h"
#include "utilities/transactions/pessimistic_transaction.h"
#include "utilities/transactions/pessimistic_transaction_db.h"

namespace rocksdb {

struct WriteOptions;

WritePreparedTxn::WritePreparedTxn(WritePreparedTxnDB* txn_db,
                                   const WriteOptions& write_options,
                                   const TransactionOptions& txn_options)
    : PessimisticTransaction(txn_db, write_options, txn_options),
      wpt_db_(txn_db) {
  PessimisticTransaction::Initialize(txn_options);
}

Status WritePreparedTxn::Get(const ReadOptions& read_options,
                             ColumnFamilyHandle* column_family,
                             const Slice& key, PinnableSlice* pinnable_val) {
  auto snapshot = GetSnapshot();
  auto snap_seq =
      snapshot != nullptr ? snapshot->GetSequenceNumber() : kMaxSequenceNumber;

  WritePreparedTxnReadCallback callback(wpt_db_, snap_seq);
  return write_batch_.GetFromBatchAndDB(db_, read_options, column_family, key,
                                        pinnable_val, &callback);
}

Status WritePreparedTxn::PrepareInternal() {
  WriteOptions write_options = write_options_;
  write_options.disableWAL = false;
  WriteBatchInternal::MarkEndPrepare(GetWriteBatch()->GetWriteBatch(), name_);
  const bool disable_memtable = true;
  uint64_t seq_used;
  Status s =
      db_impl_->WriteImpl(write_options, GetWriteBatch()->GetWriteBatch(),
                          /*callback*/ nullptr, &log_number_, /*log ref*/ 0,
                          !disable_memtable, &seq_used);
  prepare_seq_ = seq_used;
  wpt_db_->AddPrepared(prepare_seq_);
  return s;
}

Status WritePreparedTxn::CommitWithoutPrepareInternal() {
  return CommitBatchInternal(GetWriteBatch()->GetWriteBatch());
}

Status WritePreparedTxn::CommitBatchInternal(WriteBatch* batch) {
  const bool disable_memtable = true;
  const uint64_t no_log_ref = 0;
  uint64_t seq_used;
  auto s = db_impl_->WriteImpl(write_options_, batch, nullptr, nullptr,
                               no_log_ref, !disable_memtable, &seq_used);
  uint64_t& prepare_seq = seq_used;
  uint64_t& commit_seq = seq_used;
  // TODO(myabandeh): skip AddPrepared
  wpt_db_->AddPrepared(prepare_seq);
  wpt_db_->AddCommitted(prepare_seq, commit_seq);
  return s;
}

Status WritePreparedTxn::CommitInternal() {
  // We take the commit-time batch and append the Commit marker.
  // The Memtable will ignore the Commit marker in non-recovery mode
  WriteBatch* working_batch = GetCommitTimeWriteBatch();
  // TODO(myabandeh): prevent the users from writing to txn after the prepare
  // phase
  assert(working_batch->Count() == 0);
  WriteBatchInternal::MarkCommit(working_batch, name_);

  // any operations appended to this working_batch will be ignored from WAL
  working_batch->MarkWalTerminationPoint();

  const bool disable_memtable = true;
  uint64_t seq_used;
  auto s = db_impl_->WriteImpl(write_options_, working_batch, nullptr, nullptr,
                               log_number_, disable_memtable, &seq_used);
  uint64_t& commit_seq = seq_used;
  // TODO(myabandeh): Reject a commit request if AddCommitted cannot encode
  // commit_seq. This happens if prep_seq <<< commit_seq.
  wpt_db_->AddCommitted(prepare_seq_, commit_seq);
  return s;
}

Status WritePreparedTxn::Rollback() {
  // TODO(myabandeh) Implement this
  throw std::runtime_error("Rollback not Implemented");
  return Status::OK();
}

}  // namespace rocksdb

#endif  // ROCKSDB_LITE
