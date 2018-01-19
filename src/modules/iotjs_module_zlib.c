#include "iotjs_def.h"
#include "iotjs_objectwrap.h"
#include "iotjs_module_buffer.h"
#include "zlib.h"
#include <sys/types.h>

// Custom constants used by both node_constants.cc and node_zlib.cc
#define Z_MIN_WINDOWBITS 8
#define Z_MAX_WINDOWBITS 15
#define Z_DEFAULT_WINDOWBITS 15
// Fewer than 64 bytes per chunk is not recommended.
// Technically it could work with as few as 8, but even 64 bytes
// is low.  Usually a MB or more is best.
#define Z_MIN_CHUNK 64
#define Z_MAX_CHUNK 65535
#define Z_DEFAULT_CHUNK (16 * 1024)
#define Z_MIN_MEMLEVEL 1
#define Z_MAX_MEMLEVEL 9
#define Z_DEFAULT_MEMLEVEL 8
#define Z_MIN_LEVEL -1
#define Z_MAX_LEVEL 9
#define Z_DEFAULT_LEVEL Z_DEFAULT_COMPRESSION

enum node_zlib_mode {
  NONE,
  DEFLATE,
  INFLATE,
  GZIP,
  GUNZIP,
  DEFLATERAW,
  INFLATERAW,
  UNZIP
};

typedef struct {
  iotjs_jobjectwrap_t jobjectwrap;
  Bytef* dictionary_;
  size_t dictionary_len_;
  int err_;
  int flush_;
  bool init_done_;
  int level_;
  int memLevel_;
  enum node_zlib_mode mode_;
  int strategy_;
  z_stream strm_;
  int windowBits_;
  bool write_in_progress_;
  bool pending_close_;
  unsigned int refs_;
  unsigned int gzip_id_bytes_read_;
  uint32_t* write_result_;
} IOTJS_VALIDATED_STRUCT(iotjs_zlib_t);

static JNativeInfoType this_module_native_info = { .free_cb = NULL };

static iotjs_zlib_t* iotjs_zlib_create(const jerry_value_t jval, jerry_value_t mode) {
  iotjs_zlib_t* zlib = IOTJS_ALLOC(iotjs_zlib_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_zlib_t, zlib);
  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, jval,
                               &this_module_native_info);

  _this->dictionary_ = NULL;
  _this->dictionary_len_ = 0;
  _this->err_ = 0;
  _this->flush_ = 0;
  _this->init_done_ = false;
  _this->level_ = 0;
  _this->memLevel_ = 0;
  _this->mode_ = jerry_get_number_value(mode);
  _this->strategy_ = 0;
  _this->windowBits_ = 0;
  _this->write_in_progress_ = false;
  _this->pending_close_ = false;
  _this->refs_ = 0;
  _this->gzip_id_bytes_read_ = 0;
  _this->write_result_ = NULL;
  return zlib;
}

JS_FUNCTION(ZlibConstructor) {
  DJS_CHECK_THIS();
  const jerry_value_t val = JS_GET_THIS();
  iotjs_zlib_create(val, jargv[0]);
  return jerry_create_undefined();
}

JS_FUNCTION(ZlibInit) {
  JS_DECLARE_THIS_PTR(zlib, zlib);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_zlib_t, zlib);

  if (jargc == 5) {
    fprintf(stderr,
      "WARNING: You are likely using a version of node-tar or npm that "
      "is incompatible with this version of Node.js.\nPlease use "
      "either the version of npm that is bundled with Node.js, or "
      "a version of npm (> 5.5.1 or < 5.4.0) or node-tar (> 4.0.1) "
      "that is compatible with Node.js 9 and above.\n");
  }

  int windowBits = jerry_get_number_value(jargv[0]);
  if (windowBits < Z_MIN_WINDOWBITS || windowBits > Z_MAX_WINDOWBITS) {
    return JS_CREATE_ERROR(COMMON, "invalid windowBits");
  }

  int level = jerry_get_number_value(jargv[1]);
  if (level < Z_MIN_LEVEL || level > Z_MAX_LEVEL) {
    return JS_CREATE_ERROR(COMMON, "invalid compression level");
  }

  int memLevel = jerry_get_number_value(jargv[2]);
  if (memLevel < Z_MIN_MEMLEVEL || memLevel > Z_MAX_MEMLEVEL) {
    return JS_CREATE_ERROR(COMMON, "invalid memlevel");
  }

  int strategy = jerry_get_number_value(jargv[3]);
  if (strategy != Z_FILTERED &&
    strategy != Z_HUFFMAN_ONLY &&
    strategy != Z_RLE &&
    strategy != Z_FIXED &&
    strategy != Z_DEFAULT_STRATEGY) {
    return JS_CREATE_ERROR(COMMON, "invalid strategy");
  }

  _this->level_ = level;
  _this->windowBits_ = windowBits;
  _this->memLevel_ = memLevel;
  _this->strategy_ = strategy;
  _this->strm_.zalloc = Z_NULL;
  _this->strm_.zfree = Z_NULL;
  _this->strm_.opaque = Z_NULL;
  _this->flush_ = Z_NO_FLUSH;
  _this->err_ = Z_OK;

  switch (_this->mode_) {
    case DEFLATE:
    case GZIP:
    case DEFLATERAW:
      _this->err_ = deflateInit2(&_this->strm_,
                                 _this->level_,
                                 Z_DEFLATED,
                                 _this->windowBits_,
                                 _this->memLevel_,
                                 _this->strategy_);
      break;
    case INFLATE:
    case GUNZIP:
    case INFLATERAW:
    case UNZIP:
      _this->err_ = inflateInit2(&_this->strm_, _this->windowBits_);
      break;
    default:
      _this->mode_ = NONE;
      return jerry_create_boolean(false);
  }

  _this->dictionary_ = NULL;
  _this->dictionary_len_ = 0;
  _this->write_in_progress_ = false;
  _this->init_done_ = true;

  if (_this->err_ != Z_OK) {
    _this->mode_ = NONE;
    return jerry_create_boolean(false);
  }
  return jerry_create_boolean(true);
}

