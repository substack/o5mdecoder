#include <stdio.h>
#include <stdlib.h>
#include <o5mdecoder.h>

int main (int argc, char **argv) {
  char *data = (char*) malloc(4096);
  char *dbuf = (char*) malloc(4096);
  o5mdecoder::Decoder d(dbuf);
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
          printf("<node id=\"%d\" lon=\"%f\" lat=\"%f\"/>\n",
            node.id, node.lon, node.lat);
        } else if (d.type == o5mdecoder::WAY) {
          printf("<way id=\"%d\"/>\n", way.id);
        } else if (d.type == o5mdecoder::REL) {
          printf("<relation id=\"%d\"/>\n", rel.id);
        }
      }
    } catch (char *err) {
      fprintf(stderr, "error: %s\n", err);
    }
  } while (len == 4096);
  return 0;
}
