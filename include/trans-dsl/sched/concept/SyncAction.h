//
// Created by Darwin Yuan on 2020/6/9.
//

#ifndef TRANS_DSL_2_SYNCACTION_H
#define TRANS_DSL_2_SYNCACTION_H

#include <trans-dsl/tsl_ns.h>
#include "SchedAction.h"

TSL_NS_BEGIN

struct SyncActionAdapter : SchedAction  {
   OVERRIDE(handleEvent(TransactionContext&, Event&) -> Status) {
      return Result::FATAL_BUG;
   }
   OVERRIDE(kill(TransactionContext&, Status)        -> void) {}
   OVERRIDE(stop(TransactionContext&, Status) -> Status) {
      return Result::SUCCESS;
   }
};

TSL_NS_END

#endif //TRANS_DSL_2_SYNCACTION_H
