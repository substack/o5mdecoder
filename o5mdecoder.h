#ifndef O5MDECODER_H
#define O5MDECODER_H

#include <stdint.h>

namespace o5mdecoder {
  typedef unsigned char TYPE;
  const TYPE NODE = 0x10;
  const TYPE WAY = 0x11;
  const TYPE REL = 0x12;

  typedef unsigned char _STATE;
  enum _STATE {
    _BEGIN = 1;
    _START = 2;
    _LEN = 3;
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
    char *buffer;
    size_t length;
    size_t pos;
    Decoder (char *data, size_t len) {
      buffer = data;
      length = len;
    }
    Doc *read () {
    }
  };
}

#endif
