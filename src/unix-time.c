#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static TextLayer *time_layer; 

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
	
	layer_add_child(window_layer, text_layer_get_layer(text_layer));
	layer_add_child(window_layer, text_layer_get_layer(time_layer));
}

static void window_unload(Window *window) {
	text_layer_destroy(text_layer);
	text_layer_destroy(time_layer);

	fonts_unload_custom_font(start_font); 
	fonts_unload_custom_font(font_large); 
}

static void handleMinuteTick(struct tm* now, TimeUnits units_changed)
{
	static char time[] = "00:00"; 

	strftime(time, sizeof(time), "%I:%M", now); 
	text_layer_set_text(time_layer, time);			
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
	handleMinuteTick(currentTime, SECOND_UNIT);
	tick_timer_service_subscribe(SECOND_UNIT, &handleMinuteTick); 
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
