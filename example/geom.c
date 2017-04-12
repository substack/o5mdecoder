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
  o5mdecoder::Node node;
  o5mdecoder::Way way;
  o5mdecoder::Rel rel;
  uint64_t ref;
  std::map<uint64_t,Point*> nodes;
  std::map<uint64_t,Point*>::const_iterator ipt;

  size_t len;
  do {
    len = fread(data, sizeof(char), 4096, stdin);
    d.write(data, len);
    try {
      while (d.read(&node, &way, &rel)) {
        if (d.type == o5mdecoder::NODE) {
          nodes[node.id] = new Point(node.lon,node.lat);
        } else if (d.type == o5mdecoder::WAY) {
          while (way.getRef(&ref)) {
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
