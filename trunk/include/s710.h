/* $Id: s710.h,v 1.19 2007/05/09 17:27:54 dave Exp $ */

#ifndef __S710_H__
#define __S710_H__

#include <stdio.h>
#include <limits.h>
#include <stdarg.h>
#include <time.h>
#include <gd.h>

/* constants */

#define S710_SPEED_KPH       "kph"
#define S710_SPEED_MPH       "mph"
#define S710_DISTANCE_KM     "km"
#define S710_DISTANCE_MI     "mi"
#define S710_ALTITUDE_M      "m"
#define S710_ALTITUDE_FT     "ft"
#define S710_TEMPERATURE_C   "C"
#define S710_TEMPERATURE_F   "F"

#define S710_REQUEST         0xa3
#define S710_RESPONSE        0x5c

#define S710_USB_VENDOR_ID   0x0da4
#define S710_USB_PRODUCT_ID  1
#define S710_USB_INTERFACE   0

#define S710_MODE_HEART_RATE  0
#define S710_MODE_ALTITUDE    2
#define S710_MODE_CADENCE     4
#define S710_MODE_POWER       8
#define S710_MODE_BIKE1      16
#define S710_MODE_BIKE2      32
#define S710_MODE_SPEED      (S710_MODE_BIKE1 | S710_MODE_BIKE2)

#define S710_AM_PM_MODE_UNSET 0
#define S710_AM_PM_MODE_SET   1
#define S710_AM_PM_MODE_PM    2

#define S710_WORKOUT_HEADER   1
#define S710_WORKOUT_LAPS     2
#define S710_WORKOUT_SAMPLES  4
#define S710_WORKOUT_FULL     7

#define S710_TIC_PLAIN        0
#define S710_TIC_LINES        1
#define S710_TIC_SHADE        2
#define S710_TIC_SHADE_RED    (S710_TIC_SHADE | 4)
#define S710_TIC_SHADE_GREEN  (S710_TIC_SHADE | 8)
#define S710_TIC_SHADE_BLUE   (S710_TIC_SHADE | 16)

#define S710_Y_AXIS_LEFT        -1
#define S710_Y_AXIS_RIGHT        1

#define S710_X_MARGIN         15
#define S710_Y_MARGIN         15

#define S710_BLANK_SAMPLE_LIMIT  2880 /* 4 hours at 5 second intervals. */

#define S710_HEADER_SIZE_S625X 130
#define S710_HEADER_SIZE_S710  109
#define S710_HEADER_SIZE_S610   78


typedef enum {
  S710_DRIVER_SERIAL,
  S710_DRIVER_IR,
  S710_DRIVER_USB
} S710_Driver_Type;

typedef enum {
  S710_X_AXIS_DISTANCE,
  S710_X_AXIS_TIME
} S710_X_Axis;

typedef enum { 
  S710_Y_AXIS_HEART_RATE,
  S710_Y_AXIS_ALTITUDE,
  S710_Y_AXIS_SPEED,
  S710_Y_AXIS_CADENCE,
  S710_Y_AXIS_POWER
} S710_Y_Axis;

typedef enum {
  S710_FILTER_OFF,
  S710_FILTER_ON
} S710_Filter;

typedef enum { 
  S710_MODE_RDWR,
  S710_MODE_RDONLY
} S710_Mode;

typedef enum {
  S710_GET_OVERVIEW,
  S710_GET_USER,
  S710_GET_WATCH,
  S710_GET_LOGO,
  S710_GET_BIKE,
  S710_GET_EXERCISE_1,
  S710_GET_EXERCISE_2,
  S710_GET_EXERCISE_3,
  S710_GET_EXERCISE_4,
  S710_GET_EXERCISE_5,
  S710_GET_REMINDER_1,
  S710_GET_REMINDER_2,
  S710_GET_REMINDER_3,
  S710_GET_REMINDER_4,
  S710_GET_REMINDER_5,
  S710_GET_REMINDER_6,
  S710_GET_REMINDER_7,
  S710_GET_FILES,
  S710_CONTINUE_TRANSFER,
  S710_CLOSE_CONNECTION,
  S710_SET_USER,
  S710_SET_WATCH,
  S710_SET_LOGO,
  S710_SET_BIKE,
  S710_SET_EXERCISE_1,
  S710_SET_EXERCISE_2,
  S710_SET_EXERCISE_3,
  S710_SET_EXERCISE_4,
  S710_SET_EXERCISE_5,
  S710_SET_REMINDER_1,
  S710_SET_REMINDER_2,
  S710_SET_REMINDER_3,
  S710_SET_REMINDER_4,
  S710_SET_REMINDER_5,
  S710_SET_REMINDER_6,
  S710_SET_REMINDER_7,
  S710_HARD_RESET
} S710_Packet_Index;

