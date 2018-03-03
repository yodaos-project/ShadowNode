#include <stdlib.h>
#include <MQTTPacket/MQTTPacket.h>
#include "iotjs_def.h"
#include "iotjs_objectwrap.h"
#include "iotjs_module_buffer.h"

enum QoS { 
  QOS0, 
  QOS1, 
  QOS2, 
  SUBFAIL=0x80 
};

typedef struct {
  iotjs_jobjectwrap_t jobjectwrap;
  char* host;
  int port;
  unsigned char* src;
  int srclen;
  int offset;
  MQTTPacket_connectData options_;
  uv_loop_t* loop;
} IOTJS_VALIDATED_STRUCT(iotjs_mqtt_t);

static JNativeInfoType this_module_native_info = { .free_cb = NULL };

static iotjs_mqtt_t* iotjs_mqtt_create(const jerry_value_t value) {
  iotjs_mqtt_t* mqtt = IOTJS_ALLOC(iotjs_mqtt_t);
  IOTJS_VALIDATED_STRUCT_CONSTRUCTOR(iotjs_mqtt_t, mqtt);
  iotjs_jobjectwrap_initialize(&_this->jobjectwrap, value,
                               &this_module_native_info);
  return mqtt;
}

JS_FUNCTION(MqttConstructor) {
  DJS_CHECK_THIS();

  const jerry_value_t self = JS_GET_THIS();
  iotjs_mqtt_t* mqtt = iotjs_mqtt_create(self);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_mqtt_t, mqtt);

  jerry_value_t opts = JS_GET_ARG(0, object);
  jerry_value_t version = iotjs_jval_get_property(opts, "protocolVersion");
  jerry_value_t keepalive = iotjs_jval_get_property(opts, "keepalive");
  jerry_value_t username = iotjs_jval_get_property(opts, "username");
  jerry_value_t password = iotjs_jval_get_property(opts, "password");
  jerry_value_t clientId = iotjs_jval_get_property(opts, "clientId");

  MQTTPacket_connectData options = MQTTPacket_connectData_initializer;
  options.willFlag = 0;
  options.MQTTVersion = (int)iotjs_jval_as_number(version);
  options.keepAliveInterval = (int)iotjs_jval_as_number(keepalive);
  options.cleansession = 1;

  if (jerry_value_is_string(username)) {
    iotjs_string_t str = iotjs_jval_as_string(username);
    MQTTString mqttstr = MQTTString_initializer;
    mqttstr.lenstring.data = strdup((char*)iotjs_string_data(&str));
    mqttstr.lenstring.len = (int)iotjs_string_size(&str);
    options.username = mqttstr;
    iotjs_string_destroy(&str);
  }
  if (jerry_value_is_string(password)) {
    iotjs_string_t str = iotjs_jval_as_string(password);
    MQTTString mqttstr = MQTTString_initializer;
    mqttstr.lenstring.data = strdup((char*)iotjs_string_data(&str));
    mqttstr.lenstring.len = (int)iotjs_string_size(&str);
    options.password = mqttstr;
    iotjs_string_destroy(&str);
  }
  if (jerry_value_is_string(clientId)) {
    iotjs_string_t str = iotjs_jval_as_string(clientId);
    MQTTString mqttstr = MQTTString_initializer;
    mqttstr.lenstring.data = strdup((char*)iotjs_string_data(&str));
    mqttstr.lenstring.len = (int)iotjs_string_size(&str);
    options.clientID = mqttstr;
    iotjs_string_destroy(&str);
  }
  _this->options_ = options;

  jerry_release_value(version);
  jerry_release_value(keepalive);
  jerry_release_value(username);
  jerry_release_value(password);
  jerry_release_value(clientId);
  return jerry_create_undefined();
}

int iotjs_mqtt_read_buf(unsigned char* buf, int len, void* ctx) {
  iotjs_mqtt_t_impl_t* _this = (iotjs_mqtt_t_impl_t*)ctx;
  memset(buf, 0, (size_t)len);
  memcpy(buf, _this->src + _this->offset, (size_t)len);
  _this->offset += len;
  return len;
}

