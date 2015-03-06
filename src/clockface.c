/*
Lutron Logo Polar Clock watchface

  Based off of Polar Clock watch (SDK 2.0)
	https://github.com/op12/PolarClock2.0

 */

#include "pebble.h"

#define TICKER_THICKNESS 5
#define HOUR_TICKER_RADIUS 20
#define MINUTE_TICKER_RADIUS 30
#define SECOND_TICKER_RADIUS 80
#define LOGO_STARTING_ANGLE 12
    
enum {
    
    KEY_SHOW_DATE_TIME_TEXT = 0x1,   
    KEY_SPIN_LOGO = 0x2,
    KEY_INVERT = 0x3
};

static bool SHOW_DATE_TIME_TEXT = false;
static bool SPIN_LOGO = true;
static bool INVERT = true;

static GColor BACKGROUND_COLOR = GColorBlack;
static GColor FOREGROUND_COLOR = GColorWhite;

Window *window;

Layer *minute_display_layer;
Layer *hour_display_layer;
Layer *second_display_layer;
static BitmapLayer *image_layer;
InverterLayer *image_inverter_layer;
static GBitmap *image;
TextLayer *text_time_layer;
TextLayer *text_date_layer;

const GPathInfo SECOND_SEGMENT_PATH_POINTS = {
  3,
  (GPoint []) {
    {0, 0},
    {-22, -(SECOND_TICKER_RADIUS + TICKER_THICKNESS)}, // 70 = radius + fudge; 7 = 70*tan(6 degrees); 6 degrees per minute;
    {22,  -(SECOND_TICKER_RADIUS + TICKER_THICKNESS)},
  }
};

static GPath *second_segment_path;


const GPathInfo MINUTE_SEGMENT_PATH_POINTS = {
  3,
  (GPoint []) {
    {0, 0},
    {-5, -(MINUTE_TICKER_RADIUS + TICKER_THICKNESS)}, // 58 = radius + fudge; 6 = 58*tan(6 degrees); 30 degrees per hour;
    {5,  -(MINUTE_TICKER_RADIUS + TICKER_THICKNESS)},
  }
};

static GPath *minute_segment_path;


const GPathInfo HOUR_SEGMENT_PATH_POINTS = {
  3,
  (GPoint []) {
    {0, 0},
    {-4, -(HOUR_TICKER_RADIUS + TICKER_THICKNESS)}, // 48 = radius + fudge; 5 = 48*tan(6 degrees); 6 degrees per second;
    {4,  -(HOUR_TICKER_RADIUS + TICKER_THICKNESS)},
  }
};

static GPath *hour_segment_path;

//Efficient yet lazy way of doing the calculations
//Yes i feel dirty doing this...
//It needs to be: 3-4-4-4-3-4-4-4-3-4-4-4-3-4-4-4
static unsigned int calculateBlocksToDrawCount(int second)
{
    switch(second)
    {
        case 1:
        case 2:
        case 3:
            return 1;
        case 4:
        case 5:
        case 6:
        case 7:
            return 2;
        case 8:
        case 9:
        case 10:
        case 11:
            return 3;
        case 12:
        case 13:
        case 14:
        case 15:
            return 4;
        case 16:
        case 17:
        case 18:
            return 5;
        case 19:
        case 20:
        case 21:
        case 22:
            return 6;
        case 23:
        case 24:
        case 25:
        case 26:
            return 7;
        case 27:
        case 28:
        case 29:
        case 30:
            return 8;
        case 31:
        case 32:
        case 33:
            return 9;
        case 34:
        case 35:
        case 36:
        case 37:
            return 10;
        case 38:
        case 39:
        case 40:
        case 41:
            return 11;
        case 42:
        case 43:
        case 44:
        case 45:
            return 12;
        case 46:
        case 47:
        case 48:
            return 13;
        case 49:
        case 50:
        case 51:
        case 52:
            return 14;
        case 53:
        case 54:
        case 55:
        case 56:
            return 15;
        case 57:
        case 58:
        case 59:
        case 60:
            return 16;
    }
    return 0;
}

