//
//  Created by Bradley Austin Davis on 2015/07/08
//  Copyright 2013-2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once
#ifndef hifi_BufferParser_h
#define hifi_BufferParser_h

#include <cstdint>

#include <QUuid>
#include <QtEndian>

#include "GLMHelpers.h"
#include "ByteCountCoding.h"
#include "PropertyFlags.h"

class BufferParser {
public:
    BufferParser(const uint8_t* data, size_t size, size_t offset = 0) :
        _offset(offset), _data(data), _size(size) {
    }

    template<typename T>
    void readValue(T& result) {
        Q_ASSERT(remaining() >= sizeof(T));
        memcpy(&result, _data + _offset, sizeof(T));
        _offset += sizeof(T);
    }

    template<>
    void readValue<quat>(quat& result) {
        size_t advance = unpackOrientationQuatFromBytes(_data + _offset, result);
        _offset += advance;
    }

    template<>
    void readValue<QString>(QString& result) {
        uint16_t length; readValue(length);
        result = QString((const char*)_data + _offset);
    }

    template<>
    void readValue<QUuid>(QUuid& result) {
        uint16_t length; readValue(length);
        Q_ASSERT(16 == length);
        readUuid(result);
    }

    template<>
    void readValue<xColor>(xColor& result) {
        readValue(result.red);
        readValue(result.blue);
        readValue(result.green);
    }

    template<>
    void readValue<QVector<glm::vec3>>(QVector<glm::vec3>& result) {
        uint16_t length; readValue(length);
        result.resize(length);
        memcpy(result.data(), _data + _offset, sizeof(glm::vec3) * length);
        _offset += sizeof(glm::vec3) * length;
    }

    template<>
    void readValue<QByteArray>(QByteArray& result) {
        uint16_t length; readValue(length);
        result = QByteArray((char*)_data + _offset, (int)length);
        _offset += length;
    }

    void readUuid(QUuid& result) {
        readValue(result.data1);
        readValue(result.data2);
        readValue(result.data3);
        readValue(result.data4);
        result.data1 = qFromBigEndian<quint32>(result.data1);
        result.data2 = qFromBigEndian<quint16>(result.data2);
        result.data3 = qFromBigEndian<quint16>(result.data3);
    }

    template <typename T>
    void readFlags(PropertyFlags<T>& result) {
        // FIXME doing heap allocation
        QByteArray encoded((const char*)(_data + _offset), remaining());
        result.decode(encoded);
        _offset += result.getEncodedLength();
    }

    template<typename T>
    void readCompressedCount(T& result) {
        // FIXME switch to a heapless implementation as soon as Brad provides it.
        QByteArray encoded((const char*)(_data + _offset), std::min(sizeof(T) << 1, remaining()));
        ByteCountCoded<T> codec = encoded;
        result = codec.data;
        encoded = codec;
        _offset += encoded.size();
    }

    inline size_t remaining() const {
        return _size - _offset;
    }

    inline size_t offset() const {
    	return _offset;
    }

    inline const uint8_t* data() const {
        return _data;
    }

private:

    size_t _offset{ 0 };
    const uint8_t* const _data;
    const size_t _size;
};

#endif