typedef enum {
  S710_OFF,
  S710_ON
} S710_On_Off;

typedef enum {
  S710_HOURS_24,
  S710_HOURS_12
} S710_Hours;

typedef enum {
  S710_BIKE_NONE,
  S710_BIKE_1,
  S710_BIKE_2
} S710_Bike;

typedef enum {
  S710_EXE_NONE,
  S710_EXE_BASIC_USE,
  S710_EXE_SET_1,
  S710_EXE_SET_2,
  S710_EXE_SET_3,
  S710_EXE_SET_4,
  S710_EXE_SET_5
} S710_Exercise;

typedef enum {
  S710_REPEAT_OFF,
  S710_REPEAT_HOURLY,
  S710_REPEAT_DAILY,
  S710_REPEAT_WEEKLY,
  S710_REPEAT_MONTHLY,
  S710_REPEAT_YEARLY
} S710_Repeat;

typedef enum {
  S710_UNITS_METRIC,
  S710_UNITS_ENGLISH
} S710_Units;

typedef enum {
  S710_HT_SHOW_LIMITS,
  S710_HT_STORE_LAP,
  S710_HT_SWITCH_DISP
} S710_Heart_Touch;

typedef enum {
  S710_RECORD_INT_05,
  S710_RECORD_INT_15,
  S710_RECORD_INT_60
} S710_Recording_Interval;

typedef enum {
  S710_INTERVAL_MODE_OFF = 0,
  S710_INTERVAL_MODE_ON  = 1
} S710_Interval_Mode;

typedef enum {
  S710_ACTIVITY_LOW,
  S710_ACTIVITY_MEDIUM,
  S710_ACTIVITY_HIGH,
  S710_ACTIVITY_TOP
} S710_Activity_Level;

typedef enum { 
  S710_GENDER_MALE,
  S710_GENDER_FEMALE
} S710_Gender;

typedef enum {
  S710_ATTRIBUTE_TYPE_INTEGER,
  S710_ATTRIBUTE_TYPE_STRING,
  S710_ATTRIBUTE_TYPE_BYTE,
  S710_ATTRIBUTE_TYPE_BOOLEAN,
  S710_ATTRIBUTE_TYPE_ENUM_INTEGER,
  S710_ATTRIBUTE_TYPE_ENUM_STRING
} S710_Attribute_Type;

typedef enum {
  S710_MAP_TYPE_USER,
  S710_MAP_TYPE_WATCH,
  S710_MAP_TYPE_LOGO,
  S710_MAP_TYPE_BIKE,
  S710_MAP_TYPE_EXERCISE,
  S710_MAP_TYPE_REMINDER,
  S710_MAP_TYPE_COUNT
} S710_Map_Type;

typedef enum {
  S710_HRM_AUTO    =  0,
  S710_HRM_S610    = 11,  /* same as in hrm files */
  S710_HRM_S710    = 12,  /* same as in hrm files */
  S710_HRM_S810    = 13,  /* same as in hrm files */
  S710_HRM_S625X   = 22,  /* same as in hrm files */
  S710_HRM_UNKNOWN = 255
} S710_HRM_Type;

typedef enum {
  S710_MERGE_TRUE,
  S710_MERGE_CONCAT
} S710_Merge_Type;


/* helpful macros */

