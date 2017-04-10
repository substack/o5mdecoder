#ifndef O5MDECODER_H
#define O5MDECODER_H

#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

namespace o5mdecoder {
  typedef unsigned char TYPE;
  const TYPE NODE = 0x10;
  const TYPE WAY = 0x11;
  const TYPE REL = 0x12;

  enum _STATE {
    _BEGIN = 1,
    _TYPE = 2,
    _LEN = 3,
    _DATA = 4,
    _END = 5
  };
  size_t signedDelta (int64_t *d, size_t len, char *data) {
    size_t i;
    unsigned char b, m = 0x40;
    int64_t npow = data[0]%2 ? -1 : 1;
    for (i = 0; i < len; i++) {
      b = data[i] >> (i == 0 ? 1 : 0);
      *d += (b % m) * npow;
      if (b < 0x80) break;
      npow *= m;
      m = 0x80;
    }
    return i+1;
  }
  size_t signedDelta (uint64_t *d, size_t len, char *data) {
    return signedDelta((int64_t*)d, len, data);
  }
  size_t xunsigned () {
  }

  class Doc {
    public:
    TYPE type;
    uint64_t id;
    uint64_t version;
    int64_t timestamp;
    uint64_t changeset;
    uint64_t uid;
    char *user;
    size_t _taglen;
    size_t _tagpos;
    char *_tags;
    Doc () {
      id = 0;
      version = 0;
      timestamp = 0;
      changeset = 0;
      uid = 0;
      user = NULL;
    }
    bool getTag (char **key, char **value) {
      size_t i = 0;
      return false;
    }
  };
  class Node : public Doc {
    public:
    double lat;
    double lon;
    int64_t ulat;
    int64_t ulon;
    Node () {
      lon = 0;
      lat = 0;
      ulon = 0;
      ulat = 0;
    }
  };
  class Way : public Doc {
    public:
  };
  class Rel : public Doc {
    public:
  };
  class Decoder {
    public:
    char *buffer, *docbuf;
    size_t length, pos;
    size_t doclen, docpow, docsize;
    char _err[256];
    Node _prevNode;
    Way _prevWay;
    Rel _prevRel;
    Doc *_prevDoc;
    TYPE type;

    _STATE _state;
    Decoder (char *dbuf) {
      _state = _BEGIN;
      buffer = NULL;
      pos = 0;
      docbuf = dbuf;
      length = 0;
      docpow = 1;
      doclen = 0;
      docsize = 0;
      _prevDoc = NULL;
    }
    void write (char *data, size_t len) {
      buffer = data;
      length = len;
    }
    bool read (Node *node, Way *way, Rel *rel) {
      size_t j;
      for (; pos < length; pos++) {
        unsigned char b = buffer[pos];
        if (_state == _BEGIN && b != 0xff) {
          sprintf(_err, "first byte in frame, expected 0xff, got 0x%x", b);
          throw _err;
        } else if (_state == _BEGIN) {
          _state = _TYPE;
        } else if (_state == _TYPE && b == 0xff) { // reset
          _state = _TYPE;
        } else if (_state == _TYPE) {
          _state = _LEN;
          type = b;
        } else if (_state == _LEN) {
          doclen += (b & 0x7f) * docpow;
          docpow *= 0x80;
          if (b < 0x80) {
            _state = _DATA;
          }
        } else if (_state == _DATA) {
          j = fminl(length, pos + doclen - docsize);
          memcpy(docbuf+docsize, buffer+pos, j-pos);
          docsize += j - pos;
          pos = j;
          if (docsize == doclen) {
            if (type == NODE) _parseNode(node, doclen, docbuf);
            else if (type == WAY) _parseWay(way, doclen, docbuf);
            else if (type == REL) _parseRel(rel, doclen, docbuf);
            _state = _TYPE;
            doclen = 0;
            docpow = 1;
            docsize = 0;
            return true;
          }
        } else if (_state == _END) {
          // ...
        }
      }
      return false;
    }
    size_t _parseDoc (Doc *doc, size_t len, char *buf) {
      size_t pos = 0;
      if (_prevDoc) doc->id = _prevDoc->id;
      pos += signedDelta(&(doc->id), len-pos, buf+pos); // id
      if (buf[pos] == 0x00) { // version
        pos++;
      } else {
        // ...
      }
      return pos;
    }
    size_t _parseTags (Doc *doc, size_t len, char *buf) {
      doc->_tags = buf;
      doc->_taglen = len;
      doc->_tagpos = 0;
    }
    void _parseNode (Node *node, size_t len, char *buf) {
      size_t pos = _parseDoc(node, len, buf);
      int64_t ulat = _prevNode.ulat;
      int64_t ulon = _prevNode.ulon;
      pos += signedDelta(&ulon, len-pos, buf+pos); // lon
      pos += signedDelta(&ulat, len-pos, buf+pos); // lat
      node->ulon = ulon;
      node->ulat = ulat;
      node->lon = ((double) ulon) / 1e7;
      node->lat = ((double) ulat) / 1e7;
      pos += _parseTags(node, len-pos, buf+pos);
      _prevNode = *node;
      _prevDoc = node;
    }
    void _parseWay (Way *way, size_t len, char *buf) {
      size_t pos = _parseDoc(way, len, buf);
      _prevWay = *way;
      _prevDoc = way;
    }
    void _parseRel (Rel *rel, size_t len, char *buf) {
      size_t pos = _parseDoc(rel, len, buf);
      _prevRel = *rel;
      _prevDoc = rel;
    }
  };
}

#endif
