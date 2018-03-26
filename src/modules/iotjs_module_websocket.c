
#include <stdlib.h>
#include <time.h>
#include "iotjs_def.h"
#include "iotjs_module_buffer.h"

#define WS_FINAL_FRAME    0x1
#define WS_NEXT_FRAME     0x2
#define WS_MASKED_FRAME   0x4

enum ws_frame_type {
  WS_ERROR_FRAME = 0x00,
  WS_INCOMPLETE_FRAME = 0x08,
  WS_UNFINISHED_FRAME = 0x04,
  WS_TEXT_FRAME = 0x01,
  WS_BINARY_FRAME = 0x02,
  WS_OPENING_FRAME = 0x05,
  WS_CLOSING_FRAME = 0x06,
  WS_PING_FRAME = 0x09,
  WS_PONG_FRAME = 0x0A
};

#define WS_PAYLOAD_MAX_SIZE 128 * 1024
enum ws_frame_parse_error {
  WS_PARSE_ERROR_INVALID = -1,
  WS_PARSE_ERROR_LENGTH = -2,
  WS_PARSE_ERROR_FRAME = -3,
  WS_PARSE_ERROR_PAYLOAD = -4,
  WS_PARSE_ERROR_PAYLOAD_TOO_LARGE = -5,
};

/*
   0               1               2               3              
   0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
  +-+-+-+-+-------+-+-------------+-------------------------------+
  |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
  |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
  |N|V|V|V|       |S|             |   (if payload len==126/127)   |
  | |1|2|3|       |K|             |                               |
  +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
  |     Extended payload length continued, if payload len == 127  |
  + - - - - - - - - - - - - - - - +-------------------------------+
  |                               |Masking-key, if MASK set to 1  |
  +-------------------------------+-------------------------------+
  | Masking-key (continued)       |          Payload Data         |
  +-------------------------------- - - - - - - - - - - - - - - - +
  :                     Payload Data continued ...                :
  + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
  |                     Payload Data continued ...                |
  +---------------------------------------------------------------+
*/
typedef struct ws_frame {
  uint8_t fin;
  uint8_t rsv1;
  uint8_t rsv2;
  uint8_t rsv3;
  uint8_t opcode;
  uint8_t mask;
  uint64_t payload_len;
  int32_t masking_key;
  uint8_t *payload;
  enum ws_frame_type type;
} ws_frame;

void w64to8(uint8_t *dstbuffer, uint64_t value, size_t length) {
  if (dstbuffer == NULL) {
    return;
  }

  for (dstbuffer += length - 1; length > 0; length--, dstbuffer--) {
    *dstbuffer = (uint8_t)value;
    value >>= 8;
  }
}

void iotjs_ws_parse_frame_type(struct ws_frame *frame) {
  uint8_t fin = frame->fin;
  uint8_t opcode = frame->opcode;
  if (fin == 0) {
    frame->type = WS_UNFINISHED_FRAME; 
  } else if (opcode == 0x01) {
    frame->type = WS_TEXT_FRAME;
  } else if (opcode == 0x00) {
    frame->type = WS_INCOMPLETE_FRAME;
  } else if (opcode == 0x02) {
    frame->type = WS_BINARY_FRAME;
  } else if (opcode == 0x08) {
    frame->type = WS_CLOSING_FRAME;
  } else if (opcode == 0x09) {
    frame->type = WS_PING_FRAME;
  } else if (opcode == 0x0A) {
    frame->type = WS_PONG_FRAME;
  } else {
    frame->type = WS_ERROR_FRAME;
  }
}

uint64_t f_uint64(uint8_t *value) {
  uint64_t length = 0;
  for (int i = 0; i < 8; i++) {
    length = (length << 8) | value[i];
  }
  return length;
}

uint16_t f_uint16(uint8_t *value) {
  uint16_t length = 0;
  for (int i = 0; i < 2; i++) {
    length = (length << 8) | value[i];
  }
  return length;
}