JS_FUNCTION(MqttReadPacket) {
  JS_DECLARE_THIS_PTR(mqtt, mqtt);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_mqtt_t, mqtt);

  iotjs_bufferwrap_t* wrap = iotjs_bufferwrap_from_jbuffer(jargv[0]);
  _this->src = (unsigned char*)iotjs_bufferwrap_buffer(wrap);
  _this->srclen = iotjs_bufferwrap_length(wrap);
  _this->offset = 0;

  unsigned char buf[1024];
  int r = MQTTPacket_read(buf, sizeof(buf), iotjs_mqtt_read_buf, _this);

  jerry_value_t ret = jerry_create_object();
  iotjs_jval_set_property_jval(ret, "type", jerry_create_number(r));

  if (r == CONNACK) {
    unsigned char present;
    unsigned char code;
    if (MQTTDeserialize_connack(&present, &code, buf, sizeof(buf)) != 1 || code != 0) {
      return JS_CREATE_ERROR(COMMON, "Unable to connect.");
    }
  } else if (r == SUBACK) {
    int count = 0;
    int grantedQoS = 0;
    unsigned short msgId;
    if (MQTTDeserialize_suback(&msgId, 
                               1, 
                               &count, 
                               (int*)&grantedQoS, 
                               buf, sizeof(buf)) != 1 || grantedQoS == 0x80) {
      return JS_CREATE_ERROR(COMMON, "SUBACK failed with server.");
    }
  } else if (r == PUBLISH) {
    jerry_value_t databuf = iotjs_bufferwrap_create_buffer(sizeof(buf));
    iotjs_bufferwrap_t* wrap = iotjs_bufferwrap_from_jbuffer(databuf);
    iotjs_bufferwrap_copy(wrap, (const char*)buf, sizeof(buf));
    iotjs_jval_set_property_jval(ret, "buffer", databuf);
    jerry_release_value(databuf);
  }
  return ret;
}

JS_FUNCTION(MqttGetConnect) {
  JS_DECLARE_THIS_PTR(mqtt, mqtt);
  IOTJS_VALIDATED_STRUCT_METHOD(iotjs_mqtt_t, mqtt);

  unsigned char buf[100];
  int len = MQTTSerialize_connect(buf, sizeof(buf), &_this->options_);
  if (len == MQTTPACKET_BUFFER_TOO_SHORT) {
    return JS_CREATE_ERROR(COMMON, "connection length is too short.");
  }

  jerry_value_t retbuf = iotjs_bufferwrap_create_buffer((size_t)len);
  iotjs_bufferwrap_t* wrap = iotjs_bufferwrap_from_jbuffer(retbuf);
  iotjs_bufferwrap_copy(wrap, (const char*)buf, (size_t)len);
  return retbuf;
}

JS_FUNCTION(MqttGetPublish) {
  iotjs_string_t topic = JS_GET_ARG(0, string);
  jerry_value_t opts = JS_GET_ARG(1, object);
  jerry_value_t msg_id = iotjs_jval_get_property(opts, "id");
  jerry_value_t msg_qos = iotjs_jval_get_property(opts, "qos");
  jerry_value_t msg_dup = iotjs_jval_get_property(opts, "dup");
  jerry_value_t msg_retain = iotjs_jval_get_property(opts, "retain");
  jerry_value_t msg_payload = iotjs_jval_get_property(opts, "payload");
  iotjs_string_t msg_payload_str = iotjs_jval_as_string(msg_payload);

  MQTTString top = MQTTString_initializer;
  top.cstring = (char *)iotjs_string_data(&topic);

  int msg_size = (int)iotjs_string_size(&msg_payload_str);
  int buf_size = 1024;
  if (buf_size <= msg_size) {
    buf_size = msg_size + 50;
  }
  unsigned char buf[buf_size];
  int len = MQTTSerialize_publish(buf, buf_size, 
                                  iotjs_jval_as_boolean(msg_dup) ? 1 : 0, 
                                  iotjs_jval_as_number(msg_qos),
                                  iotjs_jval_as_boolean(msg_retain) ? 1 : 0,
                                  (unsigned short)iotjs_jval_as_number(msg_id),
                                  top,
                                  (unsigned char*)iotjs_string_data(&msg_payload_str),
                                  msg_size);
  printf("len: %d\n", len);

  jerry_value_t retbuf = iotjs_bufferwrap_create_buffer((size_t)len);
  iotjs_bufferwrap_t* wrap = iotjs_bufferwrap_from_jbuffer(retbuf);
  iotjs_bufferwrap_copy(wrap, (const char*)buf, (size_t)len);

  jerry_release_value(msg_id);
  jerry_release_value(msg_dup);
  jerry_release_value(msg_retain);
  jerry_release_value(msg_payload);
  iotjs_string_destroy(&topic);
  iotjs_string_destroy(&msg_payload_str);
  return retbuf;
}

