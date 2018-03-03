
#include <stdlib.h>
#include <time.h>
#include "iotjs_def.h"
#include "iotjs_module_buffer.h"

#define WS_FINAL_FRAME    0x1
#define WS_NEXT_FRAME     0x2
#define WS_MASKED_FRAME   0x4

enum ws_frame_type {
  WS_ERROR_FRAME = 0,
  WS_INCOMPLETE_FRAME = 0x08,
  WS_TEXT_FRAME = 0x01,
  WS_BINARY_FRAME = 0x02,
  WS_OPENING_FRAME = 0x05,
  WS_CLOSING_FRAME = 0x06,
  WS_PING_FRAME = 0x09,
  WS_PONG_FRAME = 0x0A
};

void w64to8(uint8_t *dstbuffer, uint64_t value, size_t length) {
  if (dstbuffer == NULL) {
    return;
  }

  for (dstbuffer += length - 1; length > 0; length--, dstbuffer--) {
    *dstbuffer = (uint8_t)value;
    value >>= 8;
  }
}

enum ws_frame_type type(uint8_t* packet) {
  int opcode = packet[0] & 0xf;
  if (opcode == 0x01) {
    return WS_TEXT_FRAME;
  } else if (opcode == 0x00) {
    return WS_INCOMPLETE_FRAME;
  } else if (opcode == 0x02) {
    return WS_BINARY_FRAME;
  } else if (opcode == 0x08) {
    return WS_CLOSING_FRAME;
  } else if (opcode == 0x09) {
    return WS_PING_FRAME;
  } else if (opcode == 0x0A) {
    return WS_PONG_FRAME;
  } else {
    return WS_ERROR_FRAME;
  }
}

enum ws_frame_type iotjs_ws_parse_input(uint8_t* input_frame,
                                        size_t input_len) {
  enum ws_frame_type frame_type;
  if (input_frame == NULL)
    return WS_ERROR_FRAME;

  if (input_len < 2)
    return WS_INCOMPLETE_FRAME;

  frame_type = type(input_frame);
  return frame_type;
}

int _masked(uint8_t *packet) {
    return (packet[1] >> 7) & 0x1;
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

uint64_t _payload_length(uint8_t *packet) {
  int length = -1;
  uint8_t temp = 0;

  if (_masked(packet)) {
    temp = packet[1];
    length = (temp &= ~(1 << 7));
  } else {
    length = packet[1];
  }

  if (length < 125) {
    return (uint64_t)length;
  } else if (length == 126) {
    return f_uint16(&packet[2]);
  } else if (length == 127) {
    return f_uint64(&packet[2]);
  } else {
    return (uint64_t)length;
  }
}

uint8_t* _extract_mask_len1(uint8_t* packet) {
  uint8_t* mask;
  int j = 0;

  mask = malloc(sizeof(uint8_t) * 4);
  for (int i = 2; i < 6; i++) {
    mask[j] = packet[i];
    j++;
  }
  return mask;
}

uint8_t* _extract_mask_len2(uint8_t* packet) {
  uint8_t *mask;
  int j = 0;

  mask = malloc(sizeof(uint8_t) * 4);
  for (int i = 4; i < 8; i++) {
    mask[j] = packet[i];
    j++;
  }
  return mask;
}

uint8_t* _extract_mask_len3(uint8_t *packet) {
  uint8_t *mask;
  int j = 0;

  mask = malloc(sizeof(uint8_t) * 4);
  for (int i = 10; i < 14; i++) {
    mask[j] = packet[i];
    j++;
  }
  return mask;
}

uint8_t* unmask(uint8_t* packet, uint64_t length, uint8_t *mask) {
  for (uint64_t i = 0; i < length; i++) {
    packet[i] ^= mask[i % 4];
  }
  free(mask);
  return packet;
}

uint8_t* iotjs_ws_decode_payload(uint8_t* packet, uint64_t* length) {
  uint8_t* mask;
  int m = _masked(packet);
  *length = _payload_length(packet);

  if (m == 1) {
    if (*length < 126) {
      mask = _extract_mask_len1(packet);
      return unmask(&packet[6], *length, mask);
    } else if (*length > 126 && *length < 65536) {
      mask = _extract_mask_len2(packet);
      return unmask(&packet[8], *length, mask);
    } else if (*length >= 65536) {
      mask = _extract_mask_len3(packet);
      return unmask(&packet[14], *length, mask);
    }
  } else {
    if (*length < 126) {
      return &packet[2];
    } else if (*length > 126 && *length < 65536) {
      return &packet[4];
    } else if (*length >= 65536) {
      return &packet[10];
    }
  }
  return NULL;
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

  printf("header: %hhu %hhu\n", header[0], header[1]);

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
  uint64_t out_frame[out_len + 1];

  memset(out_frame, 0, data_len + 1);
  memcpy(out_frame, header, header_len);

  uint8_t* mask = header + header_len - 4;
  char* masked = iotjs_bufferwrap_buffer(data);

  for (size_t i = 0; i < data_len; ++i) {
    out_frame[header_len + i] = (uint8_t) (masked[i] ^ mask[i % 4]);
  }

  jerry_value_t jframe = iotjs_bufferwrap_create_buffer(data_len + header_len);
  iotjs_bufferwrap_t* frame_wrap = iotjs_bufferwrap_from_jbuffer(jframe);
  iotjs_bufferwrap_copy(frame_wrap, (const char*)out_frame, out_len);
  
  printf("send ");
  for (size_t i = 0; i < out_len + 1; i++) {
    printf("%llu ", out_frame[i]);
  }
  printf("\n");

  free(header);
  return jframe;
}

JS_FUNCTION(DecodeFrame) {
  iotjs_bufferwrap_t* data = iotjs_bufferwrap_from_jbuffer(jargv[0]);
  uint8_t* chunk = (uint8_t*)iotjs_bufferwrap_buffer(data);
  size_t chunk_size = iotjs_bufferwrap_length(data);

  enum ws_frame_type frame_type = iotjs_ws_parse_input(chunk, chunk_size);
  uint64_t packet_len;
  uint8_t* packet = iotjs_ws_decode_payload(chunk, &packet_len);

  jerry_value_t jbuffer = iotjs_bufferwrap_create_buffer(packet_len);
  iotjs_bufferwrap_t* buffer_wrap = iotjs_bufferwrap_from_jbuffer(jbuffer);
  iotjs_bufferwrap_copy(buffer_wrap, (const char*)packet, packet_len);

  jerry_value_t frame = jerry_create_object();
  iotjs_jval_set_property_jval(frame, "type", jerry_create_number((int)frame_type));
  iotjs_jval_set_property_jval(frame, "buffer", jbuffer);
  jerry_release_value(jbuffer);
  return frame;
}

jerry_value_t InitWebSocket() {
  srand(time(NULL));

  jerry_value_t exports = jerry_create_object();
  iotjs_jval_set_method(exports, "encodeFrame", EncodeFrame);
  iotjs_jval_set_method(exports, "decodeFrame", DecodeFrame);
  return exports;
}
