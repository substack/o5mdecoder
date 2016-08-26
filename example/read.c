#include <stdio.h>
#include <stdlib.h>
#include <o5mdecoder.h>

int main (int argc, char **argv) {
  char *data = (char*) malloc(4096);
  char *dbuf = (char*) malloc(4096);
  o5mdecoder::Decoder d(dbuf);
  o5mdecoder::Doc doc;
  size_t len;
  do {
    len = fread(data, sizeof(char), 4096, stdin);
    d.write(data, len);
    try {
      while (d.read(&doc)) {
        printf("id=%d\n", doc.id);
      }
    } catch (char *err) {
      fprintf(stderr, "error: %s\n", err);
    }
  } while (len == 4096);
  return 0;
}
