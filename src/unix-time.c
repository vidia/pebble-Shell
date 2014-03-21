#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static TextLayer *time_layer; 
static TextLayer *prompt_layer; 

GFont start_font;// = fonts_load_custom_font( resource_get_handle(RESOURCE_ID_FONT_START_12) );
GFont font_large; 
static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	start_font = fonts_load_custom_font( resource_get_handle(RESOURCE_ID_FONT_SYS_16) );	
	font_large = fonts_load_custom_font( resource_get_handle(RESOURCE_ID_FONT_SYS_30) );

	text_layer = text_layer_create((GRect) { .origin = {5,5}, .size = { bounds.size.w, 20 } });
	text_layer_set_text(text_layer, "~$date +%I:%M");
	text_layer_set_text_color(text_layer, GColorWhite);
	text_layer_set_background_color(text_layer, GColorClear); 
	text_layer_set_text_alignment(text_layer, GTextAlignmentLeft);
	text_layer_set_font(text_layer, start_font); 

	time_layer = text_layer_create((GRect) { .origin = {5, 27}, .size = { bounds.size.w, 30 } });
	text_layer_set_text_color(time_layer, GColorWhite);
	text_layer_set_background_color(time_layer, GColorClear); 
	text_layer_set_text_alignment(time_layer, GTextAlignmentLeft);
	text_layer_set_font(time_layer, font_large); 

	prompt_layer = text_layer_create((GRect) { .origin = {5,65}, .size = {bounds.size.w, 20} });
	//text_layer_set_text(prompt_layer, "~$_");
	text_layer_set_text_color(prompt_layer, GColorWhite);
	text_layer_set_background_color(prompt_layer, GColorClear); 
	text_layer_set_text_alignment(prompt_layer, GTextAlignmentLeft);
	text_layer_set_font(prompt_layer, start_font); 

	layer_add_child(window_layer, text_layer_get_layer(text_layer));
	layer_add_child(window_layer, text_layer_get_layer(time_layer));
	layer_add_child(window_layer, text_layer_get_layer(prompt_layer));
}

static void window_unload(Window *window) {
	text_layer_destroy(text_layer);
	text_layer_destroy(prompt_layer);
	text_layer_destroy(time_layer);

	fonts_unload_custom_font(start_font); 
	fonts_unload_custom_font(font_large); 
}

static void handleMinuteTick(struct tm* now, TimeUnits units_changed)
{
	app_log(APP_LOG_LEVEL_DEBUG, "unix-time.c", 52, "---Minute tick %d", now->tm_min); 
	static char time[] = "00:00"; 

	strftime(time, sizeof(time), "%I:%M", now); 
	text_layer_set_text(time_layer, time);			
}

static void handleSecondTick(struct tm* now, TimeUnits units_changed)
{
	app_log(APP_LOG_LEVEL_DEBUG, "unix-time.c", 60, "Second tick %d", now->tm_sec); 
	static char prompt[] = "~$      ";
	static bool cursor = true; 
	static int cursor_loc = 2; 

	if(now->tm_sec < 57) 
	{
		if( prompt[3] != ' ')
		{
			app_log(APP_LOG_LEVEL_DEBUG, "unix-time.c", 67, "clearing prompt..."); 
			cursor_loc = 2; 
			strcpy(prompt+2, "      ");
		}
	}
	else if(prompt[3] != 'l')
	{	
		cursor_loc = 7; 
		strcpy(prompt+2, "clear ");
	}
	
	if ((units_changed & MINUTE_UNIT) != 0)
		handleMinuteTick(now, MINUTE_UNIT);
	
	prompt[cursor_loc] = (cursor) ? '_' : ' '; 
	cursor = cursor ? false : true ; 
	text_layer_set_text(prompt_layer, prompt); 
}

static void init(void) {
	window = window_create();
	window_set_background_color(window, GColorBlack); 
	window_set_window_handlers(window, (WindowHandlers) {
			.load = window_load,
			.unload = window_unload,
			});
	const bool animated = true;
	window_stack_push(window, animated);

	time_t now = time(NULL); 
	struct tm *currentTime = localtime(&now); 
	handleMinuteTick(currentTime, MINUTE_UNIT);
	handleSecondTick(currentTime, SECOND_UNIT);
	tick_timer_service_subscribe(SECOND_UNIT, &handleSecondTick); 
}

static void deinit(void) {
	text_layer_destroy(text_layer);
	window_destroy(window);
}

int main(void) {
	init();

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

	app_event_loop();
	deinit();
}
