#include <pebble.h>

static Window *s_window;
static TextLayer *s_time_layer, *s_date_layer;
static BitmapLayer *s_background_layer;
static GBitmap *s_bitmap_poogie;

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);
  text_layer_set_text(s_time_layer, s_buffer);
}

static void update_date() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  static char s_buffer[16];
  strftime(s_buffer, sizeof(s_buffer), "%B %e", tick_time);
  text_layer_set_text(s_date_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  if (units_changed & DAY_UNIT) {
    update_date();
  }
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_background_layer = bitmap_layer_create(GRect(0, 15, bounds.size.w, 65));
  bitmap_layer_set_compositing_mode(s_background_layer, GCompOpSet);
  s_bitmap_poogie = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_POOGIE);
  bitmap_layer_set_bitmap(s_background_layer, s_bitmap_poogie);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));

  s_time_layer = text_layer_create(GRect(0, bounds.size.h-90, bounds.size.w, 45));
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_36_BOLD_NUMBERS));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  update_time();
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  s_date_layer = text_layer_create(GRect(0, bounds.size.h-50, bounds.size.w, 45));
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  update_date();
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
}

static void prv_window_unload(Window *window) {
  gbitmap_destroy(s_bitmap_poogie);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  bitmap_layer_destroy(s_background_layer);
}

static void prv_init(void) {
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  const bool animated = true;
  window_stack_push(s_window, animated);
}

static void prv_deinit(void) {
  window_destroy(s_window);
  tick_timer_service_unsubscribe();
}

int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}