void _payload_length(uint8_t *packet, struct ws_frame *frame) {
  int length = packet[1] & 0x7f;

  if (length < 126) {
    frame->payload_len = (uint64_t)length;
  } else if (length == 126) {
    frame->payload_len = f_uint16(&packet[2]);
  } else {
    frame->payload_len = f_uint64(&packet[2]);
  }
}

void iotjs_ws_parse_masking_key(uint8_t* packet, int offset, struct ws_frame *frame) {
  int j = 0;

  uint8_t* masking_key = (uint8_t*)&frame->masking_key;
  int end = offset + 4;
  for (int i = offset; i < end; i++) {
    masking_key[j] = packet[i];
    j++;
  }
}

void iotjs_ws_decode_payload(uint8_t* packet, struct ws_frame *frame) {
  uint8_t *masking_key = (uint8_t*)&frame->masking_key;
  for (uint64_t i = 0; i < frame->payload_len; i++) {
    packet[i] ^= masking_key[i % 4];
  }
  frame->payload = packet;
}

void iotjs_ws_parse_payload(uint8_t* packet, struct ws_frame *frame) {
  _payload_length(packet, frame);

  if (frame->mask == 1) {
    int masking_key_offset;
    if (frame->payload_len < 126) {
      masking_key_offset = 2;
    } else if (frame->payload_len < 65536) {
      masking_key_offset = 4;
    } else {
      masking_key_offset = 10;
    }
    iotjs_ws_parse_masking_key(packet, masking_key_offset, frame);
    iotjs_ws_decode_payload(&packet[masking_key_offset + 4], frame);
  } else {
    if (frame->payload_len < 126) {
      frame->payload = &packet[2];
    } else if (frame->payload_len < 65536) {
      frame->payload = &packet[4];
    } else {
      frame->payload = &packet[10];
    }
  }
}

int iotjs_ws_parse_input(uint8_t* input_frame,
                          size_t input_len,
                          struct ws_frame *frame) {
  if (input_frame == NULL)
    return WS_PARSE_ERROR_INVALID;
  if (input_len < 2)
    return WS_PARSE_ERROR_LENGTH;
  memset(frame, 0, sizeof(struct ws_frame));
  frame->fin = input_frame[0] & 0x80;
  frame->rsv1 = input_frame[0] & 0x40;
  frame->rsv2 = input_frame[0] & 0x20;
  frame->rsv3 = input_frame[0] & 0x10;
  frame->opcode = input_frame[0] & 0x0f;
  frame->mask = input_frame[1] & 0x80;
  iotjs_ws_parse_frame_type(frame);
  if (frame->opcode != WS_ERROR_FRAME) {
    iotjs_ws_parse_payload(input_frame, frame);
    return frame->payload != NULL ? 0 : WS_PARSE_ERROR_PAYLOAD;
  } else {
    return WS_PARSE_ERROR_FRAME;
  }
}

uint8_t* iotjs_ws_make_header(size_t data_len, 
                              enum ws_frame_type frame_type, 
                              size_t* header_len, int options) {
  uint8_t *header;
  uint8_t end_byte;
  uint8_t masked = 0x80;

  if (data_len < 1) {
    return NULL;
  }

  if ((options & WS_FINAL_FRAME) == 0x0) {
    end_byte = 0x0;
  } else if (options & WS_FINAL_FRAME) {
    end_byte = 0x80;
  } else {
    return NULL;
  }
  *header_len = 0;

  if (data_len < 126) {
    header = malloc(sizeof(uint8_t) * 2);
    header[0] = end_byte | frame_type;
    header[1] = (uint8_t) (masked | data_len);
    *header_len = 2;
  } else if (data_len > 126 && data_len < 65536) {
    header = malloc(sizeof(uint8_t) * 4);
    header[0] = end_byte | frame_type;
    header[1] = (uint8_t) (masked | 0x7e);
    header[2] = (uint8_t) (data_len >> 8);
    header[3] = (uint8_t) data_len & 0xff;
    *header_len = 4;
  } else if (data_len >= 65536) {
    header = malloc(sizeof(uint8_t) * 10);
    header[0] = end_byte | frame_type;
    header[1] = (uint8_t) (masked | 0x7f);
    w64to8(&header[2], data_len, 8);
    *header_len = 10;
  } else {
    return NULL;
  }

  int offset = *header_len;
  header[offset + 0] = rand() / (RAND_MAX / 0xff);
  header[offset + 1] = rand() / (RAND_MAX / 0xff);
  header[offset + 2] = rand() / (RAND_MAX / 0xff);
  header[offset + 3] = rand() / (RAND_MAX / 0xff);

  *header_len += 4;
  return header;
}


