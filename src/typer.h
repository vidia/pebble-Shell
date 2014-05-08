#include <pebble.h>

struct typer_data {
  void (*onFinish)();
  TextLayer *layer;
  char* text; 
  size_t index; 
  size_t length; 
  int tickTime;
  char* buffer; 
  int cont; 
};

struct typer_data *init_typer(char* text, TextLayer* tl, int time, void (*onFinish)());
void destroy_typer(struct typer_data* data);
void typeTextInTextLayer(void *data);
void typer_cancel(struct typer_data* data);