#define UNIB(x)      ((x)>>4)
#define LNIB(x)      ((x)&0x0f)
#define BINU(x)      ((x)<<4)
#define BINL(x)      ((x)&0x0f)
#define BCD(x)       (UNIB(x)*10 + LNIB(x))
#define HEX(x)       ((((x)/10)<<4) + ((x)%10))
#define DIGITS01(x)  ((x)%100)
#define DIGITS23(x)  (((x)/100)%100)
#define DIGITS45(x)  (((x)/10000)%100)

#define CLAMP(a,b,c) if((a)<(b))(a)=(b);if((a)>(c))(a)=(c)


/* structure and type definitions */

typedef struct S710_Driver {
  S710_Driver_Type  type;
  S710_Mode         mode;
  char              path[PATH_MAX];
  void             *data;
} S710_Driver;


typedef char           S710_Label[8];
typedef struct tm      S710_Date;

typedef struct S710_Time {
  int    hours;
  int    minutes;
  int    seconds;
  int    tenths;
} S710_Time;

/* basic types */

typedef unsigned char  S710_Heart_Rate;
typedef unsigned char  S710_Cadence;
typedef short          S710_Altitude;
typedef unsigned short S710_Speed;     /* 1/16ths of a km/hr */
typedef float          S710_Distance;

typedef struct S710_Power {
  unsigned short power;
  unsigned char  lr_balance;
  unsigned char  pedal_index;
} S710_Power;

typedef char           S710_Temperature;

typedef struct S710_HR_Limit {
  S710_Heart_Rate  lower;
  S710_Heart_Rate  upper;
} S710_HR_Limit;

/* basic communication packet */

typedef char          *PacketName;
typedef unsigned char  PacketType;
typedef unsigned char  PacketID;
typedef unsigned short PacketLength;
typedef unsigned short PacketChecksum;
typedef unsigned char  PacketData[1];

typedef struct packet_t {
  PacketName     name;
  PacketType     type;
  PacketID       id;
  PacketLength   length;
  PacketChecksum checksum;
  PacketData     data;
} packet_t;

/* overview data */

typedef struct overview_t {
  int files;
  int bytes;
} overview_t;

/* user data */

typedef struct user_t {
  S710_On_Off              altimeter;
  S710_On_Off              fitness_test;
  S710_On_Off              predict_hr_max;
  S710_On_Off              energy_expenditure;
  S710_On_Off              options_lock;
  S710_On_Off              help;
  S710_On_Off              activity_button_sound;
  S710_Units               units;
  S710_Heart_Touch         heart_touch;
  S710_Recording_Interval  recording_interval;
  S710_Activity_Level      activity_level;
  S710_Gender              gender;
  unsigned int             weight;
  unsigned int             height;
  unsigned int             vo2max;
  S710_Heart_Rate          max_hr;
  unsigned int             user_id;
  S710_Label               name;
  S710_Date                birth_date;
  unsigned char            unknown[2];
} user_t;

/* watch data */

typedef struct watch_t {
  S710_Date               time1;
  S710_Time               time2;
  S710_Time               alarm;
  S710_On_Off             alarm_on;
  S710_Hours              time1_hours;
  S710_Hours              time2_hours;
  int                     time_zone;
} watch_t;

/* logo data */

typedef struct logo_t {
  unsigned char column[47];
} logo_t;

/* bike data */

typedef struct bike_data_t {
  unsigned int            wheel_size;
  S710_Label              label;
  S710_On_Off             power_sensor;
  S710_On_Off             cadence_sensor;
  unsigned int            chain_weight;
  unsigned int            chain_length;
  unsigned int            span_length;
} bike_data_t;

typedef struct bike_t {
  S710_Bike               in_use;
  bike_data_t             bike[2];
} bike_t;

/* exercise data */

typedef struct exercise_t {
  unsigned int            which;
  S710_Label              label;
  S710_Time               timer[3];
  S710_HR_Limit           hr_limit[3];
  S710_Time               recovery_time;
  S710_Heart_Rate         recovery_hr;
} exercise_t;

/* reminder data */

typedef struct reminder_t {
  unsigned int            which;
  S710_Label              label;
  S710_Date               date;
  S710_On_Off             on;
  S710_Exercise           exercise;
  S710_Repeat             repeat;
} reminder_t;

