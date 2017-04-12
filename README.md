# o5mdecoder

c++ parser for [o5m](http://wiki.openstreetmap.org/wiki/O5m) files

To use, include a single `#include <o5mdecoder.h>` in your program.

# example

o5m to not-quite-osmxml converter:

``` c++
#include <stdio.h>
#include <stdlib.h>
#include <o5mdecoder.h>

void printTags (o5mdecoder::Doc *doc) {
  char *key, *value;
  while (doc->getTag(&key, &value)) {
    printf("  <tag k=\"%s\" v=\"%s\"/>\n",key,value);
  }
}

void printExtraAttrs (o5mdecoder::Doc *doc) {
  if (doc->version) printf(" version=\"%u\"", doc->version);
  if (doc->timestamp) printf(" timestamp=\"%u\"", doc->timestamp);
  if (doc->changeset) printf(" changeset=\"%u\"", doc->changeset);
  if (doc->uid) printf(" uid=\"%u\"", doc->uid);
  if (doc->user) printf(" user=\"%s\"", doc->user);
}

const char *doctype (o5mdecoder::TYPE type) {
  switch (type) {
    case o5mdecoder::NODE: return "node\0";
    case o5mdecoder::WAY: return "way\0";
    case o5mdecoder::REL: return "relation\0";
  }
  return "?\0";
}

int main (int argc, char **argv) {
  char *data = (char*) malloc(4096);
  char *dbuf = (char*) malloc(4096);
  char *table = (char*) malloc(256*15000);
  char *key, *value, *memrole;
  const char *dtype;
  uint64_t ref;
  o5mdecoder::TYPE memtype;
  o5mdecoder::Decoder d(dbuf,table);
  o5mdecoder::Node node;
  o5mdecoder::Way way;
  o5mdecoder::Rel rel;
  size_t len;
  do {
    len = fread(data, sizeof(char), 4096, stdin);
    d.write(data, len);
    try {
      while (d.read(&node, &way, &rel)) {
        if (d.type == o5mdecoder::NODE) {
          printf("<node id=\"%d\" lon=\"%f\" lat=\"%f\"",
            node.id, node.lon, node.lat);
          printExtraAttrs(&node);
          printf(">\n");
          printTags(&node);
          printf("</node>\n");
        } else if (d.type == o5mdecoder::WAY) {
          printf("<way id=\"%d\"", way.id);
          printExtraAttrs(&way);
          printf(">\n");
          while (way.getRef(&ref)) {
            printf("  <nd ref=\"%u\">\n", ref);
          }
          printTags(&way);
          printf("</way>\n");
        } else if (d.type == o5mdecoder::REL) {
          printf("<relation id=\"%d\"", rel.id);
          printExtraAttrs(&rel);
          printf(">\n");
          while (rel.getMember(&memtype,&ref,&memrole)) {
            dtype = doctype(memtype);
            printf("  <member type=\"%s\" ref=\"%u\" role=\"%s\"/>\n",
              dtype, ref, memrole);
          }
          printTags(&rel);
          printf("</relation>\n");
        }
      }
    } catch (char *err) {
      printf("error: %s\n", err);
    }
  } while (len == 4096);
  return 0;
}
```

