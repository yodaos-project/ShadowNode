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

static void
utf8_string_print (FILE *fp, lit_utf8_byte_t *buffer_p, lit_utf8_size_t sz)
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

typedef void (*string_print_type) (FILE *fp, ecma_string_t *name_str_p);

static void
string_print (FILE *fp, ecma_string_t *name_str_p)
{
  fprintf (fp, "{\"id\":%u,\"chars\":\"", ecma_make_string_value (name_str_p));

  lit_utf8_size_t sz = ecma_string_get_utf8_size (name_str_p);
  lit_utf8_byte_t buffer_p[sz];
  ecma_string_to_utf8_bytes (name_str_p, buffer_p, sz);
  utf8_string_print (fp, buffer_p, sz);

  fprintf (fp, "\"}\n");
}

static void
string_node_print (FILE *fp, ecma_string_t *str_p)
{
  ecma_value_t value = ecma_make_string_value(str_p);
  lit_utf8_size_t sz = ecma_string_get_utf8_size (str_p);
  fprintf (fp, "{\"node\":");
  fprintf (fp, "[%u,%u,%u,%u]",
           NODE_TYPE_STRING,
           value,
           value,
           (uint32_t) sizeof (ecma_string_t) + sz);
  fprintf (fp, "\n,\"references\":[]\n}\n");
}

static void
heapdump_magic_strings (FILE *fp,
                        string_print_type callback,
                        bool skip_first_comma)
{
  for (lit_magic_string_id_t id = 0;
       id < LIT_NON_INTERNAL_MAGIC_STRING__COUNT;
       id++)
  {
    ecma_string_t *string_p = ecma_get_magic_string (id);
    if (!skip_first_comma || id != 0)
    {
      fprintf (fp, ",");
    }
    callback (fp, string_p);
  }
}

static void
heapdump_strings (FILE *fp, string_print_type callback)
{
  ecma_lit_storage_item_t *string_list_p = JERRY_CONTEXT (string_list_first_p);

  while (string_list_p != NULL)
  {
    for (int i = 0; i < ECMA_LIT_STORAGE_VALUE_COUNT; i++)
    {
      if (string_list_p->values[i] != JMEM_CP_NULL)
      {
        ecma_string_t *value_p;
        value_p = JMEM_CP_GET_NON_NULL_POINTER (ecma_string_t,
                                                string_list_p->values[i]);
        fprintf (fp, ",");
        callback (fp, value_p);
      }
    }

    string_list_p = JMEM_CP_GET_POINTER (ecma_lit_storage_item_t,
                                         string_list_p->next_cp);
  }
}

static void
heapdump_numbers (FILE *fp)
{
  ecma_lit_storage_item_t *number_list_p = JERRY_CONTEXT (number_list_first_p);

  while (number_list_p != NULL)
  {
    for (int i = 0; i < ECMA_LIT_STORAGE_VALUE_COUNT; i++)
    {
      if (number_list_p->values[i] != JMEM_CP_NULL)
      {
        ecma_string_t *value_p = JMEM_CP_GET_NON_NULL_POINTER (ecma_string_t,
            number_list_p->values[i]);
        fprintf (fp, ",");
        fprintf (fp, "{\"node\":");
        fprintf (fp, "[%u,%u,%u,%u]",
                 NODE_TYPE_NUMBER,
                 ecma_make_magic_string_value(LIT_MAGIC_STRING_NUMBER),
                 value_p->u.lit_number,
                 (uint32_t) sizeof (double));
        fprintf (fp, "\n,\"references\":[]\n}\n");
      }
    }

    number_list_p = JMEM_CP_GET_POINTER (ecma_lit_storage_item_t,
                                         number_list_p->next_cp);
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
    ecma_value_t name_value = ecma_make_string_value (edge_name_p);
    ecma_value_t to_node = ecma_make_object_value (obj_p);
    fprintf (fp, "[%u,%u,%u]", edge_type, name_value, to_node);
  }

  if (first_visit && *first_visit)
  {
    *first_visit = false;
  }
}

static void
heapdump_object (FILE *fp, ecma_object_t *object_p)
{
  ecma_value_t node_id = ecma_make_object_value (object_p);
  ecma_string_t *node_name = ecma_object_get_name (object_p);
  uint32_t node_self_size = ecma_object_get_size (object_p);
  int node_type;

  if (ecma_is_lexical_environment (object_p))
  {
    node_type = NODE_TYPE_HIDDEN;
  }
  else
  {
    ecma_object_type_t type = ecma_get_object_type (object_p);
    switch (type)
    {
    case ECMA_OBJECT_TYPE_GENERAL:
      node_type = NODE_TYPE_OBJECT;
      break;
    case ECMA_OBJECT_TYPE_CLASS:
      node_type = NODE_TYPE_OBJECT; // ?
      break;
    case ECMA_OBJECT_TYPE_FUNCTION:
      node_type = NODE_TYPE_CLOSURE;
      break;
    case ECMA_OBJECT_TYPE_EXTERNAL_FUNCTION:
      node_type = NODE_TYPE_CLOSURE;
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
      break;
#endif /* !CONFIG_DISABLE_ES2015_ARROW_FUNCTION */
    default:
      JERRY_UNREACHABLE ();
    }
  }

  fprintf (fp, "\"node\":[%d,%u,%u,%u]\n",
           node_type,
           ecma_make_string_value (node_name),
           node_id,
           node_self_size);
}

void
heap_profiler_take_snapshot (FILE *fp)
{
  fprintf(fp, "{");
  fprintf(fp, "\"strings\":[\n");
  heapdump_magic_strings (fp, string_print, true);
  heapdump_strings (fp, string_print);
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
    ecma_vist_object_references (obj_iter_p,
                                 heapdump_object_value,
                                 (void*) fp);
    fprintf (fp, "]\n}\n");
    obj_iter_p = ECMA_GET_POINTER (ecma_object_t, obj_iter_p->gc_next_cp);
  }
  heapdump_magic_strings (fp, string_node_print, false);
  heapdump_strings (fp, string_node_print);
  heapdump_numbers (fp);
  fprintf(fp, "]\n"); // nodes end

  fprintf(fp, "}");
}

#endif /* JERRY_HEAP_PROFILER */