/* workout data */

typedef struct files_t {
  unsigned short          bytes;
  unsigned short          cursor;
  unsigned char           data[32768];
} files_t;


/* lap data */

typedef struct lap_data_t {
  S710_Time        split;
  S710_Time        cumulative;
  S710_Heart_Rate  lap_hr;
  S710_Heart_Rate  avg_hr;
  S710_Heart_Rate  max_hr;
  S710_Altitude    alt;
  S710_Altitude    ascent;
  S710_Altitude    cumul_ascent;
  S710_Temperature temp;
  S710_Cadence     cad;
  int              distance;
  int              cumul_distance;
  S710_Speed       speed;
  S710_Power       power;
} lap_data_t;


/* units info */

typedef struct units_data_t {
  S710_Units       system;       /* S710_UNITS_METRIC or S710_UNITS_ENGLISH */
  char            *altitude;     /* "m" or "ft" */
  char            *speed;        /* "kph" or "mph" */
  char            *distance;     /* "km" or "mi" */
  char            *temperature;  /* "C" or "F" */
} units_data_t;


/* a single workout */

typedef struct workout_t {
  S710_HRM_Type           type;
  S710_Date               date;
  int                     ampm;
  time_t                  unixtime;
  int                     user_id;
  S710_Interval_Mode      interval_mode;
  int                     exercise_number;
  S710_Label              exercise_label;
  S710_Time               duration;
  S710_Heart_Rate         avg_hr;
  S710_Heart_Rate         max_hr;
  int                     bytes;
  int                     laps;
  int                     manual_laps;
  int                     samples;
  units_data_t            units;
  int                     mode;
  int                     recording_interval;
  int                     filtered;
  S710_HR_Limit           hr_limit[3];
  S710_Time               hr_zone[3][3];
  S710_Time               bestlap_split;
  S710_Time               cumulative_exercise;
  S710_Time               cumulative_ride;
  int                     odometer;
  int                     exercise_distance;
  S710_Cadence            avg_cad;
  S710_Cadence            max_cad;
  S710_Altitude           min_alt;
  S710_Altitude           avg_alt;
  S710_Altitude           max_alt;
  S710_Temperature        min_temp;
  S710_Temperature        avg_temp;
  S710_Temperature        max_temp;
  S710_Altitude           ascent;
  S710_Power              avg_power;
  S710_Power              max_power;
  S710_Speed              avg_speed;
  S710_Speed              max_speed;
  S710_Speed              median_speed;
  S710_Speed		  highest_speed;
  int                     energy;
  int                     total_energy;
  lap_data_t             *lap_data;
  S710_Heart_Rate        *hr_data;
  S710_Altitude          *alt_data;
  S710_Speed             *speed_data;
  S710_Distance          *dist_data;       /* computed from speed_data */
  S710_Cadence           *cad_data;
  S710_Power             *power_data;
} workout_t;


/* A few macros that are useful */

#define S710_HAS_FIELD(x,y)   (((x) & S710_MODE_##y) != 0)

#define S710_HAS_CADENCE(x)   S710_HAS_FIELD(x,CADENCE)
#define S710_HAS_POWER(x)     S710_HAS_FIELD(x,POWER)
#define S710_HAS_SPEED(x)     S710_HAS_FIELD(x,SPEED)
#define S710_HAS_ALTITUDE(x)  S710_HAS_FIELD(x,ALTITUDE)
#define S710_HAS_BIKE1(x)     S710_HAS_FIELD(x,BIKE1)


/* structs used in rendering images using GD */

typedef struct byte_zone_t {
  int min;
  int max;
  struct {
    int         red;
    int         green;
    int         blue;
    int         pixel;
  } color;
  int seconds;   /* spent in this zone (HR or cadence) */
} byte_zone_t;

typedef struct byte_histogram_t {
  int           zones;
  byte_zone_t  *zone;
  int           hist[0x10000];  /* histogram of seconds at each byte value */
} byte_histogram_t;

/* structs and unions used when setting watch properties */