JS_FUNCTION(EncodeFrame) {
  int type = JS_GET_ARG(0, number);
  iotjs_bufferwrap_t* data = iotjs_bufferwrap_from_jbuffer(jargv[1]);

  uint8_t* header;
  size_t header_len;
  size_t data_len = iotjs_bufferwrap_length(data);
  header = iotjs_ws_make_header(data_len, (enum ws_frame_type)type, &header_len, WS_FINAL_FRAME);

  size_t out_len = data_len + header_len;
  uint8_t out_frame[out_len + 1];

  memset(out_frame, 0, out_len + 1);
  memcpy(out_frame, header, header_len);

  uint8_t* mask = header + header_len - 4;
  char* masked = iotjs_bufferwrap_buffer(data);

  for (size_t i = 0; i < data_len; ++i) {
    out_frame[header_len + i] = (uint8_t) (masked[i] ^ mask[i % 4]);
  }
  jerry_value_t jframe = iotjs_bufferwrap_create_buffer(data_len + header_len);
  iotjs_bufferwrap_t* frame_wrap = iotjs_bufferwrap_from_jbuffer(jframe);
  iotjs_bufferwrap_copy(frame_wrap, (const char*)out_frame, out_len);
  
  free(header);
  return jframe;
}

JS_FUNCTION(DecodeFrame) {
  iotjs_bufferwrap_t* data = iotjs_bufferwrap_from_jbuffer(jargv[0]);
  uint8_t* chunk = (uint8_t*)iotjs_bufferwrap_buffer(data);
  size_t chunk_size = iotjs_bufferwrap_length(data);

  struct ws_frame frame;
  int r = iotjs_ws_parse_input(chunk, chunk_size, &frame);
  if (r < 0) {
    return JS_CREATE_ERROR(COMMON, "websocket frame parse error");
  }

  jerry_value_t jbuffer = iotjs_bufferwrap_create_buffer(frame.payload_len);
  iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuffer);
  iotjs_bufferwrap_copy(buffer_wrap, (const char*)frame.payload, frame.payload_len);

  jerry_value_t ret = jerry_create_object();
  iotjs_jval_set_property_jval(ret, "type", jerry_create_number((int)frame.type));
  iotjs_jval_set_property_jval(ret, "fin", jerry_create_number(frame.fin));
  iotjs_jval_set_property_jval(ret, "opcode", jerry_create_number(frame.opcode));
  iotjs_jval_set_property_jval(ret, "mask", jerry_create_number(frame.mask));
  iotjs_jval_set_property_jval(ret, "masking_key", jerry_create_number(frame.masking_key));
  iotjs_jval_set_property_jval(ret, "buffer", jbuffer);
  jerry_release_value(jbuffer);

  return ret;
}

jerry_value_t InitWebSocket() {
  srand(time(NULL));

  jerry_value_t exports = jerry_create_object();
  iotjs_jval_set_method(exports, "encodeFrame", EncodeFrame);
  iotjs_jval_set_method(exports, "decodeFrame", DecodeFrame);
  return exports;
}
