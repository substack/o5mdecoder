#include <stdio.h>
#include <stdlib.h>
#include <o5mdecoder.h>

int main (int argc, char **argv) {
  char *data = (char*) malloc(4096);
  char *dbuf = (char*) malloc(4096);
  char *table = (char*) malloc(256*15000);
  char *key, *value, *memtype, *memrole;
  uint64_t ref;
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
          printf("<node id=\"%d\" lon=\"%f\" lat=\"%f\">\n",
            node.id, node.lon, node.lat);
          while (node.getTag(&key, &value)) {
            printf("  <tag k=\"%s\" v=\"%s\"/>\n",key,value);
          }
          printf("</node>\n");
        } else if (d.type == o5mdecoder::WAY) {
          printf("<way id=\"%d\">\n", way.id);
          while (way.getRef(&ref)) {
            printf("  <nd ref=\"%u\"/>\n", ref);
          }
          while (way.getTag(&key, &value)) {
            printf("  <tag k=\"%s\" v=\"%s\"/>\n",key,value);
          }
          printf("</way>\n");
        } else if (d.type == o5mdecoder::REL) {
          printf("<relation id=\"%d\">\n", rel.id);
          while (rel.getMember(&memtype,&ref,&memrole)) {
            printf("  <member type=\"%s\" ref=\"%u\" role=\"%s\"/>\n",
              memtype, ref, memrole);
          }
          while (rel.getTag(&key, &value)) {
            printf("  <tag k=\"%s\" v=\"%s\"/>\n",key,value);
          }
          printf("</relation>\n");
        }
      }
    } catch (char *err) {
      fprintf(stderr, "error: %s\n", err);
    }
  } while (len == 4096);
  return 0;
}