typedef union attribute_value_t {
  int                 int_value;  
  S710_Label          string_value;
  unsigned char       byte_value;
  S710_On_Off         bool_value;
  struct {
    int               eval;
    int               ival;
  } enum_int_value;
  struct {
    char             *sval;
    int               ival;
  } enum_string_value;
} attribute_value_t;

/* 
   if the type is S710_ATTRIBUTE_TYPE_INTEGER, then values is a three-element 
   array with the first element being the lower bound and the second element 
   the upper bound for the value, and the third element the offset (amount to
   add to the value before storing it).
   
   if the type is S710_ATTRIBUTE_TYPE_STRING, then values is a single-element 
   array with the only element being the (integer) maximum string length.

   if the type is S710_ATTRIBUTE_TYPE_BOOLEAN or S710_ATTRIBUTE_TYPE_BYTE,
   then values is NULL.
   
   if the type is S710_ATTRIBUTE_TYPE_ENUM_INTEGER, then values is a 
   negative-terminated list of allowed (non-negative) integer values.

   if the type is S710_ATTRIBUTE_TYPE_ENUM_STRING, then values is a 
   NULL-terminated list of pairs of allowed string values and the integer
   values they correspond to.
*/

typedef struct attribute_pair_t {
  char                    *name;
  S710_Attribute_Type      type;
  void                    *ptr;  
  attribute_value_t       *values;
  int                      vcount;  
  attribute_value_t        value;
  int                      oosync;
  struct attribute_pair_t *next;
} attribute_pair_t;

typedef struct attribute_map_t {
  union { 
    user_t     user;
    watch_t    watch;
    bike_t     bike;
    logo_t     logo;
    exercise_t exercise;
    reminder_t reminder;
  } data;    
  S710_Map_Type           type;
  attribute_pair_t       *pairs;
  int                     oosync;
} attribute_map_t;


/* externs */

extern unsigned char gByteMap[256];


