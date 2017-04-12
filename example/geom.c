#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <o5mdecoder.h>

int main (int argc, char **argv) {
  char *data = (char*) malloc(4096);
  char *dbuf = (char*) malloc(4096);
  char *table = (char*) malloc(256*15000);
  o5mdecoder::Decoder d(dbuf,table);
  o5mdecoder::Node node;
  o5mdecoder::Way way;
  o5mdecoder::Rel rel;
  size_t nodelen = 1000000;
  float *nodebuf = (float*) malloc(nodelen);
  size_t nodepos = 0;
  uint64_t ref;
  o5mdecoder::TYPE memtype;
  char *key, *value, *memrole;
  std::map<uint64_t,float*> nodes;

  size_t len;
  do {
    len = fread(data, sizeof(char), 4096, stdin);
    d.write(data, len);
    try {
      while (d.read(&node, &way, &rel)) {
        if (d.type == o5mdecoder::NODE) {
          printf("node: %f,%f\n", node.lon, node.lat);
          nodebuf[nodepos] = node.lon;
          nodebuf[nodepos+1] = node.lat;
          nodes[node.id] = nodebuf+nodepos;
          nodepos += 2;
          if (nodepos >= nodelen) {
            fprintf(stderr,"node size too small\n");
            return 1;
          }
          while (node.getTag(&key,&value));
        } else if (d.type == o5mdecoder::WAY) {
          printf("way:");
          while (way.getRef(&ref)) {
            printf(" %f,%f", nodes[ref][0], nodes[ref][1]);
          }
          printf("\n");
          while (way.getTag(&key,&value));
        } else if (d.type == o5mdecoder::REL) {
          while (rel.getMember(&memtype,&ref,&memrole));
          while (rel.getTag(&key,&value));
        }
      }
    } catch (char *err) {
      printf("error: %s\n", err);
    }
  } while (len == 4096);
  return 0;
}
