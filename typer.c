#include <pebble.h>
#include "typer.h"

struct typer_data *init_typer(char* text, TextLayer* tl, int time, void (*onFinish)())
{
  struct typer_data *data = (struct typer_data*) malloc(sizeof(struct typer_data));

  data->buffer = (char*) malloc(sizeof(char) * strlen(text) + 1);

  data->index = 0; 
  data->text = text; 
  data->length = strlen(data->text);
  data->tickTime = time; 
  data->layer = tl; 
  data->onFinish = onFinish;
  data->cont = 1;

  return data; 
}

void destroy_typer(struct typer_data* data)
{
  static char empty[1] = ""; 
  text_layer_set_text(data->layer, empty);
  free(data->buffer);
  free(data);
}

void typeTextInTextLayer(void *data)
{
  struct typer_data *TYPER = (struct typer_data*) data; 

  memset(TYPER->buffer, 0, strlen(TYPER->buffer)+1);
  strncpy(TYPER->buffer, TYPER->text, TYPER->index);
  TYPER->index++; 
  text_layer_set_text(TYPER->layer, TYPER->buffer);

  if( TYPER->index > TYPER->length )
  {
    TYPER->onFinish();
  }
  else 
  {
    if(TYPER->cont == 1)
    {
      app_timer_register(TYPER->tickTime, typeTextInTextLayer, (void*) TYPER); 
    }
  }
}

void typer_cancel(struct typer_data* data)
{
  data->cont = 0;
}