JS_FUNCTION(MqttGetPingReq) {
  unsigned char buf[100];
  int len = MQTTSerialize_pingreq(buf, sizeof(buf));
  jerry_value_t retbuf = iotjs_bufferwrap_create_buffer((size_t)len);
  iotjs_bufferwrap_t* wrap = iotjs_bufferwrap_from_jbuffer(retbuf);
  iotjs_bufferwrap_copy(wrap, (const char*)buf, (size_t)len);
  return retbuf;
}

JS_FUNCTION(MqttGetAck) {
  int msg_id = JS_GET_ARG(0, number);
  int qos = JS_GET_ARG(1, number);
  unsigned char buf[100];
  int len = 0;

  if (qos == QOS1) {
    len = MQTTSerialize_ack(buf, sizeof(buf), PUBACK, 0, msg_id);
  } else if (qos == QOS2) {
    len = MQTTSerialize_ack(buf, sizeof(buf), PUBREC, 0, msg_id);
  } else {
    return JS_CREATE_ERROR(COMMON, "invalid qos from message.");
  }

  if (len <= 0)
    return JS_CREATE_ERROR(COMMON, "invalid serialization for ack.")

  jerry_value_t retbuf = iotjs_bufferwrap_create_buffer((size_t)len);
  iotjs_bufferwrap_t* wrap = iotjs_bufferwrap_from_jbuffer(retbuf);
  iotjs_bufferwrap_copy(wrap, (const char*)buf, (size_t)len);
  return retbuf;
}

JS_FUNCTION(MqttGetSubscribe) {
  jerry_value_t opts = JS_GET_ARG(1, object);
  jerry_value_t msg_id = iotjs_jval_get_property(opts, "id");
  jerry_value_t msg_qos = iotjs_jval_get_property(opts, "qos");

  jerry_value_t jtopics = jargv[0];
  uint32_t size = jerry_get_array_length(jtopics);
  int qos = (int)iotjs_jval_as_number(msg_qos);

  MQTTString topics[size];
  for (uint32_t i = 0; i < size; i++) {
    jerry_value_t jtopic = iotjs_jval_get_property_by_index(jtopics, i);
    iotjs_string_t topicstr = iotjs_jval_as_string(jtopic);
    MQTTString curr = MQTTString_initializer;
    curr.cstring = (char*)strdup(iotjs_string_data(&topicstr));
    topics[i] = curr;

    jerry_release_value(jtopic);
    iotjs_string_destroy(&topicstr);
  }

  unsigned char buf[100];
  int len = MQTTSerialize_subscribe(buf, sizeof(buf),
                                    0,
                                    (unsigned short)iotjs_jval_as_number(msg_id),
                                    (int)size,
                                    topics,
                                    (int*)&qos);

  jerry_value_t retbuf = iotjs_bufferwrap_create_buffer((size_t)len);
  iotjs_bufferwrap_t* wrap = iotjs_bufferwrap_from_jbuffer(retbuf);
  iotjs_bufferwrap_copy(wrap, (const char*)buf, (size_t)len);

  for (uint32_t i = 0; i < size; i++) {
    if (topics[i].cstring != NULL)
      free(topics[i].cstring);
  }
  return retbuf;
}

JS_FUNCTION(MqttGetUnsubscribe) {
  jerry_value_t jtopics = jargv[0];
  int msgId = JS_GET_ARG(1, number);
  uint32_t size = jerry_get_array_length(jtopics);

  MQTTString topics[size];
  for (uint32_t i = 0; i < size; i++) {
    jerry_value_t jtopic = iotjs_jval_get_property_by_index(jtopics, i);
    iotjs_string_t topicstr = iotjs_jval_as_string(jtopic);
    MQTTString curr = MQTTString_initializer;
    curr.cstring = (char*)strdup(iotjs_string_data(&topicstr));
    topics[i] = curr;

    jerry_release_value(jtopic);
    iotjs_string_destroy(&topicstr);
  }

  unsigned char buf[100];
  int len = MQTTSerialize_unsubscribe(buf, sizeof(buf),
                                      0,
                                      (unsigned short)msgId,
                                      (int)size,
                                      topics);

  jerry_value_t retbuf = iotjs_bufferwrap_create_buffer((size_t)len);
  iotjs_bufferwrap_t* wrap = iotjs_bufferwrap_from_jbuffer(retbuf);
  iotjs_bufferwrap_copy(wrap, (const char*)buf, (size_t)len);
  
  for (uint32_t i = 0; i < size; i++) {
    if (topics[i].cstring != NULL)
      free(topics[i].cstring);
  }
  return retbuf;
}

