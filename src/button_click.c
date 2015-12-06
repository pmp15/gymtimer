#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static TextLayer *info_layer;

enum enum_flag_mode {
	timer, 
	menu,
	reps
};

static enum enum_flag_mode flag_mode=menu;
static bool flag_pause=false;
static int flag_steps=0;
static int flag_argument=0;
static int flag_seconds=0;
static int flag_set=0;

static int total_sets=4;
static int total_reps=10;
static int total_rest=60;

static void buzz(int length) {
	uint32_t segments[] = { length };
	VibePattern pattern = {
		.durations = segments,
		.num_segments = ARRAY_LENGTH(segments)
	};
	vibes_enqueue_custom_pattern(pattern);
}

static void switchmode(enum enum_flag_mode newmode, int argument) {
	flag_seconds=-1;
	flag_mode=newmode;
	flag_steps=0;
	flag_argument=argument;
	static char strbuffer[8]={0};
	switch(newmode) {
		case menu:
			text_layer_set_text(info_layer, "");
		break;
		case timer:
			text_layer_set_text(info_layer, "REST");
		break;
		case reps:
			snprintf(strbuffer, sizeof(strbuffer), "SET %d", flag_set);
			text_layer_set_text(info_layer, strbuffer);
		break;
	}
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer, "Select");
	if(flag_mode==menu) {
		flag_set=0;
		switchmode(timer,3);
	}
	else {
		flag_pause=!flag_pause;
		text_layer_set_text(text_layer, "STOP");
	}
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer, "Up");
	if(flag_pause) {
		switchmode(timer,3);
		flag_set--;
		flag_pause=false;
	}
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer, "Down");
	if(flag_pause) {
		switchmode(timer,3);
		flag_set--;
		flag_pause=false;
	}
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
if(!flag_pause) {
	int remaining;
	remaining=(flag_argument-flag_seconds);
	static char strbuffer[8]={0};
	printf("tick %d", flag_seconds);
	switch(flag_mode) {
		case menu: 
		break;
		
		case timer:
			snprintf(strbuffer, sizeof(strbuffer), "%d", remaining);
			text_layer_set_text(text_layer, strbuffer);
			if(remaining<=3) {
				buzz(300);
				text_layer_set_text(info_layer, "GET READY");
			}
			if(remaining<=1) {
				flag_set++;
				switchmode(reps, total_reps);
			}
		break;
		
		case reps:
			if(flag_seconds==0||flag_seconds==5) {
				flag_seconds=0;
				flag_steps++;
				printf("     step %d", flag_steps);
				snprintf(strbuffer, sizeof(strbuffer), "%d", flag_steps);
				text_layer_set_text(text_layer, strbuffer);
				buzz(1300);
			}
			if(flag_seconds==2||flag_seconds==3||flag_seconds==4) {
				buzz(250);
			}
			if(flag_steps>=flag_argument) {
				if (flag_set<total_sets) {switchmode(timer, total_rest);}
				else {switchmode(menu, 0);}
			}
		break;
	}
	if (!flag_pause) {flag_seconds++;}
}}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

   text_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(text_layer, GColorClear);
  text_layer_set_text_color(text_layer, GColorBlack);
  text_layer_set_text(text_layer, "Hi");
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  //text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
	
	
	info_layer = text_layer_create(
      GRect(0, 30, bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(info_layer, GColorClear);
  text_layer_set_text_color(info_layer, GColorBlack);
  text_layer_set_text(info_layer, "Yo");
  text_layer_set_font(info_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(info_layer, GTextAlignmentCenter);
	
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(info_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
	.load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}