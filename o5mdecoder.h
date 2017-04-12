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
  size_t xunsigned (uint64_t *d, size_t len, char *data) {
    size_t i;
    unsigned char b;
    int64_t npow = 1;
    for (i = 0; i < len; i++) {
      b = data[i];
      *d += (b & 0x7f) * npow;
      if (b < 0x80) break;
      npow *= 0x80;
    }
    return i+1;
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
    char *_table;
    size_t _tablesize;
    Doc () {
      id = 0;
      version = 0;
      timestamp = 0;
      changeset = 0;
      uid = 0;
      user = NULL;
      _table = NULL;
      _tablesize = 0;
    }
    bool getTag (char **key, char **value) {
      size_t begin, end;
      uint64_t ntable;
      if (_tagpos >= _taglen) return false;
      if (*(_tags+_tagpos) == 0) {
        begin = ++_tagpos;
        *key = _tags+_tagpos;
        for (_tagpos++;*(_tags+_tagpos) != 0 && _tagpos < _taglen; _tagpos++);
        _tagpos++;
        *value = _tags+_tagpos;
        for (;*(_tags+_tagpos) != 0 && _tagpos < _taglen; _tagpos++);
        end = _tagpos;
        _tagpos++;
        memcpy(_table+256*_tablesize++, _tags+begin, end-begin);
        return true;
      } else {
        ntable = 0;
        _tagpos += xunsigned(&ntable, _taglen-_tagpos, _tags+_tagpos);
        printf("ntable=%u\n", ntable);
        printf("table ref not implemented (%d)\n", _tablesize);
      }
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
    size_t _reflen, _refpos;
    uint64_t _ref;
    char *_refbuf;
    bool getRef (uint64_t *ref) {
      if (_refpos >= _reflen) return false;
      _refpos += signedDelta(&_ref, _reflen-_refpos, _refbuf+_refpos);
      *ref = _ref;
      return true;
    }
  };
  class Rel : public Doc {
    public:
    size_t _memlen, _mempos;
    uint64_t _ref;
    char *_membuf;
    bool getMember (TYPE *memtype, uint64_t *ref, char **memrole) {
      if (_mempos >= _memlen) return false;
      _mempos += signedDelta(&_ref, _memlen-_mempos, _membuf+_mempos);
      *ref = _ref;
      if (*(_membuf+_mempos) == 0) {
        _mempos++;
        *memtype = ((unsigned char) *(_membuf+_mempos)) - 0x20;
        _mempos++;
        *memrole = _membuf+_mempos;
        for (; _mempos < _memlen && *(_membuf+_mempos) != 0; _mempos++);
        _mempos++;
      } else {
        printf("table ref not implemented (%d)\n", _tablesize);
      }
      return true;
    }
  };
  class Decoder {
    public:
    char *buffer, *docbuf, *table;
    size_t length, pos, tablesize;
    size_t doclen, docpow, docsize;
    char _err[256];
    Node _prevNode;
    Way _prevWay;
    Rel _prevRel;
    Doc *_prevDoc;
    TYPE type;

    _STATE _state;
    Decoder (char *dbuf, char *stable) {
      _state = _BEGIN;
      table = stable;
      tablesize = 0;
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
      size_t pos = 0, begin = 0;
      if (_prevDoc) {
        doc->id = _prevDoc->id;
        doc->version = _prevDoc->version;
        doc->timestamp = _prevDoc->timestamp;
        doc->changeset = _prevDoc->changeset;
        doc->uid = _prevDoc->uid;
      } else {
        doc->id = 0;
        doc->version = 0;
        doc->timestamp = 0;
        doc->changeset = 0;
        doc->uid = 0;
      }
      pos += signedDelta(&(doc->id), len-pos, buf+pos);
      if (buf[pos] == 0x00) { // no version
        doc->version = 0;
        doc->timestamp = 0;
        doc->changeset = 0;
        doc->uid = 0;
        doc->user = NULL;
        pos++;
      } else {
        pos += xunsigned(&(doc->version), len-pos, buf+pos);
        pos += signedDelta(&(doc->timestamp), len-pos, buf+pos);
        if (doc->timestamp == 0) {
          doc->changeset = 0;
          doc->uid = 0;
          doc->user = NULL;
        } else {
          pos += signedDelta(&(doc->changeset), len-pos, buf+pos);
          if (*(buf+pos) != 0) {
            sprintf(_err, "expected 0x00 after changeset, got 0x%x", *(buf+pos));
            throw _err;
          }
          begin = ++pos;
          pos += xunsigned(&(doc->uid), len-pos, buf+pos);
          if (*(buf+pos) != 0) {
            sprintf(_err, "expected 0x00 after uid, got 0x%x", *(buf+pos));
            throw _err;
          }
          memcpy(table+tablesize*256,buf+begin,pos-begin);
          tablesize++;
          for (; pos < len && *(buf+pos) != 0; pos++);
          pos++;
          doc->user = buf+pos;
          for (; pos < len && *(buf+pos) != 0; pos++);
          pos++;
        }
      }
      return pos;
    }
    size_t _parseTags (Doc *doc, size_t len, char *buf) {
      doc->_tags = buf;
      doc->_taglen = len;
      doc->_tagpos = 0;
      return len;
    }
    void _parseNode (Node *node, size_t len, char *buf) {
      node->_table = table;
      node->_tablesize = tablesize;
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
      way->_table = table;
      way->_tablesize = tablesize;
      size_t pos = _parseDoc(way, len, buf);
      way->_reflen = 0;
      way->_refpos = 0;
      way->_ref = 0;
      pos += xunsigned(&(way->_reflen), len-pos, buf+pos);
      way->_refbuf = buf+pos;
      pos += way->_reflen;
      pos += _parseTags(way, len-pos, buf+pos);
      _prevWay = *way;
      _prevDoc = way;
    }
    void _parseRel (Rel *rel, size_t len, char *buf) {
      rel->_table = table;
      rel->_tablesize = tablesize;
      size_t pos = _parseDoc(rel, len, buf);
      rel->_memlen = 0;
      rel->_mempos = 0;
      rel->_ref = 0;
      pos += xunsigned(&(rel->_memlen), len-pos, buf+pos);
      rel->_membuf = buf+pos;
      pos += rel->_memlen;
      pos += _parseTags(rel, len-pos, buf+pos);
      _prevRel = *rel;
      _prevDoc = rel;
    }
  };
}

#endif