/* function prototypes */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

  /* driver.c */

  int       driver_init  ( int argc, char **argv, S710_Driver *d );
  int       driver_open  ( S710_Driver *d, S710_Mode mode );
  int       driver_close ( S710_Driver *d );

  /* byte_map.c */

  void      compute_byte_map ( void );

  /* serial.c */

  int       init_serial_port ( S710_Driver *d, S710_Mode mode );
  int       read_serial_byte ( S710_Driver *d, unsigned char *byte );

  /* usb.c */

  int       init_usb_port     ( S710_Driver *d, S710_Mode mode);
  int       read_usb_byte     ( S710_Driver *d, unsigned char *byte );
  int       send_packet_usb   ( unsigned char *serialized,
				int            bytes, 
				S710_Driver   *d );
  int       shutdown_usb_port ( S710_Driver *d );

  /* crc.c */

  void crc_process ( unsigned short      * context, 
		     unsigned char         ch );
  void crc_block   ( unsigned short      * context,
		     const unsigned char * blk,
		     int                   len );

  /* alpha_map.c */

  char          alpha_map         ( unsigned char  c );
  unsigned char inverse_alpha_map ( char c );
  void          extract_label     ( unsigned char *buf, 
				    S710_Label    *label, 
				    int            bytes );
  void          encode_label      ( S710_Label     label,
				    unsigned char *buf,
				    int            bytes );

  /* comm.c */

  int       send_packet      ( packet_t *packet, S710_Driver *d );
  packet_t *recv_packet      ( S710_Driver *d );

  /* overview.c */

  int       get_overview     ( S710_Driver *d, overview_t *overview );
  void      print_overview   ( overview_t *o, FILE *fp );

  /* user.c */

  int       get_user          ( S710_Driver *d, user_t *user );
  int       set_user          ( user_t *user, S710_Driver *d );
  void      set_user_name     ( user_t *u, char *name );
  void      set_user_birthday ( user_t *u, int day, int month, int year );
  void      print_user        ( user_t *u, FILE *fp );
  void      load_user_attributes ( attribute_map_t *map );

  /* watch.c */

  int       get_watch        ( S710_Driver *d, watch_t *watch );
  int       set_watch        ( watch_t *watch, S710_Driver *d );
  void      print_watch      ( watch_t *w, FILE *fp );
  void      load_watch_attributes ( attribute_map_t *map );

  /* logo.c */

  int       get_logo         ( S710_Driver *d, logo_t *logo );
  int       set_logo         ( logo_t *logo, S710_Driver *d );
  void      print_logo       ( logo_t *l, FILE *fp );
  void      load_logo_attributes ( attribute_map_t *map );

  /* bike.c */

  int       get_bike         ( S710_Driver *d, bike_t *bike );
  int       set_bike         ( bike_t *bike, S710_Driver *d );
  void      print_bike       ( bike_t *b, FILE *fp );
  void      load_bike_attributes ( attribute_map_t *map );

  /* exercise.c */

  int       get_exercise     ( S710_Driver        *d, 
			       S710_Packet_Index   which, 
			       exercise_t         *exercise );
  int       set_exercise     ( exercise_t         *exercise,
			       S710_Packet_Index   which,
			       S710_Driver        *d );
  void      print_exercise   ( exercise_t         *e, 
			       FILE               *fp );
  void      load_exercise_attributes ( attribute_map_t *map );

  /* reminder.c */

  int       get_reminder     ( S710_Driver        *d, 
			       S710_Packet_Index   which, 
			       reminder_t         *reminder );
  int       set_reminder     ( reminder_t         *reminder,
			       S710_Packet_Index   which,
			       S710_Driver        *d );
  void      print_reminder   ( reminder_t         *r, 
			       FILE               *fp );
  void      load_reminder_attributes ( attribute_map_t *map );

  /* files.c */

  int       get_files        ( S710_Driver *d, files_t *files, FILE *fp );
  int       receive_file     ( S710_Driver *d, files_t *file, FILE *fp );
  int       save_files       ( files_t *f, char *dir, FILE *fp );
  void      print_files      ( files_t *f, FILE *fp );
  time_t    file_timestamp   ( unsigned char *data );

  /* packet.c */
  
  int       num_packets      ( void );
  packet_t *packet           ( S710_Packet_Index index );
  packet_t *make_set_packet  ( S710_Packet_Index index );
  int       send_set_packet  ( packet_t *packet, S710_Driver *d );
  packet_t *get_response     ( S710_Packet_Index request, S710_Driver *d );
  void      close_connection ( S710_Driver *d );
  void      print_packet     ( packet_t *packet, FILE *fp );

  /* time.c */

  void   print_s710_time          ( S710_Time *  t, 
				    char *       format, 
				    FILE *       fp );

  time_t s710_time_to_tenths      ( S710_Time *  t );
  time_t s710_time_to_seconds     ( S710_Time *  t );
  
  void   diff_s710_time           ( S710_Time *  t1, 
				    S710_Time *  t2, 
				    S710_Time *  diff );
  void   sum_s710_time            ( S710_Time *  t1, 
				    S710_Time *  t2, 
				    S710_Time *  sum );
  void   increment_s710_time      ( S710_Time *  t, 
				    unsigned int seconds );
  float  get_hours_from_s710_time ( S710_Time *  t );

  /* workout_util.c */

  int  header_size                ( workout_t * w );

  int  bytes_per_lap              ( S710_HRM_Type type, 
				    unsigned char bt, 
				    unsigned char bi );

  int  bytes_per_sample           ( unsigned char bt );

  int  allocate_sample_space      ( workout_t * w );

  void free_workout               ( workout_t * w );
  
  /* workout_merge.c */

  workout_t * merge_workouts       ( workout_t *     w1, 
				     workout_t *     w2,
				     S710_Merge_Type type );

  /* workout_print.c */

  void        print_workout        ( workout_t * w, FILE * fp, int what );
  void        print_workout_as_hrm ( workout_t * w, FILE * fp );


  /* workout_read.c */

  workout_t * read_workout         ( char *          filename, 
				     S710_Filter     filter, 
				     S710_HRM_Type   type );

  /* workout_write.c */

  int         write_workout        ( workout_t * w, char * filename );

  /* filter.c */

  void filter_workout ( workout_t * w );

  /* axes.c */

  void   draw_x_axis ( gdImagePtr    im, 
		       int           t_or_d,
		       units_data_t *units,
		       float         max_x,  
		       int           xi,     
		       int           yi,     
		       int           xf,     
		       int           yf,     
		       int           pixel );

  void   draw_y_axis( gdImagePtr  im,
		      int         side,
		      char       *units,
		      float       ymin,
		      float       ymax,
		      int         r,
		      int         g,
		      int         b,
		      int         tic_opts,
		      int         xi,
		      int         yi,
		      int         xf,
		      int         yf,
		      int         pixel );

  /* plot.c */

  int    plot_workout_xy ( workout_t *w, 
			   int        y_axis, 
			   int        x_axis, 
			   int        r,
			   int        g,
			   int        b,
			   int        tic_opts,
			   int        width,
			   int        height,
			   char      *filename );

  /* histogram.c */
       
  byte_histogram_t *make_byte_histogram ( workout_t   *w, 
					  byte_zone_t *z, 
					  int          nz, 
					  int          what );
  void              free_byte_histogram ( byte_histogram_t *h );
  int               plot_byte_histogram ( byte_histogram_t *h,
					  int               r,
					  int               g,
					  int               b,
					  int               min,
					  int               max,
					  int               width,
					  int               height,
					  int               dither,
					  char             *filename );
  int               plot_byte_zones ( byte_histogram_t *h,
				      int               width,
				      int               height,
				      char             *filename );

  /* glob.c */
  
  int  is_like ( char *given, char *wanted );

  /* transfer.c */

  void handle_retrieval ( S710_Driver      *d, 
			  int               r, 
			  char             *filedir );

  void handle_transfer  ( S710_Driver      *d, 
			  int               r, 
			  char             *request,
			  attribute_map_t  *map );

  /* add_attr.c */
  
  void add_string_attribute  ( attribute_map_t *m, 
			       char            *name,
			       void            *ptr,
			       int              length );
  
  void add_integer_attribute ( attribute_map_t *m, 
			       char            *name, 
			       void            *ptr,
			       int              min, 
			       int              max,
			       int              offset );
  
  void add_boolean_attribute ( attribute_map_t *m,
			       char            *name,
			       void            *ptr );
  
  void add_byte_attribute ( attribute_map_t *m,
			    char            *name,
			    void            *ptr );
  
  void add_enum_integer_attribute ( attribute_map_t *m, 
				    char            *name, 
				    void            *ptr,
				    ... );
  
  void add_enum_string_attribute ( attribute_map_t *m, 
				   char            *name, 
				   void            *ptr,
				   ... );
  
  attribute_pair_t *add_attribute ( attribute_pair_t    **p, 
				    S710_Attribute_Type   type, 
				    char                 *name,
				    void                 *ptr );

  /* merge_attr.c */

  void merge_attribute_map                 ( attribute_map_t *map );
  
  void sync_attribute_map                  ( attribute_map_t *map );

  int  merge_attribute_value               ( char *value, 
					     attribute_pair_t *p );
  int  merge_integer_attribute_value       ( char *value, 
					     attribute_pair_t *p );
  int  merge_string_attribute_value        ( char *value, 
					     attribute_pair_t *p );
  int  merge_boolean_attribute_value       ( char *value, 
					     attribute_pair_t *p );
  int  merge_byte_attribute_value          ( char *value, 
					     attribute_pair_t *p );
  int  merge_enum_integer_attribute_value  ( char *value, 
					    attribute_pair_t *p );
  int  merge_enum_string_attribute_value   ( char *value, 
					     attribute_pair_t *p );

  /* print_attr.c */

  void print_attribute_map ( int c, attribute_map_t *m );

  void print_current_attribute_value ( attribute_pair_t *p, FILE *fp );

  /* free_attr.c */

  void clear_attribute_map     ( attribute_map_t   *map );
  void free_attribute_pairs    ( attribute_pair_t  *pairs );

  /* parse_attr.c */

  void parse_attribute_pairs ( char *s, attribute_map_t *map );
  
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __S710_H__ */
