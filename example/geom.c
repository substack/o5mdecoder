#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <o5mdecoder.h>

struct Point { float lon, lat; };

int main (int argc, char **argv) {
  char *data = (char*) malloc(4096);
  char *dbuf = (char*) malloc(16384);
  char *table = (char*) malloc(256*15000);
  o5mdecoder::Decoder d(dbuf,16384,table);
  uint64_t ref;
  std::map<uint64_t,Point> nodes;
  std::map<uint64_t,Point>::const_iterator ipt;
  Point *pt;

  size_t len;
  do {
    len = fread(data, sizeof(char), 4096, stdin);
    d.write(data, len);
    try {
      while (d.read()) {
        if (d.node) {
          pt = &(nodes[d.node->id]);
          pt->lon = d.node->lon;
          pt->lat = d.node->lat;
        } else if (d.way) {
          while (d.way->getRef(&ref)) {
            ipt = nodes.find(ref);
            if (ipt == nodes.end()) continue;
            printf("%f,%f ", ipt->second.lon, ipt->second.lat);
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