//This function is called only when the layer is marked for a redraw (dirty)
static void second_display_layer_update_callback(Layer *layer, GContext* ctx) 
{
    if(SPIN_LOGO == false)
        return;

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
    
    graphics_context_set_fill_color(ctx, BACKGROUND_COLOR);       
    
    unsigned int currentAngle = 0;
    unsigned int blocksToDraw = calculateBlocksToDrawCount(t->tm_sec);
    for(unsigned int itr = 0; itr <= 15; itr++)
    {
        currentAngle += (22 + (itr % 2));
        if(itr >= blocksToDraw)
        {
            gpath_rotate_to(second_segment_path, (TRIG_MAX_ANGLE / 360) * currentAngle);
            gpath_draw_filled(ctx, second_segment_path);
        }
    }
}

//This function is called only when the layer is marked for a redraw (dirty)
static void minute_display_layer_update_callback(Layer *layer, GContext* ctx) 
{
    //Allow switching between showing text and showing the circles.
    //THere isnt enough room for both.
    if(SHOW_DATE_TIME_TEXT)
        return;
    
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  unsigned int angle = t->tm_min * 6;

  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);

  graphics_context_set_fill_color(ctx, FOREGROUND_COLOR);

    //This draws a circle centerd in the center of the watchface that
    //is FOREGROUND_COLOR (white) to create a circle that the outside
    //edge will become the "ticker"
  graphics_fill_circle(ctx, center, MINUTE_TICKER_RADIUS);

    //Setting the fill color here to BACKGROUND_COLOR lets the semi-circle drawing
    //and the full circle drawing below draw in as black effectively erasing them.
  graphics_context_set_fill_color(ctx, BACKGROUND_COLOR);

    //This draws a black semi circle starting at the angle computed above (the time)
    //and then swoops around the hour_segment_path to black out the circle that is
    //drawn in.
  for(; angle < 355; angle += 6) {

    gpath_rotate_to(minute_segment_path, (TRIG_MAX_ANGLE / 360) * angle);

    gpath_draw_filled(ctx, minute_segment_path);

  }

   //This draws a circle centered in the center of the watchface that
    //is BACKGROUND_COLOR (black) to erase the center of the circle that
    //was drawn above and now has the time sliced out of it.  This
    //makes the thickness of the "ticker" into the difference between
    //the radius of this circle and the radius of the previous circle
  graphics_fill_circle(ctx, center, MINUTE_TICKER_RADIUS - TICKER_THICKNESS);

}

//This function is called only when the layer is marked for a redraw (dirty)
static void hour_display_layer_update_callback(Layer *layer, GContext* ctx) 
{
    //Allow switching between showing text and showing the circles.
    //THere isnt enough room for both.
    if(SHOW_DATE_TIME_TEXT)
        return;

  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  unsigned int angle;
     if (clock_is_24h_style())
     {
         //Hour (max 12) times 15 degress per hour.  Add on up to 15 more degress from minutes
      angle = (( t->tm_hour) * 15) + (t->tm_min / 4);
     }
    else
    {
        //Hour (max 12) times 30 degress per hour.  Add on up to 30 more degrees from minutes
      angle = (( t->tm_hour % 12 ) * 30) + (t->tm_min / 2);
    }
  angle = angle - (angle % 6);

  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);

  graphics_context_set_fill_color(ctx, FOREGROUND_COLOR);

    //This draws a circle centerd in the center of the watchface that
    //is FOREGROUND_COLOR (white) to create a circle that the outside
    //edge will become the "ticker"
  graphics_fill_circle(ctx, center, HOUR_TICKER_RADIUS);

    //Setting the fill color here to BACKGROUND_COLOR lets the semi-circle drawing
    //and the full circle drawing below draw in as black effectively erasing them.
  graphics_context_set_fill_color(ctx, BACKGROUND_COLOR);

    //This draws a black semi circle starting at the angle computed above (the time)
    //and then swoops around the hour_segment_path to black out the circle that is
    //drawn in.
  for(; angle < 355; angle += 6) {

    gpath_rotate_to(hour_segment_path, (TRIG_MAX_ANGLE / 360) * angle);

    gpath_draw_filled(ctx, hour_segment_path);
  }
   //This draws a circle centered in the center of the watchface that
    //is BACKGROUND_COLOR (black) to erase the center of the circle that
    //was drawn above and now has the time sliced out of it.  This
    //makes the thickness of the "ticker" into the difference between
    //the radius of this circle and the radius of the previous circle
  graphics_fill_circle(ctx, center, HOUR_TICKER_RADIUS - TICKER_THICKNESS);
}


