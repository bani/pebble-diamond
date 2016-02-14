#include <pebble.h>

#define KEY_GEM_COLOR 0

Window *window;
TextLayer *s_time_layer;
static GBitmap *s_bitmap;
static BitmapLayer *s_bitmap_layer;

static void set_background_gem(int color) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Updating background gem");
  if (s_bitmap) gbitmap_destroy(s_bitmap);
  switch(color) {
    case 0:
      s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DIAMOND);
      break;
    case 1:
      s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DIAMONDAQUA);
      break;
    case 2:
      s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DIAMONDBLUE);
      break;
    case 3:
      s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DIAMONDGREEN);
      break;
    case 4:
      s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DIAMONDPINK);
      break;
    case 5:
      s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DIAMONDPURPLE);
      break;
    case 6:
      s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DIAMONDRED);
      break;
    case 7:
      s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DIAMONDYELLOW);
      break;
    default:
      s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DIAMOND);
  }
  bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Background Image
  s_bitmap_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);
  set_background_gem(0);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));

  // Time Layout
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  bitmap_layer_destroy(s_bitmap_layer);
  gbitmap_destroy(s_bitmap);
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "New config received");
  Tuple *gem_color_t = dict_find(iter, KEY_GEM_COLOR);

  if (gem_color_t) {
    int gem_color = gem_color_t->value->int32;
    persist_write_int(KEY_GEM_COLOR, gem_color);
    set_background_gem(gem_color);
  }
}

void handle_init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_stack_push(window, true);
  
  app_message_open(64, 64);
  app_message_register_inbox_received(inbox_received_handler);
  
  update_time();
  
  // App Logging!
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Just pushed a window!");
}

void handle_deinit(void) {
  window_destroy(window);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

void start_clock(void) {
  typedef void(* TickHandler)(struct tm *tick_time, TimeUnits units_changed);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

int main(void) {
  handle_init();
  start_clock();
  app_event_loop();
  handle_deinit();
}
