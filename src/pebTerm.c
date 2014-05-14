/*
    Pebble Terminal: A Pebble face that simulates a linux terminal running the date command.
	Copyright (C) 2014  David Tschida

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
   */

#include <pebble.h>
#include "typer.h"

static Window *window;
static TextLayer *time_prompt_layer;
static TextLayer *time_layer; 
static TextLayer *dprompt_layer; 
static TextLayer *prompt_layer; 
static TextLayer *date_layer; 

GFont start_font;// = fonts_load_custom_font( resource_get_handle(RESOURCE_ID_FONT_START_12) );
GFont font_large; 

static char _24hourmin[] = "~$date +%H:%M";
static char _12hourmin[] = "~$date +%I:%M";
static char *hourmin;

static char _24hourformat[] = "%H:%M";
static char _12hourformat[] = "%I:%M";
static char *timeFormat;

static char monthday[] = "~$date +%h\\ %d";

static struct tm* lastTime;

static int TYPING_TICK = 150; 

static struct typer_data *time_command_data;
static struct typer_data *date_command_data;
static struct typer_data *clear_command_data;

static char prompt[] = "~$      ";

static bool promptVisible = false; 

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	start_font = fonts_load_custom_font( resource_get_handle(RESOURCE_ID_FONT_SOURCE_CODE_PRO_16) );	
	font_large = fonts_load_custom_font( resource_get_handle(RESOURCE_ID_FONT_SOURCE_CODE_PRO_30) );

	time_prompt_layer = text_layer_create((GRect) { .origin = {5,5}, .size = { bounds.size.w, 20 } });
	text_layer_set_text_color(time_prompt_layer, GColorClear);
	text_layer_set_background_color(time_prompt_layer, GColorClear); 
	text_layer_set_text_alignment(time_prompt_layer, GTextAlignmentLeft);
	text_layer_set_font(time_prompt_layer, start_font); 

	time_layer = text_layer_create((GRect) { .origin = {5, 20}, .size = { bounds.size.w, 30 } });

	text_layer_set_text_color(time_layer, GColorClear);
	text_layer_set_background_color(time_layer, GColorClear); 
	text_layer_set_text_alignment(time_layer, GTextAlignmentLeft);
	text_layer_set_font(time_layer, font_large); 

	dprompt_layer = text_layer_create((GRect) { .origin = {5,55}, .size = {bounds.size.w, 20}});
	text_layer_set_text_color(dprompt_layer, GColorClear);
	text_layer_set_background_color(dprompt_layer, GColorClear); 
	text_layer_set_text_alignment(dprompt_layer, GTextAlignmentLeft);
	text_layer_set_font(dprompt_layer, start_font); 

	date_layer = text_layer_create((GRect) { .origin = {5, 70}, .size = { bounds.size.w, 39 } });

	text_layer_set_text_color(date_layer, GColorClear);
	text_layer_set_background_color(date_layer, GColorClear); 
	text_layer_set_text_alignment(date_layer, GTextAlignmentLeft);
	text_layer_set_font(date_layer, font_large); 

	prompt_layer = text_layer_create((GRect) { .origin = {5,80+25}, .size = {bounds.size.w, 20} });
	text_layer_set_text_color(prompt_layer, GColorClear);
	text_layer_set_background_color(prompt_layer, GColorClear); 
	text_layer_set_text_alignment(prompt_layer, GTextAlignmentLeft);
	text_layer_set_font(prompt_layer, start_font); 

	layer_add_child(window_layer, text_layer_get_layer(time_prompt_layer));
	layer_add_child(window_layer, text_layer_get_layer(time_layer));
	layer_add_child(window_layer, text_layer_get_layer(dprompt_layer));
	layer_add_child(window_layer, text_layer_get_layer(prompt_layer));
	layer_add_child(window_layer, text_layer_get_layer(date_layer));
}

static void window_unload(Window *window) {
	text_layer_destroy(time_prompt_layer);
	text_layer_destroy(dprompt_layer);
	text_layer_destroy(prompt_layer);
	text_layer_destroy(time_layer);
	text_layer_destroy(date_layer);

	fonts_unload_custom_font(start_font); 
	fonts_unload_custom_font(font_large); 

  if(date_command_data != NULL)
    destroy_typer(date_command_data);
  if(time_command_data != NULL)
    destroy_typer(time_command_data);
  if(clear_command_data != NULL)
    destroy_typer(clear_command_data);
}

