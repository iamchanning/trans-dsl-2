//
// Created by Darwin Yuan on 2020/6/11.
//

#ifndef TRANS_DSL_2_LOOPHELPER_H
#define TRANS_DSL_2_LOOPHELPER_H

#include <trans-dsl/sched/concept/TransactionContext.h>
#include <trans-dsl/sched/action/SchedOptional.h>
#include <type_traits>
#include <cub/base/IsClass.h>
#include <algorithm>
#include <trans-dsl/sched/helper/Pred.h>
#include <trans-dsl/sched/helper/LoopPred.h>
#include <trans-dsl/sched/helper/LoopPredAction.h>
TSL_NS_BEGIN

namespace details {
   struct BreakActionSignature {};
   using BreakPred = LoopPred<BreakActionSignature>;
   using BreakPredAction = LoopPredAction<BreakActionSignature>;
}

#define __break_if(pred, ...) decltype(TSL_NS::details::BreakPred::DeduceType<pred, ##__VA_ARGS__>())

namespace details {

   template<typename T>
   using IsSchedAction = std::enable_if_t<std::is_base_of_v<SchedAction, T>>;

   using LoopSeq = unsigned short;

   template<size_t V_SIZE, size_t V_ALIGN, LoopSeq V_SEQ, typename = void, typename ... T_ACTIONS>
   struct GenericLoop_;

   template<size_t V_SIZE, size_t V_ALIGN, LoopSeq V_SEQ, typename T_HEAD, typename ... T_TAIL>
   struct GenericLoop_<V_SIZE, V_ALIGN, V_SEQ, IsSchedAction<T_HEAD>, T_HEAD, T_TAIL...> {
      using Action = T_HEAD;
      using Next =
      typename GenericLoop_<
         std::max(V_SIZE, sizeof(Action)),
         std::max(V_ALIGN, alignof(Action)),
         V_SEQ + 1,
         void,
         T_TAIL...>::Inner;

      struct Inner : Next {
         using Next::cache;

         auto get(LoopSeq seq, int& v) -> SchedAction* {
            if(seq == V_SEQ) {
               v = 0;
               return new (cache) Action;
            } else {
               return Next::get(seq, v);
            }
         }
      };
   };

   template<typename T>
   using IsBreak = typename std::enable_if_t<std::is_base_of_v<BreakActionSignature, T> && (sizeof(T) > 1)>;

   template<size_t V_SIZE, size_t V_ALIGN, LoopSeq V_SEQ, typename T_HEAD, typename ... T_TAIL>
   struct GenericLoop_<V_SIZE, V_ALIGN, V_SEQ, IsBreak<T_HEAD>, T_HEAD, T_TAIL...> {
      using Action = BreakPredAction::GenericAction<T_HEAD>;
      using Next =
      typename GenericLoop_<
         V_SIZE,
         V_ALIGN,
         V_SEQ + 1,
         void,
         T_TAIL...>::Inner;

      struct Inner : Next {
         using Next::cache;

         auto get(LoopSeq seq, int& v) -> SchedAction* {
            if(seq == V_SEQ) {
               v = 1;
               return &breakAction;
            } else {
               return Next::get(seq, v);
            }
         }

      private:
         Action breakAction;
      };
   };

   template<typename T>
   using IsEmptyBreak =
      typename std::enable_if_t<std::is_base_of_v<BreakActionSignature, T> && \
         sizeof(T) == 1>;

   template<size_t V_SIZE, size_t V_ALIGN, LoopSeq V_SEQ, typename T_HEAD, typename ... T_TAIL>
   struct GenericLoop_<V_SIZE, V_ALIGN, V_SEQ, IsEmptyBreak<T_HEAD>, T_HEAD, T_TAIL...> {
      using Action = BreakPredAction::GenericAction<T_HEAD>;
      using Next =
      typename GenericLoop_<
         std::max(V_SIZE, sizeof(Action)),
         std::max(V_ALIGN, alignof(Action)),
         V_SEQ + 1,
         void,
         T_TAIL...>::Inner;

      struct Inner : Next {
         using Next::cache;

         auto get(LoopSeq seq, int& v) -> SchedAction* {
            if(seq == V_SEQ) {
               v = 2;
               return new (cache) Action;
            } else {
               return Next::get(seq, v);
            }
         }
      };
   };

   template<size_t T_SIZE, size_t T_ALIGN, LoopSeq T_SEQ>
   struct GenericLoop_<T_SIZE, T_ALIGN, T_SEQ> {
      struct Inner  {
         auto get(LoopSeq seq, int& i) -> SchedAction* {
            return nullptr;
         }
      protected:
         alignas(T_ALIGN) char cache[T_SIZE];
      };
   };

   template<typename T_ACTION, typename ... T_ACTIONS>
   struct LOOP__ {
      using Actions = typename GenericLoop_<0, 0, 0, void, T_ACTION, T_ACTIONS...>::Inner;
      struct Inner : Actions {
      };
   };
}

#define __loop(...) TSL_NS::details::LOOP__<__VA_ARGS__>::Inner

TSL_NS_END

#endif //TRANS_DSL_2_LOOPHELPER_H