static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) 
{
  layer_mark_dirty(second_display_layer);
  layer_mark_dirty(minute_display_layer);
  layer_mark_dirty(hour_display_layer);

  if (SHOW_DATE_TIME_TEXT)
  {
	  // Need to be static because it's used by the system later.
	  static char time_text[] = "00:00";
	
	  char *time_format;
	
	  if (clock_is_24h_style()) {
		time_format = "%R";
	  } else {
		time_format = "%I:%M";
	  }
	
	  strftime(time_text, sizeof(time_text), time_format, tick_time);
	
	  text_layer_set_text(text_time_layer, time_text);

	  static char date_text[] = "00 Xxx";
	  
	  strftime(date_text, sizeof(date_text), "%b %d", tick_time);
	  
	  text_layer_set_text(text_date_layer, date_text);
  }
}


//Handle incoming messages from the Pebble app (config settings)
 void in_received_handler(DictionaryIterator *received, void *context) {
	 Tuple *text_tuple = dict_find(received, KEY_SHOW_DATE_TIME_TEXT);
	 Tuple *spin_tuple = dict_find(received, KEY_SPIN_LOGO);
     Tuple *invert_tuple = dict_find(received, KEY_INVERT);
     
     if(text_tuple)
     {
	     SHOW_DATE_TIME_TEXT = text_tuple->value->int8 ? true : false;
	     persist_write_bool(KEY_SHOW_DATE_TIME_TEXT, SHOW_DATE_TIME_TEXT);
         
        layer_set_hidden((Layer*)text_time_layer, !SHOW_DATE_TIME_TEXT);
        layer_set_hidden((Layer*)text_date_layer, !SHOW_DATE_TIME_TEXT);
     }
     if(spin_tuple)
     {
	     SPIN_LOGO = spin_tuple->value->int8 ? false : true; //Flipped so that default is on
	     persist_write_bool(KEY_SPIN_LOGO, SPIN_LOGO);
     }
     if(invert_tuple)
     {
         INVERT = invert_tuple->value->int8 ? true : false;
         persist_write_bool(KEY_INVERT, INVERT);
         
        layer_set_hidden((Layer*)image_inverter_layer, INVERT);
     }
     
	 window_set_background_color(window, BACKGROUND_COLOR);
 }


 void in_dropped_handler(AppMessageResult reason, void *context) {
   // incoming message dropped
 }
///////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////LUTRON IMAGE HANDLING/////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  image = gbitmap_create_with_resource(RESOURCE_ID_LUTRON_LOGO_WHITE);
  window_set_background_color(window, BACKGROUND_COLOR);
    
  const GPoint center = grect_center_point(&bounds);
  GRect image_frame = (GRect) { .origin = center, .size = image->bounds.size };
  image_frame.origin.x -= image->bounds.size.w/2;
  image_frame.origin.y -= image->bounds.size.h/2;

  // Use GCompOpOr to display the white portions of the image
  image_layer = bitmap_layer_create(image_frame);
  bitmap_layer_set_bitmap(image_layer, image);
  bitmap_layer_set_compositing_mode(image_layer, GCompOpOr);
}