JS_FUNCTION(ZlibWrite) {
  // JS_DECLARE_THIS_PTR(zlib, zlib);
  // IOTJS_VALIDATED_STRUCT_METHOD(iotjs_zlib_t, zlib);

  return jerry_create_null();
}

JS_FUNCTION(ZlibReset) {
  JS_DECLARE_THIS_PTR(zlib, zlib);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_zlib_t, zlib);
  
  _this->err_ = Z_OK;
  switch (_this->mode_) {
    case DEFLATE:
    case DEFLATERAW:
    case GZIP:
      _this->err_ = deflateReset(&_this->strm_);
      break;
    case INFLATE:
    case INFLATERAW:
    case GUNZIP:
      _this->err_ = inflateReset(&_this->strm_);
      break;
    default:
      break;
  }
  if (_this->err_ != Z_OK) {
    return JS_CREATE_ERROR(COMMON, "Failed to reset stream");
  } else {
    return jerry_create_boolean(true);
  }
}

jerry_value_t InitZlib() {
  jerry_value_t exports = jerry_create_object();
#define IOTJS_DEFINE_ZLIB_CONSTANTS(name) do {            \
  iotjs_jval_set_property_number(exports, #name, name);   \
} while (0)

  // states
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_NO_FLUSH);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_PARTIAL_FLUSH);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_SYNC_FLUSH);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_FULL_FLUSH);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_FINISH);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_BLOCK);

  // return/error codes
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_OK);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_STREAM_END);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_NEED_DICT);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_ERRNO);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_STREAM_ERROR);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_DATA_ERROR);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_MEM_ERROR);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_BUF_ERROR);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_VERSION_ERROR);

  // flags
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_NO_COMPRESSION);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_BEST_SPEED);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_BEST_COMPRESSION);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_DEFAULT_COMPRESSION);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_FILTERED);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_HUFFMAN_ONLY);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_RLE);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_FIXED);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_DEFAULT_STRATEGY);
  IOTJS_DEFINE_ZLIB_CONSTANTS(ZLIB_VERNUM);

  // modes
  IOTJS_DEFINE_ZLIB_CONSTANTS(DEFLATE);
  IOTJS_DEFINE_ZLIB_CONSTANTS(INFLATE);
  IOTJS_DEFINE_ZLIB_CONSTANTS(GZIP);
  IOTJS_DEFINE_ZLIB_CONSTANTS(GUNZIP);
  IOTJS_DEFINE_ZLIB_CONSTANTS(DEFLATERAW);
  IOTJS_DEFINE_ZLIB_CONSTANTS(INFLATERAW);
  IOTJS_DEFINE_ZLIB_CONSTANTS(UNZIP);

  // params
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_MIN_WINDOWBITS);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_MAX_WINDOWBITS);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_DEFAULT_WINDOWBITS);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_MIN_CHUNK);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_MAX_CHUNK);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_DEFAULT_CHUNK);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_MIN_MEMLEVEL);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_MAX_MEMLEVEL);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_DEFAULT_MEMLEVEL);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_MIN_LEVEL);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_MAX_LEVEL);
  IOTJS_DEFINE_ZLIB_CONSTANTS(Z_DEFAULT_LEVEL);
#undef IOTJS_DEFINE_ZLIB_CONSTANTS

  jerry_value_t zlibConstructor =
      jerry_create_external_function(ZlibConstructor);
  iotjs_jval_set_property_jval(exports, "Zlib", zlibConstructor);

  jerry_value_t proto = jerry_create_object();
  iotjs_jval_set_method(proto, "init", ZlibInit);
  iotjs_jval_set_method(proto, "write", ZlibWrite);
  iotjs_jval_set_method(proto, "reset", ZlibReset);
  iotjs_jval_set_property_jval(zlibConstructor, "prototype", proto);

  jerry_release_value(proto);
  jerry_release_value(zlibConstructor);
  return exports;
}
