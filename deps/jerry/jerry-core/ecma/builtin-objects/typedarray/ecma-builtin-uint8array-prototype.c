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

#ifndef CONFIG_DISABLE_ES2015_TYPEDARRAY_BUILTIN

#define ECMA_BUILTINS_INTERNAL
#include "ecma-builtins-internal.h"

#define BUILTIN_INC_HEADER_NAME "ecma-builtin-uint8array-prototype.inc.h"
#define BUILTIN_UNDERSCORED_ID uint8array_prototype
#include "ecma-builtin-internal-routines-template.inc.h"

/** \addtogroup ecma ECMA
 * @{
 *
 * \addtogroup ecmabuiltins
 * @{
 *
 * \addtogroup uint8arrayprototype ECMA Uint8Array.prototype object built-in
 * @{
 */

/**
 * The Uint8Array.prototype object's 'slice' routine
 *
 * See also:
 *          ECMA-262 v6, 22.2.3.23
 *
 * @return ecma value
 *         Returned value must be freed with ecma_free_value.
 */
static ecma_value_t
ecma_builtin_uint8array_prototype_object_slice (ecma_value_t this_arg, /**< 'this' argument */
                                                ecma_value_t arg1, /**< start */
                                                ecma_value_t arg2) /**< end */
{
  ecma_value_t ret_value = ECMA_VALUE_EMPTY;

  /* 1. */
  ECMA_TRY_CATCH (obj_this,
                  ecma_op_to_object (this_arg),
                  ret_value);

  ecma_object_t *obj_p = ecma_get_object_from_value (obj_this);

  ECMA_TRY_CATCH (len_value,
                  ecma_op_object_get_by_magic_id (obj_p, LIT_MAGIC_STRING_LENGTH),
                  ret_value);

  /* 3. */
  ECMA_OP_TO_NUMBER_TRY_CATCH (len_number, len_value, ret_value);

  /* 4. */
  const uint32_t len = ecma_number_to_uint32 (len_number);

  uint32_t start = 0, end = len;

  /* 5. */
  ECMA_OP_TO_NUMBER_TRY_CATCH (start_num, arg1, ret_value);

  start = ecma_builtin_helper_array_index_normalize (start_num, len);

  /* 7. */
  if (ecma_is_value_undefined (arg2))
  {
    end = len;
  }
  else
  {
    /* 7. part 2 */
    ECMA_OP_TO_NUMBER_TRY_CATCH (end_num, arg2, ret_value);

    end = ecma_builtin_helper_array_index_normalize (end_num, len);

    ECMA_OP_TO_NUMBER_FINALIZE (end_num);
  }

  ECMA_OP_TO_NUMBER_FINALIZE (start_num);

  JERRY_ASSERT (start <= len && end <= len);

#ifndef CONFIG_DISABLE_ES2015_CLASS
  ecma_value_t new_array = ecma_op_create_array_object_by_constructor (NULL, 0, false, obj_p);

  if (ECMA_IS_VALUE_ERROR (new_array))
  {
    ecma_free_value (len_value);
    ecma_free_value (obj_this);
    return new_array;
  }
#else /* CONFIG_DISABLE_ES2015_CLASS */
  ecma_value_t new_array = ecma_op_create_array_object (NULL, 0, false);
  JERRY_ASSERT (!ECMA_IS_VALUE_ERROR (new_array));
#endif /* !CONFIG_DISABLE_ES2015_CLASS */

  ecma_object_t *new_array_p = ecma_get_object_from_value (new_array);

  /* 9. */
  uint32_t n = 0;

  /* 10. */
  for (uint32_t k = start; k < end && ecma_is_value_empty (ret_value); k++, n++)
  {
    /* 10.a */
    ecma_string_t *curr_idx_str_p = ecma_new_ecma_string_from_uint32 (k);

    /* 10.c */
    ECMA_TRY_CATCH (get_value, ecma_op_object_find (obj_p, curr_idx_str_p), ret_value);

    if (ecma_is_value_found (get_value))
    {
      /* 10.c.i */
      ecma_string_t *to_idx_str_p = ecma_new_ecma_string_from_uint32 (n);

      /* 10.c.ii */
      /* This will always be a simple value since 'is_throw' is false, so no need to free. */
      ecma_value_t put_comp = ecma_builtin_helper_def_prop (new_array_p,
                                                            to_idx_str_p,
                                                            get_value,
                                                            true, /* Writable */
                                                            true, /* Enumerable */
                                                            true, /* Configurable */
                                                            false);
      JERRY_ASSERT (ecma_is_value_true (put_comp));

      ecma_deref_ecma_string (to_idx_str_p);
    }

    ECMA_FINALIZE (get_value);

    ecma_deref_ecma_string (curr_idx_str_p);
  }

  if (ecma_is_value_empty (ret_value))
  {
    ret_value = new_array;
  }
  else
  {
    ecma_free_value (new_array);
  }

  ECMA_OP_TO_NUMBER_FINALIZE (len_number);
  ECMA_FINALIZE (len_value);
  ECMA_FINALIZE (obj_this);

  return ret_value;
} /* ecma_builtin_uint8array_prototype_object_slice */                                                               

#endif /* !CONFIG_DISABLE_ES2015_TYPEDARRAY_BUILTIN */
