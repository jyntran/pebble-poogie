#include <pebble.h>

static Window *s_window;
static TextLayer *s_time_layer, *s_date_layer;
static BitmapLayer *s_background_layer;
static GBitmap *s_bitmap_poogie;
static BitmapLayer *s_battery_layer;
static GBitmap *s_bitmap_battery;

static int s_battery_level;

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

static void update_battery() {
  switch(s_battery_level) {
    case 100:
      s_bitmap_battery = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SHARP100);
      break;
    case 90:
      s_bitmap_battery = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SHARP090);
      break;
    case 80:
      s_bitmap_battery = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SHARP080);
      break;
    case 70:
      s_bitmap_battery = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SHARP070);
      break;
    case 60:
      s_bitmap_battery = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SHARP060);
      break;
    case 50:
      s_bitmap_battery = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SHARP050);
      break;
    case 40:
      s_bitmap_battery = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SHARP040);
      break;
    case 30:
      s_bitmap_battery = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SHARP030);
      break;
    case 20:
      s_bitmap_battery = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SHARP020);
      break;
    case 10:
      s_bitmap_battery = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SHARP010);
      break;
    default:
      s_bitmap_battery = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SHARP000);
      break;
  }
}

static void battery_callback(BatteryChargeState state) {
  s_battery_level = state.charge_percent;
  update_battery();
}

static void bluetooth_callback(bool connected) {
  if (!connected) {   
    static const uint32_t const segments[] = { 300, 300, 300, 300, 300 };
    VibePattern pat = {
      .durations = segments,
      .num_segments = ARRAY_LENGTH(segments),
    };
    vibes_enqueue_custom_pattern(pat);
    s_bitmap_poogie = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_POOGIE_ANGRY);
  } else {
    s_bitmap_poogie = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_POOGIE);
  }
  bitmap_layer_set_bitmap(s_background_layer, s_bitmap_poogie);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_background_layer = bitmap_layer_create(GRect(0, 20, bounds.size.w, 65));
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

  s_battery_layer = bitmap_layer_create(GRect(1, 2, bounds.size.w-2, 25));
  bitmap_layer_set_compositing_mode(s_battery_layer, GCompOpSet);
  bitmap_layer_set_alignment(s_battery_layer, GAlignLeft);
  battery_callback(battery_state_service_peek());
  bitmap_layer_set_bitmap(s_battery_layer, s_bitmap_battery);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_battery_layer));  

  bluetooth_callback(connection_service_peek_pebble_app_connection());
}

static void prv_window_unload(Window *window) {
  gbitmap_destroy(s_bitmap_poogie);
  gbitmap_destroy(s_bitmap_battery);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  bitmap_layer_destroy(s_background_layer);
  bitmap_layer_destroy(s_battery_layer);
}

static void prv_init(void) {
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_callback);
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });
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
  connection_service_unsubscribe();
  battery_state_service_unsubscribe();
}

int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}
