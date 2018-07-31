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

#include "heap-profiler.h"
#include "jcontext.h"
#include "ecma-globals.h"
#include "ecma-helpers.h"
#include "ecma-objects.h"
#include "ecma-function-object.h"

#ifdef JERRY_HEAP_PROFILER

static void utf8_string_print (FILE *fp, lit_utf8_byte_t *buffer_p, lit_utf8_size_t sz)
{
  for (uint32_t i = 0; i < sz; i++)
  {
    lit_utf8_byte_t c = buffer_p[i];
    switch (c)
    {
    case '\b':
      fprintf (fp, "\\b");
      break;
    case '\f':
      fprintf (fp, "\\f");
      break;
    case '\n':
      fprintf (fp, "\\n");
      break;
    case '\r':
      fprintf (fp, "\\r");
      break;
    case '\t':
      fprintf (fp, "\\t");
      break;
    case '\"':
      fprintf (fp, "\\\"");
      break;
    case '\\':
      fprintf (fp, "\\\\");
      break;
    default:
      if (c < 128 && c > 31)
      {
        fputc (c, fp);
      }
      else
      {
        fprintf (fp, "u%x,", c);
      }
      break;
    }
  }
}

static void string_print (FILE *fp, ecma_string_t *name_str_p)
{
  fprintf (fp, "{\"id\":%u,\"chars\":\"", ecma_make_string_value (name_str_p));

  lit_utf8_size_t sz = ecma_string_get_utf8_size (name_str_p);
  lit_utf8_byte_t buffer_p[sz];
  ecma_string_to_utf8_bytes (name_str_p, buffer_p, sz);
  utf8_string_print (fp, buffer_p, sz);

  fprintf (fp, "\"}\n");
}

static ecma_string_t*
get_object_name (ecma_object_t *object_p)
{
//  ecma_object_type_t type = ecma_get_object_type (object_p);
//  if (type == ECMA_OBJECT_TYPE_FUNCTION)
  {
    ecma_extended_object_t *func_obj_p = (ecma_extended_object_t *) object_p;
    if (ecma_get_object_is_builtin (object_p))
    {
      lit_magic_string_id_t id;
      ecma_built_in_props_t builtin = func_obj_p->u.built_in;
      if (ecma_builtin_function_is_routine (object_p))
      {
        id = ecma_builtin_routine_get_name (builtin.id, builtin.routine_id);
      }
      else
      {
        id = ecma_builtin_get_name (builtin.id);
      }
      return ecma_get_magic_string (id);
    }
#ifdef JERRY_FUNCTION_NAME
    else {
      const ecma_compiled_code_t *bytecode_data_p = ecma_op_function_get_compiled_code (func_obj_p);
      if (bytecode_data_p->name != ECMA_VALUE_EMPTY)
      {
        return ecma_get_string_from_value (bytecode_data_p->name);
      }
    }
#endif /* JERRY_FUNCTION_NAME */
  }

  lit_magic_string_id_t id = ecma_object_get_class_name (object_p);
  return ecma_get_magic_string(id);
}

static ecma_string_t*
get_object_constructor_name (ecma_object_t *object_p)
{
  ecma_object_t *proto_p = ecma_get_object_prototype (object_p);
  if (proto_p != NULL)
  {
    ecma_string_t *constructor_string = ecma_get_magic_string(LIT_MAGIC_STRING_CONSTRUCTOR);
    ecma_property_t *property_p = ecma_find_named_property (proto_p, constructor_string);

    if (property_p != NULL &&
        ECMA_PROPERTY_GET_TYPE (*property_p) == ECMA_PROPERTY_TYPE_NAMEDDATA)
    {
      ecma_property_value_t *prop_value = ecma_get_named_data_property (proto_p, constructor_string);
      if (ecma_is_value_object (prop_value->value))
      {
        return get_object_name (ecma_get_object_from_value(prop_value->value));
      }
    }
  }
  return NULL;
}

