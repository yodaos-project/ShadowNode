# N-API

Most N-API api in ShadowNode are aligned with definition of Node.js version 10.8. Yet there are some that may not implemented nor not supported. Documents of supported API can be found in[Node.js N-API](https://nodejs.org/docs/latest-v10.x/api/n-api.html) document. Following is a list of api that can not be used in ShadowNode.

## API not supported in ShadowNode

#### Working with JavaScript Values

##### Object Creation Functions
- napi_create_arraybuffer
- napi_create_external_arraybuffer
- napi_create_external_buffer
- napi_create_symbol
- napi_create_typedarray
- napi_create_dataview

##### Functions to convert from C types to N-API
- napi_create_bigint_int64
- napi_create_bigint_uint64
- napi_create_bigint_words
- napi_create_string_latin1
- napi_create_string_utf16

##### Functions to convert from N-API to C types
- napi_get_arraybuffer_info
- napi_get_typedarray_info
- napi_get_dataview_info
- napi_get_value_bigint_int64
- napi_get_value_bigint_uint64
- napi_get_value_bigint_words
- napi_get_value_string_latin1
- napi_get_value_string_utf16

#### Working with JavaScript Values - Abstract Operations
- napi_is_dataview

#### Working with JavaScript Functions
- napi_get_new_target

#### Custom Asynchronous Operations
- napi_async_init
- napi_async_destroy
- napi_make_callback
- napi_open_callback_scope
- napi_close_callback_scope

#### Memory Management
- napi_adjust_external_memory

#### Promises
- napi_create_promise
- napi_resolve_deferred
- napi_reject_deferred
- napi_is_promise

#### Script execution
- napi_run_script

#### Asynchronous Thread-safe Function Calls
> Experimental

- napi_create_threadsafe_function
- napi_get_threadsafe_function_context
- napi_call_threadsafe_function
- napi_acquire_threadsafe_function
- napi_release_threadsafe_function
- napi_ref_threadsafe_function
- napi_unref_threadsafe_function
