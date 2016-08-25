#include <stdio.h>
#include <o5mdecoder.h>

int main (int argc, char **argv) {
  char *data = (char*) malloc(4096);
  o5mdecoder::Decoder d;
  o5mdecoder::Doc *doc;
  size_t len;
  do {
    len = fread(data, sizeof(char), 4096, stdin);
    d.write(data, len);
    while ((doc = d.read()) !== NULL) {
      printf("%d\n", doc.id);
    }
  } while (len == 4096);
  return 0;
}
