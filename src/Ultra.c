#include <pebble.h>

static Window *window;
static GBitmap *battery_image, *bluetooth_image, *dayOfWeek_image;
static BitmapLayer *battery_image_layer, *bluetooth_image_layer, *dayOfWeek_layer;
static TextLayer *charging_layer, *battery_layer, *time_layer, *date_layer;

static void handle_time_and_date() {
    time_t temp = time(NULL); 
    struct tm *current_time = localtime(&temp);
    char *time_format = "%R";
    static char time_text[6];
    static char date_text[17];
    int weekday = current_time->tm_wday;
    
    if (dayOfWeek_image)
        gbitmap_destroy(dayOfWeek_image);

    switch (weekday) {
        case 0:
            dayOfWeek_image = gbitmap_create_with_resource(RESOURCE_ID_SUN);
            break;
        case 1:
            dayOfWeek_image = gbitmap_create_with_resource(RESOURCE_ID_MON);            
            break;
        case 2:
            dayOfWeek_image = gbitmap_create_with_resource(RESOURCE_ID_TUE);
            break;
        case 3:
            dayOfWeek_image = gbitmap_create_with_resource(RESOURCE_ID_WED);
            break;
        case 4:
            dayOfWeek_image = gbitmap_create_with_resource(RESOURCE_ID_THU);
            break;
        case 5:
            dayOfWeek_image = gbitmap_create_with_resource(RESOURCE_ID_FRI);
            break;
        case 6:
            dayOfWeek_image = gbitmap_create_with_resource(RESOURCE_ID_SAT);
            break;
    }
    
    if (!clock_is_24h_style())
        time_format = "%I:%M";

    strftime(time_text, sizeof(time_text), time_format, current_time);
    strftime(date_text, sizeof(date_text), "%e %b %Y", current_time);

    text_layer_set_text(date_layer, date_text);
    text_layer_set_text(time_layer, time_text);
}

static void handle_battery() {
    BatteryChargeState charge_state = battery_state_service_peek();
    static char charging_status[] = "xxxxxxxxxxx";
    static char battery_status[] = "xxx%";
    
    if (battery_image)
        gbitmap_destroy(battery_image);
    
    if (charge_state.charge_percent > 75)
        battery_image = gbitmap_create_with_resource(RESOURCE_ID_BATT0);
    else if (charge_state.charge_percent <= 75 && charge_state.charge_percent > 50)
        battery_image = gbitmap_create_with_resource(RESOURCE_ID_BATT1);
    else if (charge_state.charge_percent <= 50 && charge_state.charge_percent > 25)
        battery_image = gbitmap_create_with_resource(RESOURCE_ID_BATT2);
    else if (charge_state.charge_percent <= 25 && charge_state.charge_percent > 0)
        battery_image = gbitmap_create_with_resource(RESOURCE_ID_BATT3);
    else
        battery_image = gbitmap_create_with_resource(RESOURCE_ID_BATT4);
    
    bitmap_layer_set_bitmap(battery_image_layer, battery_image);
    
    if (charge_state.is_charging)
        snprintf(charging_status, sizeof(charging_status), "charging");
    else
        snprintf(charging_status, sizeof(charging_status), "discharging");
    
    text_layer_set_text(charging_layer, charging_status);
    
    snprintf(battery_status, sizeof(battery_status), "%d%%", charge_state.charge_percent);
    text_layer_set_text(battery_layer, battery_status);
}

void handle_bt(bool bt) {
    if (bluetooth_image)
        gbitmap_destroy(bluetooth_image);
    
    if (bt)
        bluetooth_image = gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH_CONNECTED);
    else
        bluetooth_image = gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH_DISCONNECTED);
    
    bitmap_layer_set_bitmap(bluetooth_image_layer, bluetooth_image);
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
    handle_time_and_date();
    handle_battery();
    handle_bt(bluetooth_connection_service_peek());
}

