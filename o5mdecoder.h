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
    char *_tags;
    void getTag (char **key, char **value) {
      
    }
  };
  class Node : public Doc {
    public:
    double lat;
    double lon;
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
    }
    void write (char *data, size_t len) {
      buffer = data;
      length = len;
    }
    bool read (Doc *doc) {
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
          doc->type = b;
          printf("type=0x%x\n", doc->type);
        } else if (_state == _LEN) {
          doclen += (b & 0x7f) * docpow;
          printf("doclen=%u, docpow=%u, b=%u\n", doclen, docpow, b);
          docpow *= 0x80;
          if (b < 0x80) {
            _state = _DATA;
          }
        } else if (_state == _DATA) {
          j = fminl(length, pos + doclen - docsize);
          memcpy(docbuf+docsize, buffer+pos, j-pos);
          docsize += j - pos;
          pos = j - 1;
          printf("cmp %u, %u; j=%u\n", docsize, doclen, j);
          if (docsize == doclen) {
            printf("doc %d bytes\n", docsize);
            // parse docbuf...
            _state = _TYPE;
            doclen = 0;
            docpow = 1;
            docsize = 0;
            //return true;
          }
        } else if (_state == _END) {
          // ...
        }
      }
      return false;
    }
  };
}

#endif
