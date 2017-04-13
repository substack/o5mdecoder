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
  o5mdecoder::Member member;
  o5mdecoder::Decoder d(dbuf,table);
  size_t len;
  do {
    len = fread(data, sizeof(char), 4096, stdin);
    d.write(data, len);
    try {
      while (d.read()) {
        if (d.node) {
          printf("<node id=\"%d\" lon=\"%f\" lat=\"%f\"",
            d.node->id, d.node->lon, d.node->lat);
          printExtraAttrs(d.node);
          printf(">\n");
          printTags(d.node);
          printf("</node>\n");
        } else if (d.way) {
          printf("<way id=\"%d\"", d.way->id);
          printExtraAttrs(d.way);
          printf(">\n");
          while (d.way->getRef(&ref)) {
            printf("  <nd ref=\"%u\">\n", ref);
          }
          printTags(d.way);
          printf("</way>\n");
        } else if (d.rel) {
          printf("<relation id=\"%d\"", d.rel->id);
          printExtraAttrs(d.rel);
          printf(">\n");
          while (d.rel->getMember(member)) {
            dtype = doctype(member.type);
            printf("  <member type=\"%s\" ref=\"%u\" role=\"%s\"/>\n",
              dtype, ref, memrole);
          }
          printTags(d.rel);
          printf("</relation>\n");
        }
      }
    } catch (char *err) {
      printf("error: %s\n", err);
    }
  } while (len == 4096);
  return 0;
}