static void
heapdump_object (FILE *fp, ecma_object_t *object_p)
{
  ecma_value_t node_id = ecma_make_object_value (object_p);
  ecma_string_t *node_name = NULL;
  int node_type;
  uint32_t node_self_size = sizeof (ecma_object_t); //default

  if (ecma_is_lexical_environment (object_p))
  {
    node_type = NODE_TYPE_HIDDEN;
    node_name = ecma_get_magic_string (LIT_MAGIC_STRING_LEX_ENV);
  }
  else
  {
    ecma_object_type_t type = ecma_get_object_type (object_p);
    if (type != ECMA_OBJECT_TYPE_GENERAL && ecma_get_object_extensible (object_p))
    {
      ecma_extended_object_t *ext_object_p = (ecma_extended_object_t*) object_p;
      node_self_size = ext_object_p->size;
    }
    switch (type)
    {
    case ECMA_OBJECT_TYPE_GENERAL:
      node_type = NODE_TYPE_OBJECT;
      node_name = get_object_constructor_name (object_p);
      break;
    case ECMA_OBJECT_TYPE_CLASS:
      node_type = NODE_TYPE_OBJECT; // ?
      break;
    case ECMA_OBJECT_TYPE_FUNCTION:
      node_type = NODE_TYPE_CLOSURE;
      node_name = get_object_name (object_p);
      break;
    case ECMA_OBJECT_TYPE_EXTERNAL_FUNCTION:
      node_type = NODE_TYPE_CLOSURE;
      node_name = ecma_get_magic_string (LIT_MAGIC_STRING_EXTERNAL_FUNCTION);
      break;
    case ECMA_OBJECT_TYPE_ARRAY:
      node_type = NODE_TYPE_ARRAY;
      break;
    case ECMA_OBJECT_TYPE_BOUND_FUNCTION:
      node_type = NODE_TYPE_CLOSURE; // ?
      break;
    case ECMA_OBJECT_TYPE_PSEUDO_ARRAY:
      node_type = NODE_TYPE_ARRAY; // ?
      break;
#ifndef CONFIG_DISABLE_ES2015_ARROW_FUNCTION
    case ECMA_OBJECT_TYPE_ARROW_FUNCTION:
      node_type = NODE_TYPE_CLOSURE;
      node_name = ecma_get_magic_string (LIT_MAGIC_STRING_ARROW_FUNCTION);
      break;
#endif /* !CONFIG_DISABLE_ES2015_ARROW_FUNCTION */
    default:
      JERRY_UNREACHABLE ();
    }
  }

  if (node_name == NULL)
  {
    lit_magic_string_id_t id = ecma_object_get_class_name (object_p);
    node_name = ecma_get_magic_string (id);
  }

  fprintf (fp, "\"node\":[%d,%u,%d,%u]\n",
           node_type,
           ecma_make_string_value (node_name),
           node_id,
           node_self_size);
}

static void heapdump_magic_strings (FILE *fp)
{
  for (lit_magic_string_id_t id = 0; id < LIT_NON_INTERNAL_MAGIC_STRING__COUNT; id++)
  {
    if (id != 0)
    {
      fprintf (fp, ",");
    }
    ecma_string_t *string_p = ecma_get_magic_string (id);
    string_print (fp, string_p);
  }
}

static void heapdump_strings (FILE *fp)
{
  ecma_lit_storage_item_t *string_list_p = JERRY_CONTEXT (string_list_first_p);

  while (string_list_p != NULL)
  {
    for (int i = 0; i < ECMA_LIT_STORAGE_VALUE_COUNT; i++)
    {
      if (string_list_p->values[i] != JMEM_CP_NULL)
      {
        ecma_string_t *value_p = JMEM_CP_GET_NON_NULL_POINTER (ecma_string_t,
                                                               string_list_p->values[i]);
        fprintf (fp, ",");
        string_print (fp, value_p);
      }
    }

    string_list_p = JMEM_CP_GET_POINTER (ecma_lit_storage_item_t, string_list_p->next_cp);
  }
}

static void
heapdump_object_value (ecma_object_t *obj_p,
                       bool *first_visit,
                       ecma_string_t *edge_name_p,
                       v8_edge_type_t edge_type,
                       void *args)
{
  FILE *fp = (FILE*) args;
  if(fp)
  {
    if (first_visit && !*first_visit)
    {
      fprintf (fp, ",");
    }
    fprintf (fp, "[%u,%u,%u]", edge_type, ecma_make_string_value (edge_name_p), ecma_make_object_value (obj_p));
  }

  if (first_visit && *first_visit)
  {
    *first_visit = false;
  }
}

void heap_profiler_take_snapshot (FILE *fp)
{
  fprintf(fp, "{");
  fprintf(fp, "\"strings\":[");
  heapdump_magic_strings (fp);
  heapdump_strings (fp);
  fprintf(fp, "],\n\"nodes\":[\n");

  ecma_object_t *obj_iter_p = JERRY_CONTEXT (ecma_gc_objects_p);
  bool first = true;
  while (obj_iter_p != NULL)
  {
    if (!first)
    {
      fprintf (fp, ",");
    }
    else {
      first = false;
    }
    fprintf (fp, "{");
    heapdump_object (fp, obj_iter_p);
    fprintf (fp, ",\"references\":[");
    ecma_vist_object_references (obj_iter_p, heapdump_object_value, (void*) fp);
    fprintf (fp, "]\n}\n");
    obj_iter_p = ECMA_GET_POINTER (ecma_object_t, obj_iter_p->gc_next_cp);
  }
  fprintf(fp, "]\n");

  fprintf(fp, "}");
}

#endif /* JERRY_HEAP_PROFILER */
