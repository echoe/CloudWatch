#include <pebble.h>
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_bt_layer;
static TextLayer *s_batt_layer;
static GFont s_time_font;
static GFont s_date_font;
static GFont s_bt_font;
static GFont s_batt_font;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
//To show if connected to bluetooth
static void bt_handler(bool connected) {
  // Show current connection state
  if (connected) {
    text_layer_set_text(s_bt_layer, "BT");
  } else {
    text_layer_set_text(s_bt_layer, "No BT");
  }
}
// To Show Battery Level
static void battery_handler(BatteryChargeState new_state) {
  // Write to buffer and display
  static char s_batt_buffer[32];
  snprintf(s_batt_buffer, sizeof(s_batt_buffer), "%d", new_state.charge_percent);
  text_layer_set_text(s_batt_layer, s_batt_buffer);
}
static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";
  static char date_buffer[10]; //idk why this is 10! :D
  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
  // Write the current date
  strftime(date_buffer, sizeof(date_buffer), "%b %e", tick_time);
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
  // Display this date on the TextLayer
  text_layer_set_text(s_date_layer, date_buffer);
}
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}
static void main_window_load(Window *window) {
  // Create GBitmap, then set to created BitmapLayer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CLOUDFACE);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));

  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 107, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  // Improve the layout to be more like a watchface
  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CHERL_44));

  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));

  // Create date TextLayer with all those steps too!
  s_date_layer = text_layer_create(GRect(0, 81, 144, 56));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorBlack);
	text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CHERL_26));
  text_layer_set_font(s_date_layer, s_date_font);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));

  // Create bluetooth TextLayer and display status
  s_bt_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_QUIRLY_16));
  s_bt_layer = text_layer_create(GRect(3, 0, 30, 16));
  text_layer_set_font(s_bt_layer, s_bt_font);
  text_layer_set_text_alignment(s_bt_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_bt_layer));
  bt_handler(bluetooth_connection_service_peek());
  
  // Create battery TextLayer and display status
  s_batt_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_QUIRLY_16));
  s_batt_layer = text_layer_create(GRect(115, 0, 29, 16));
  text_layer_set_font(s_batt_layer, s_batt_font);
  text_layer_set_text_alignment(s_batt_layer, GTextAlignmentRight);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_batt_layer));
  battery_handler(battery_state_service_peek());
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  // Unload GFont
  fonts_unload_custom_font(s_time_font);
  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);
  // Unload date stuff
  text_layer_destroy(s_date_layer);
  fonts_unload_custom_font(s_date_font);
  //Unload battery/bluetooth stuff
  text_layer_destroy(s_bt_layer);
  fonts_unload_custom_font(s_bt_font);
  text_layer_destroy(s_batt_layer);
  fonts_unload_custom_font(s_batt_font);
  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
}
static void init() {
// Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  // Update the time
  update_time();
  // Subscribe to Bluetooth updates
  bluetooth_connection_service_subscribe(bt_handler);
  // Subscribe to the Battery State Service
  battery_state_service_subscribe(battery_handler);
}

static void deinit() {
// Destroy Window
    window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}