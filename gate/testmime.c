#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

char *decode_mime_string(const char *buf);

int main() {
  char *input, *output;

  for(input = readline("Input: ");input;
    input=readline("Input: ") ) {
    
    output=decode_mime_string(input);
    printf("Output: %s\n", output);
  }

  printf("\n");

  return 0;
}