static void window_unload(Window *window) {
  bitmap_layer_destroy(image_layer);
  gbitmap_destroy(image);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

static void init(void) {
	
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_open(app_message_inbox_size_maximum(), 0);

  if(persist_exists(KEY_SHOW_DATE_TIME_TEXT)) SHOW_DATE_TIME_TEXT = persist_read_bool(KEY_SHOW_DATE_TIME_TEXT);
  if(persist_exists(KEY_SPIN_LOGO)) SPIN_LOGO = persist_read_bool(KEY_SPIN_LOGO);
  if(persist_exists(KEY_INVERT)) INVERT = persist_read_bool(KEY_INVERT);

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  window_stack_push(window, true);
	
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Init the layer for the second display
  second_display_layer = layer_create(bounds);
  layer_set_update_proc(second_display_layer, second_display_layer_update_callback);

  // Init the second segment path
  second_segment_path = gpath_create(&SECOND_SEGMENT_PATH_POINTS);
  gpath_move_to(second_segment_path, grect_center_point(&bounds));
	
  // Init the layer for the minute display
  minute_display_layer = layer_create(bounds);
  layer_set_update_proc(minute_display_layer, minute_display_layer_update_callback);

  // Init the minute segment path
  minute_segment_path = gpath_create(&MINUTE_SEGMENT_PATH_POINTS);
  gpath_move_to(minute_segment_path, grect_center_point(&bounds));

  // Init the layer for the hour display
  hour_display_layer = layer_create(bounds);
  layer_set_update_proc(hour_display_layer, hour_display_layer_update_callback);

  // Init the hour segment path
  hour_segment_path = gpath_create(&HOUR_SEGMENT_PATH_POINTS);
  gpath_move_to(hour_segment_path, grect_center_point(&bounds));

  // Init the inverter layer
    image_inverter_layer = inverter_layer_create(bounds);
    layer_set_hidden((Layer*)image_inverter_layer, true);//INVERT);
    
    //Init the Time Text Layers
    text_time_layer = text_layer_create(bounds);
    text_layer_set_text_color(text_time_layer, FOREGROUND_COLOR);
    text_layer_set_background_color(text_time_layer, GColorClear);
    text_layer_set_text_alignment(text_time_layer, GTextAlignmentCenter);
    text_layer_set_font(text_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
    layer_set_frame((Layer*)text_time_layer, GRect(0, 57, 144, 168-57));    
    layer_set_hidden((Layer*)text_time_layer, !SHOW_DATE_TIME_TEXT);
    
    //Init the Date Text Layer
    text_date_layer = text_layer_create(bounds);
    text_layer_set_text_color(text_date_layer, FOREGROUND_COLOR);
    text_layer_set_background_color(text_date_layer, GColorClear);
    text_layer_set_text_alignment(text_date_layer, GTextAlignmentCenter);
    text_layer_set_font(text_date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
    layer_set_frame((Layer*)text_date_layer, GRect(0, 80, 144, 168-80));
    layer_set_hidden((Layer*)text_date_layer, !SHOW_DATE_TIME_TEXT);
    
    //Add the layers!!!
    //The ordering here detemines which order that the layers will be displayed.
    //First in, first displayed, placed under everything else.
    layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));
    layer_add_child(window_layer, second_display_layer);
    layer_add_child(window_layer, minute_display_layer);
    layer_add_child(window_layer, hour_display_layer);
    layer_add_child(window_layer, text_layer_get_layer(text_time_layer));
    layer_add_child(window_layer, text_layer_get_layer(text_date_layer));
    layer_add_child(window_layer, (Layer*)image_inverter_layer);
    

  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
}

static void deinit(void) {
    gpath_destroy(second_segment_path);
    gpath_destroy(minute_segment_path);
    gpath_destroy(hour_segment_path);
    
    tick_timer_service_unsubscribe();
    window_destroy(window);
    layer_destroy(minute_display_layer);
    layer_destroy(hour_display_layer);
    layer_destroy(second_display_layer);
    text_layer_destroy(text_date_layer);
    text_layer_destroy(text_time_layer);
    inverter_layer_destroy(image_inverter_layer);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