static void window_load(Window *window) {
    //Load time module
    time_layer = text_layer_create(GRect(0, 50, 144, 168));
    text_layer_set_text_color(time_layer, GColorWhite);
    text_layer_set_background_color(time_layer, GColorClear);
    text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
    text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
    
    //Load weekday module
    dayOfWeek_image = gbitmap_create_with_resource(RESOURCE_ID_MON);
    dayOfWeek_layer = bitmap_layer_create(GRect(0, 40, 144, 168));
    bitmap_layer_set_alignment(dayOfWeek_layer, GAlignCenter);
    bitmap_layer_set_bitmap(dayOfWeek_layer, dayOfWeek_image);
    
    //Load date module
    date_layer = text_layer_create(GRect(0, 140, 144, 168));
    text_layer_set_text_color(date_layer, GColorWhite);
    text_layer_set_background_color(date_layer, GColorClear);
    text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
    text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
    
    //Load battery module
    battery_image = gbitmap_create_with_resource(RESOURCE_ID_BATT0);
    battery_image_layer = bitmap_layer_create(GRect(5, 10, 144-5, 168-10));
    bitmap_layer_set_alignment(battery_image_layer, GAlignTopRight);
    bitmap_layer_set_bitmap(battery_image_layer, battery_image);
    charging_layer = text_layer_create(GRect(0, 10, 144, 168));
    text_layer_set_text_color(charging_layer, GColorWhite);
    text_layer_set_text(charging_layer, "xxxxxxxxxxx");
    text_layer_set_background_color(charging_layer, GColorClear);
    text_layer_set_text_alignment(charging_layer, GTextAlignmentCenter);
    //text_layer_set_font(charging_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
    battery_layer = text_layer_create(GRect(0, 25, 144, 168));
    text_layer_set_text_color(battery_layer, GColorWhite);
    text_layer_set_text(battery_layer, "xxxx");
    text_layer_set_background_color(battery_layer, GColorClear);
    text_layer_set_text_alignment(battery_layer, GTextAlignmentCenter);
    text_layer_set_font(battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    
    //Load bluetooth module
    bool bt = bluetooth_connection_service_peek();
    if (bt)
        bluetooth_image = gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH_CONNECTED);
    else
        bluetooth_image = gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH_DISCONNECTED);
    bluetooth_image_layer = bitmap_layer_create(GRect(5, 10, 144-5, 168-10));
    bitmap_layer_set_alignment(bluetooth_image_layer, GAlignTopLeft);
    bitmap_layer_set_bitmap(bluetooth_image_layer, bluetooth_image);
    
    //Add all layers to window
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));
    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(dayOfWeek_layer));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));
    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(battery_image_layer));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(charging_layer));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(battery_layer));
    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(bluetooth_image_layer));
    
    //Update time, date, bluetooth, and battery stats
    handle_time_and_date();
    handle_battery();
    handle_bt(bluetooth_connection_service_peek());
}

static void window_unload(Window *window) {
    //Destroy time module
    text_layer_destroy(time_layer);
    
    //Destroy weekday module
    gbitmap_destroy(dayOfWeek_image);
    bitmap_layer_destroy(dayOfWeek_layer);
    
    //Destroy date module
    text_layer_destroy(date_layer);
    
    //Destroy battery module
    gbitmap_destroy(battery_image);
    bitmap_layer_destroy(battery_image_layer);
    text_layer_destroy(charging_layer);
    text_layer_destroy(battery_layer);
    
    //Destroy bluetooth module
    gbitmap_destroy(bluetooth_image);
    bitmap_layer_destroy(bluetooth_image_layer);
}

static void init(void) {
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
    });
    window_set_background_color(window, GColorBlack);
    window_stack_push(window, true);

    battery_state_service_subscribe(handle_battery);
    tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
    bluetooth_connection_service_subscribe(handle_bt);
}

static void deinit(void) {
    battery_state_service_unsubscribe();
    tick_timer_service_unsubscribe();
    bluetooth_connection_service_unsubscribe();
    
    window_destroy(window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
