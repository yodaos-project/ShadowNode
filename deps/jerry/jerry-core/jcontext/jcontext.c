/* Copyright JS Foundation and other contributors, http://js.foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "jcontext.h"

/** \addtogroup context Context
 * @{
 */

#ifndef JERRY_ENABLE_EXTERNAL_CONTEXT
/**
 * Global context.
 */
jerry_context_t jerry_global_context;

/**
 * Jerry global heap section attribute.
 */
#ifndef JERRY_HEAP_SECTION_ATTR
#define JERRY_GLOBAL_HEAP_SECTION
#else /* JERRY_HEAP_SECTION_ATTR */
#define JERRY_GLOBAL_HEAP_SECTION __attribute__ ((section (JERRY_HEAP_SECTION_ATTR)))
#endif /* !JERRY_HEAP_SECTION_ATTR */

#ifndef JERRY_SYSTEM_ALLOCATOR
/**
 * Global heap.
 */
jmem_heap_t jerry_global_heap __attribute__ ((aligned (JMEM_ALIGNMENT))) JERRY_GLOBAL_HEAP_SECTION;
#endif /* !JERRY_SYSTEM_ALLOCATOR */

#ifndef CONFIG_ECMA_LCACHE_DISABLE

/**
 * Global hash table.
 */
jerry_hash_table_t jerry_global_hash_table;

#endif /* !CONFIG_ECMA_LCACHE_DISABLE */
#endif /* !JERRY_ENABLE_EXTERNAL_CONTEXT */

void
jcontext_get_backtrace_depth (uint32_t *frames, uint32_t depth) {
  vm_frame_ctx_t *ctx_p = JERRY_CONTEXT (vm_top_context_p);
  for (uint32_t idx = 0; idx < depth && ctx_p != NULL; ++idx) {
    jmem_cpointer_t byte_code_cp;
    JMEM_CP_SET_NON_NULL_POINTER (byte_code_cp, ctx_p->bytecode_header_p);
    frames[idx] = (uint32_t) byte_code_cp;

    ctx_p = ctx_p->prev_context_p;
  }
}

/**
 * @}
 */
