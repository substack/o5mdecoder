# o5mdecoder

c++ parser for [o5m](http://wiki.openstreetmap.org/wiki/O5m) files

To use, include a single `#include <o5mdecoder.h>` in your program.

# example

denormalize lon/lat geometry as arrays of points for each way

``` c++
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <o5mdecoder.h>

struct Point {
  float lon, lat;
  Point (float _lon, float _lat) {
    lon=_lon; lat=_lat;
  }
};

int main (int argc, char **argv) {
  char *data = (char*) malloc(4096);
  char *dbuf = (char*) malloc(4096);
  char *table = (char*) malloc(256*15000);
  o5mdecoder::Decoder d(dbuf,table);
  uint64_t ref;
  std::map<uint64_t,Point*> nodes;
  std::map<uint64_t,Point*>::const_iterator ipt;

  size_t len;
  do {
    len = fread(data, sizeof(char), 4096, stdin);
    d.write(data, len);
    try {
      while (d.read()) {
        if (d.node) {
          nodes[d.node->id] = new Point(d.node->lon,d.node->lat);
        } else if (d.way) {
          while (d.way->getRef(&ref)) {
            ipt = nodes.find(ref);
            if (ipt == nodes.end()) continue;
            printf("%f,%f ", ipt->second->lon, ipt->second->lat);
          }
          printf("\n");
        }
      }
    } catch (char *err) {
      printf("error: %s\n", err);
    }
  } while (len == 4096);
  return 0;
}
```

# api

``` c++
#include <o5mdecoder.h>
```

## `o5mdecoder::Decoder decoder(char *buffer, char *table)`

Initialie a new `decoder` instance given a `buffer` and `table`.

The `buffer` is used to copy data as you read it in from the file and must be
greater than or equal to the size of the largest document in compressed binary
o5m form. 

The `table` is used to store strings in order to refer back to them. The o5m
format states that the string table should be able to hold 15000 entries where
each entry can be at most 250 characters, but this implementation assumes 256
character, so you should allocate the `table` to be 3840000 bytes (15000*256).

## `decoder.write(char *data, size_t len)`

Write `len` bytes of `data` to be parsed. You must call `.read()` in a loop to
empty out the read documents before calling `.write()` again.

## `bool decoder.read()`

Sets `decoder.node`, `decoder.way`, or `decoder.rel` and returns true or doesn't
set anything and returns false. When `.read()` returns false it is time to
`.write()` more data.

Exactly one or zero of `decoder.node`, `decoder.way`, and `decoder.rel` is set
at a time.

## o5mdecoder::Doc doc

`Node`, `Way`, and `Rel` classes inherit from `Doc`.

## `uint64_t doc.id`

Document id. This property is always `> 0`.

## `uint64_t doc.version`

Version number for each document starting from 1.

When there is no version information the version is set to `0`.

## `uint64_t doc.changeset`

The changeset integer for an update.
Many documents may be contained in the same changeset.

If the changeset is not present, the changeset is set to `0`.

## `int64_t doc.timestamp`

Number of seconds since the epoch, Jan 1 1970.
If not set, the timestamp will be `0`.

## `uint64_t doc.uid`

User id for the author of a document or `0` if not set.

## `char* doc.user`

User name string of the author. NULL if not set.

## `bool doc.getTag(char **key, char **value)`

Read a tag into key and value pointers. Call this method repeatedly to read all
the tags out of a doc until the method returns false.

The return value indicates whether a tag was read.

## `o5mdecoder::Node node`

Inherits from o5mdecoder::Doc.

## `double node.lon`

Longitude in decimal degrees.

## `double node.lat`

Latitude in decimal degrees.

## `o5mdecoder::Way way`

Inherits from o5mdecoder::Doc.

## `bool way.getRef(uint64_t *ref)`

Read a ref from a way. Call this method repeatedly to read all the refs in a
way until the method returns false. Returns true when a ref was read and false
when a ref was not read.

## `o5mdecoder::Rel rel`

Inherits from o5mdecoder::Doc.

## `bool rel.getMember(Member member)`

Read member data from the relation by reference into `member`.

Return true as long as data was read. Returns false when there is no more data.

## `o5mdecoder::Member member`

## `o5mdecoder::TYPE member.type`

Type of the member. Compare with `o5mdecoder::NODE`, `o5mdecoder::WAY`, and
`o5mdecoder::REL`.

## `uint64_t member.ref`

Reference id of the member.

## `char* member.role`

Role of the member (things like "inner" and "outer").

# license

BSD