JS_FUNCTION(MqttGetDisconnect) {
  unsigned char buf[100];
  int len = MQTTSerialize_disconnect(buf, sizeof(buf));
  jerry_value_t retbuf = iotjs_bufferwrap_create_buffer((size_t)len);
  iotjs_bufferwrap_t* wrap = iotjs_bufferwrap_from_jbuffer(retbuf);
  iotjs_bufferwrap_copy(wrap, (const char*)buf, (size_t)len);
  return retbuf;
}

JS_FUNCTION(MqttDeserialize) {
  iotjs_bufferwrap_t* wrap = iotjs_bufferwrap_from_jbuffer(jargv[0]);
  unsigned short msgId = 0;
  unsigned char dup = 0;
  unsigned char retain = 0;
  int qos = 0;

  MQTTString topic;
  char* payload;
  int payload_len;

  int r = MQTTDeserialize_publish(&dup, 
                                  (int*)&qos, 
                                  &retain, 
                                  &msgId, 
                                  &topic, 
                                  (unsigned char**)&payload, 
                                  (int*)&payload_len,
                                  (unsigned char*)iotjs_bufferwrap_buffer(wrap),
                                  (int)iotjs_bufferwrap_length(wrap));
  if (r != 1) {
    return JS_CREATE_ERROR(COMMON, "failed to deserialize publish message.");
  }

  jerry_value_t msg = jerry_create_object();
  iotjs_jval_set_property_jval(msg, "id", jerry_create_number(msgId));
  iotjs_jval_set_property_jval(msg, "qos", jerry_create_number(qos));
  iotjs_jval_set_property_jval(msg, "dup", jerry_create_boolean(dup));
  iotjs_jval_set_property_jval(msg, "retain", jerry_create_boolean(retain));

  iotjs_string_t topic_str = 
    iotjs_string_create_with_size(topic.lenstring.data, (size_t)topic.lenstring.len);
  iotjs_jval_set_property_string(msg, "topic", &topic_str);
  iotjs_string_destroy(&topic_str);

  jerry_value_t payload_buffer = iotjs_bufferwrap_create_buffer((size_t)payload_len);
  iotjs_bufferwrap_t* payload_wrap = iotjs_bufferwrap_from_jbuffer(payload_buffer);
  iotjs_bufferwrap_copy(payload_wrap, (const char*)payload, (size_t)payload_len);
  iotjs_jval_set_property_jval(msg, "payload", payload_buffer);
  jerry_release_value(payload_buffer);
  return msg;
}

jerry_value_t InitMQTT() {
  jerry_value_t exports = jerry_create_object();
  jerry_value_t mqttConstructor =
      jerry_create_external_function(MqttConstructor);
  iotjs_jval_set_property_jval(exports, "MqttHandle", mqttConstructor);

  jerry_value_t proto = jerry_create_object();
  iotjs_jval_set_method(proto, "_readPacket", MqttReadPacket);
  iotjs_jval_set_method(proto, "_getConnect", MqttGetConnect);
  iotjs_jval_set_method(proto, "_getPublish", MqttGetPublish);
  iotjs_jval_set_method(proto, "_getPingReq", MqttGetPingReq);
  iotjs_jval_set_method(proto, "_getAck", MqttGetAck);
  iotjs_jval_set_method(proto, "_getSubscribe", MqttGetSubscribe);
  iotjs_jval_set_method(proto, "_getUnsubscribe", MqttGetUnsubscribe);
  iotjs_jval_set_method(proto, "_getDisconnect", MqttGetDisconnect);
  iotjs_jval_set_method(proto, "_deserialize", MqttDeserialize);
  // iotjs_jval_set_method(proto, "disconnect", MqttDisconnect);
  iotjs_jval_set_property_jval(mqttConstructor, "prototype", proto);

  jerry_release_value(proto);
  jerry_release_value(mqttConstructor);
  return exports;
}