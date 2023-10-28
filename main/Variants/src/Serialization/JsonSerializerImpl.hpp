// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2018
// MIT License

#pragma once

#include "../MultiType.hpp"
#include "JsonSerializer.hpp"


template <typename Writer>
inline void Variants::Internals::JsonSerializer<Writer>::serialize(
    const MultiType& variant, Writer& writer) {
  switch (variant._type) {
    case JSON_FLOAT:
      writer.writeFloat(variant._content.asFloat);
      return;

    case JSON_STRING:
      writer.writeString(variant._content.asString);
      writer.writeString("\n\r");
      return;

    case JSON_UNPARSED:
      writer.writeRaw(variant._content.asString);
      return;

    case JSON_INTEGER:
    case JSON_POSITIVE_INTEGER:
      writer.writeInteger(variant._content.asInteger);
      return;

    case JSON_BOOLEAN:
      writer.writeBoolean(variant._content.asBool != 0);
      writer.writeString("\n\r");
      return;

    default:  // JSON_UNDEFINED
      return;
  }
}