static void onDateTypeFinish()
{
  static char date[] = "      ";
	strftime(date, sizeof(date), "%h %d", lastTime);
	text_layer_set_text(date_layer, date);

  promptVisible = true; 
}

static void onTimeTypeFinish()
{
	app_log(APP_LOG_LEVEL_DEBUG, "unix-time.c", 52, "TYPING COMPLETE!"); 
  
  static char time[] = "00:00"; 
	strftime(time, sizeof(time),timeFormat, lastTime); 
	text_layer_set_text(time_layer, time);

  if(date_command_data != NULL)
    destroy_typer(date_command_data);
  date_command_data = init_typer(monthday, dprompt_layer, TYPING_TICK, onDateTypeFinish, 2);
  typeTextInTextLayer((void*) date_command_data);
}

static void onClear()
{
  //text_layer_set_text(prompt_layer, "~$ ");
  strcpy(prompt+2, "      "); 

	promptVisible=false; 

	text_layer_set_text(time_layer, "");
	text_layer_set_text(date_layer, "");
	text_layer_set_text(dprompt_layer, "");
	text_layer_set_text(prompt_layer, "");

  if(time_command_data != NULL)
    destroy_typer(time_command_data);
  time_command_data = init_typer(hourmin, time_prompt_layer, TYPING_TICK, onTimeTypeFinish, 2);
  typeTextInTextLayer((void*) time_command_data); 
}

static void handleMinuteTick(struct tm* now, TimeUnits units_changed)
{
	lastTime = now; 

	app_log(APP_LOG_LEVEL_DEBUG, "unix-time.c", 52, "---Minute tick %d", now->tm_min); 
  if(clear_command_data != NULL)
    destroy_typer(clear_command_data);
  clear_command_data = init_typer(strcpy(prompt+2, "clear") - 2, prompt_layer, TYPING_TICK, onClear, 2);
  typeTextInTextLayer((void*) clear_command_data);
}

static void handleSecondTick(struct tm* now, TimeUnits units_changed)
{
	if(promptVisible)
	{
		app_log(APP_LOG_LEVEL_DEBUG, "unix-time.c", 60, "Second tick %d", now->tm_sec); 
		static bool cursor = true; 
		static int cursor_loc = 2; 

    if(clear_command_data != NULL && clear_command_data->finished == false)
    {
      //cursor_loc = clear_command_data->index;
      return;
    }

		prompt[cursor_loc] = (cursor) ? '_' : ' '; 
		cursor = cursor ? false : true ; 
		text_layer_set_text(prompt_layer, prompt); 
	}
}

static void handleTicks(struct tm* now, TimeUnits units_changed)
{
	if ((units_changed & SECOND_UNIT) != 0)
		handleSecondTick(now, units_changed); 
	if ((units_changed & MINUTE_UNIT) != 0)
		handleMinuteTick(now, units_changed);
}

static void init(void) {
	window = window_create();
	window_set_background_color(window, GColorBlack); 
	window_set_window_handlers(window, (WindowHandlers) {
			.load = window_load,
			.unload = window_unload,
			});
	const bool animated = true;
	if(clock_is_24h_style())
	{
		
		APP_LOG(APP_LOG_LEVEL_DEBUG, "In 24 hour mode: %s", _24hourmin);
		hourmin = _24hourmin; 
		timeFormat = _24hourformat; 
	}
	else 
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "In 12 hour mode: %s", _12hourmin);
		hourmin = _12hourmin; 
		timeFormat = _12hourformat; 

	}

	window_stack_push(window, animated);

	time_t now = time(NULL); 
	struct tm *currentTime = localtime(&now); 
  lastTime = currentTime; 
  onClear(); 
//	handleTicks(currentTime, MINUTE_UNIT | SECOND_UNIT | DAY_UNIT);
	tick_timer_service_subscribe(SECOND_UNIT, &handleTicks); 
}

static void deinit(void) {
//	text_layer_destroy(time_prompt_layer);
	window_destroy(window);
}

int main(void) {
	init();

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

	app_event_loop();
  
	APP_LOG(APP_LOG_LEVEL_DEBUG, "leaving event loop.");

  deinit();

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Finished");
}
