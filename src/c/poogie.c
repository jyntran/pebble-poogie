#include <pebble.h>

static Window *s_window;
static TextLayer *s_time_layer,
                 *s_date_layer;
static BitmapLayer *s_background_layer;
static GBitmap *s_bitmap_poogie,
               *s_bitmap_poogie_angry,
               *s_bitmap_poogie_heart;
static BitmapLayer *s_battery_layer;
static GBitmap *s_bitmap_battery;
static Layer *s_batt_layer;

static int s_battery_level;

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);
  text_layer_set_text(s_time_layer, s_buffer);

  bitmap_layer_set_bitmap(s_background_layer, s_bitmap_poogie);
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

static void batt_update_proc(Layer *layer, GContext *ctx) {
  GRect red1 = GRect(PBL_IF_ROUND_ELSE(54, 36), PBL_IF_ROUND_ELSE(23, 10), 8, 8);
  GRect red2 = GRect(PBL_IF_ROUND_ELSE(62, 44), PBL_IF_ROUND_ELSE(23, 10), 8, 8);
  GRect orange1 = GRect(PBL_IF_ROUND_ELSE(70, 52), PBL_IF_ROUND_ELSE(23, 10), 8, 8);
  GRect orange2 = GRect(PBL_IF_ROUND_ELSE(78, 60), PBL_IF_ROUND_ELSE(23, 10), 8, 8);
  GRect yellow1 = GRect(PBL_IF_ROUND_ELSE(86, 68), PBL_IF_ROUND_ELSE(23, 10), 8, 8);
  GRect yellow2 = GRect(PBL_IF_ROUND_ELSE(94, 76), PBL_IF_ROUND_ELSE(23, 10), 8, 8);
  GRect green1 = GRect(PBL_IF_ROUND_ELSE(102, 84), PBL_IF_ROUND_ELSE(23, 10), 8, 8);
  GRect green2 = GRect(PBL_IF_ROUND_ELSE(110, 92), PBL_IF_ROUND_ELSE(23, 10), 8, 8);
  GRect blue = GRect(PBL_IF_ROUND_ELSE(118, 100), PBL_IF_ROUND_ELSE(23, 10), 8, 8);
  GRect white = GRect(PBL_IF_ROUND_ELSE(126, 108), PBL_IF_ROUND_ELSE(23, 10), 4, 8);

  if (s_battery_level > 0) {
    graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(GColorWhite, GColorRed));
    graphics_fill_rect(ctx, red1, 0, GCornersAll);
    if (s_battery_level > 10) {
      graphics_fill_rect(ctx, red2, 0, GCornersAll);
      if (s_battery_level > 20) {
        graphics_context_set_fill_color(ctx, GColorRajah);
        graphics_fill_rect(ctx, orange1, 0, GCornersAll);
        if (s_battery_level > 30) {
          graphics_fill_rect(ctx, orange2, 0, GCornersAll);
          if (s_battery_level > 40) {
            graphics_context_set_fill_color(ctx, GColorYellow);
            graphics_fill_rect(ctx, yellow1, 0, GCornersAll);
            if (s_battery_level > 50) {
              graphics_fill_rect(ctx, yellow2, 0, GCornersAll);
              if (s_battery_level > 60) {
                graphics_context_set_fill_color(ctx, GColorGreen);
                graphics_fill_rect(ctx, green1, 0, GCornersAll);
                if (s_battery_level > 70) {
                  graphics_fill_rect(ctx, green2, 0, GCornersAll);
                  if (s_battery_level > 80) {
                    graphics_context_set_fill_color(ctx, PBL_IF_BW_ELSE(GColorWhite, GColorLiberty));
                    graphics_fill_rect(ctx, blue, 0, GCornersAll);
                    if (s_battery_level > 90) {
                      graphics_context_set_fill_color(ctx, GColorWhite);
                      graphics_fill_rect(ctx, white, 0, GCornersAll);
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

static void battery_callback(BatteryChargeState state) {
  s_battery_level = state.charge_percent;
  layer_mark_dirty(s_batt_layer);
}

static void bluetooth_callback(bool connected) {
  if (!connected) {   
    static const uint32_t const segments[] = { 300, 300, 300, 300, 300 };
    VibePattern pat = {
      .durations = segments,
      .num_segments = ARRAY_LENGTH(segments),
    };
    vibes_enqueue_custom_pattern(pat);
    bitmap_layer_set_bitmap(s_background_layer, s_bitmap_poogie_angry);
  } else {
    bitmap_layer_set_bitmap(s_background_layer, s_bitmap_poogie_heart);
  }
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_background_layer = bitmap_layer_create(GRect(0, PBL_IF_ROUND_ELSE(33, 20), bounds.size.w, 65));
  bitmap_layer_set_compositing_mode(s_background_layer, GCompOpSet);
  s_bitmap_poogie = gbitmap_create_with_resource(PBL_IF_BW_ELSE(RESOURCE_ID_IMAGE_POOGIEBW, RESOURCE_ID_IMAGE_POOGIE));
  s_bitmap_poogie_angry = gbitmap_create_with_resource(PBL_IF_BW_ELSE(RESOURCE_ID_IMAGE_POOGIEBW_ANGRY, RESOURCE_ID_IMAGE_POOGIE_ANGRY));
  s_bitmap_poogie_heart = gbitmap_create_with_resource(PBL_IF_BW_ELSE(RESOURCE_ID_IMAGE_POOGIEBW_HEART, RESOURCE_ID_IMAGE_POOGIE_HEART));
  bitmap_layer_set_bitmap(s_background_layer, s_bitmap_poogie);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));

  s_time_layer = text_layer_create(GRect(0, bounds.size.h-85, bounds.size.w, 40));
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  update_time();
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  s_date_layer = text_layer_create(GRect(0, bounds.size.h-50, bounds.size.w, 40));
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  update_date();
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  s_battery_layer = bitmap_layer_create(GRect(0, PBL_IF_ROUND_ELSE(15, 2), bounds.size.w, 25));
  bitmap_layer_set_compositing_mode(s_battery_layer, GCompOpSet);
  bitmap_layer_set_alignment(s_battery_layer, GAlignCenter);
  s_bitmap_battery = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SHARP000);
  bitmap_layer_set_bitmap(s_battery_layer, s_bitmap_battery);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_battery_layer));

  s_batt_layer = layer_create(bounds);
  layer_set_update_proc(s_batt_layer, batt_update_proc);
  battery_callback(battery_state_service_peek());
  layer_add_child(window_layer, s_batt_layer);

  bluetooth_callback(connection_service_peek_pebble_app_connection());
}

static void prv_window_unload(Window *window) {
  gbitmap_destroy(s_bitmap_poogie);
  gbitmap_destroy(s_bitmap_poogie_angry);
  gbitmap_destroy(s_bitmap_poogie_heart);
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
