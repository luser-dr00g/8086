
FILE *handles[20];
U next_handle = 5;

US *equipment_list;
UC *pcjr;
US *usable_memory;

US *keyboard_status;
UC *future_use;
US *keyboard_buffer_start;
US *keyboard_buffer_tail;
US *keyboard_buffer;

UC *drive_recalibration;
UC *drive_moter_running;
UC *drive_moter_countdown;
UC *drive_error;
UC *disk_controller_status;

UC *video_mode;
US *screen_columns;
US *bytes_per_screen_page;
US *screen_location_offset;
US *cursor_for_page;
US *cursor_size;
UC *current_screen_page;
US *port_addr_vid_chip;
UC *crt_mode;
UC *color_palette_mask;

US *cassette_tape_control;
US *master_clock_count;
UC *master_clock_rollover;

UC *keyboard_break_action;
US *keyboard_reboot_in_progress;

UC *pcjr_special_disk_control;
UC *pcjr_serial_timeout;
UC *pcjr_repeat_key_char;
UC *pcjr_repeat_key_delay;
UC *pcjr_current_function_code;
UC *pcjr_extra_keyboard_status;
UC *print_screen_status;
UC *single_disk_mimicking_double;

US *basic_default_ds;
US *basic_clocktick_isr;
US *basic_break_key_isr;
US *basic_disk_error_isr;
UC *intra_app_communication_area;

UC *rom_release_date;
UC *machine_id;

static int load_dos( char *mem ){
  handles[0] = stdin;
  handles[1] = stdout;
  handles[2] = stderr;

  { US x[] = { 0 };     memcpy( equipment_list = mem + 0x410, x, sizeof x ); }
  { UC x[] = { 0 };     memcpy( pcjr = mem + 0x412, x, sizeof x ); }
  { US x[] = { 0 };     memcpy( usable_memory = mem + 0x413, x, sizeof x ); }

  { US x[] = { 0 };     memcpy( keyboard_status = mem + 0x417, x, sizeof x ); }
  { UC x[] = { 0 };     memcpy( future_use = mem + 0x419, x, sizeof x ); }
  { US x[] = { 0x41e }; memcpy( keyboard_buffer_start = mem + 0x41a, x, sizeof x ); }
  { US x[] = { 0x41e }; memcpy( keyboard_buffer_tail = mem + 0x41c, x, sizeof x ); }
  { US x[16] = { 0 };   memcpy( keyboard_buffer = mem + 0x41e, x, sizeof x ); }

  { UC x[] = { 0xf };   memcpy( drive_recalibration = mem + 0x43e, x, sizeof x ); }
  { UC x[] = { 0 };     memcpy( drive_moter_running = mem + 0x43f, x, sizeof x ); }
  { UC x[] = { 0 };     memcpy( drive_moter_countdown = mem + 0x440, x, sizeof x ); }
  { UC x[] = { 0 };     memcpy( drive_error = mem + 0x441, x, sizeof x ); }
  { UC x[7] = { 0 };    memcpy( disk_controller_status = mem + 0x442, x, sizeof x ); }

  { UC x[] = { 0 };     memcpy( video_mode = mem + 0x449, x, sizeof x ); }
  { US x[] = { 0 };     memcpy( screen_columns = mem + 0x44a, x, sizeof x ); }
  { US x[] = { 0 };     memcpy( bytes_per_screen_page = mem + 0x44c, x, sizeof x ); }
  { US x[] = { 0 };     memcpy( screen_location_offset = mem + 0x44e, x, sizeof x ); }
 
  { US x[8] = { 0 };    memcpy( cursor_for_page = mem + 0x450, x, sizeof x ); }
  { US x[] = { 0 };     memcpy( cursor_size = mem + 0x460, x, sizeof x ); }
  { UC x[] = { 0 };     memcpy( current_screen_page = mem + 0x462, x, sizeof x ); }
  { US x[] = { 0x3d4 }; memcpy( port_addr_vid_chip = mem + 0x463, x, sizeof x ); }
  { UC x[] = { 0 };     memcpy( crt_mode = mem + 0x465, x, sizeof x ); }
  { UC x[] = { 0 };     memcpy( color_palette_mask = mem + 0x466, x, sizeof x ); }

  { US x[5] = { 0 };   memcpy( cassette_tape_control = mem + 0x467, x, sizeof x ); }
  { US x[2] = { 0 };   memcpy( master_clock_count = mem + 0x46c, x, sizeof x ); }
  { UC x[] = { 0 };    memcpy( master_clock_rollover = mem + 0x470, x, sizeof x ); }

  { UC x[] = { 0 };    memcpy( keyboard_break_action = mem + 0x471, x, sizeof x ); }
  { US x[] = { 0 };    memcpy( keyboard_reboot_in_progress = mem + 0x472, x, sizeof x ); }

  { UC x[4] = { 0 };   memcpy( pcjr_special_disk_control = mem + 0x474, x, sizeof x ); }
  { US x[4] = { 0 };   memcpy( pcjr_serial_timeout = mem + 0x478, x, sizeof x ); }
  { UC x[] = { 0 };    memcpy( pcjr_repeat_key_char = mem + 0x485, x, sizeof x ); }
  { UC x[] = { 0 };    memcpy( pcjr_repeat_key_delay = mem + 0x486, x, sizeof x ); }
  { UC x[] = { 0 };    memcpy( pcjr_current_function_code = mem + 0x487, x, sizeof x ); }
  { UC x[] = { 0 };    memcpy( pcjr_extra_keyboard_status = mem + 0x488, x, sizeof x ); }
  { UC x[] = { 0 };    memcpy( print_screen_status = mem + 0x500, x, sizeof x ); }
  { UC x[] = { 0 };    memcpy( single_disk_mimicking_double = mem + 0x504, x, sizeof x ); }

  { US x[] = { 0 };    memcpy( basic_default_ds = mem + 0x510, x, sizeof x ); }
  { US x[2] = { 0 };   memcpy( basic_clocktick_isr = mem + 0x512, x, sizeof x ); }
  { US x[2] = { 0 };   memcpy( basic_break_key_isr = mem + 0x516, x, sizeof x ); }
  { US x[2] = { 0 };   memcpy( basic_disk_error_isr = mem + 0x51a, x, sizeof x ); }
  { UC x[16]= { 0 };   memcpy( intra_app_communication_area = mem + 0x4f0, x, sizeof x ); }
  
  { UC x[8] = { 0 };   memcpy( rom_release_date = mem + 0xFfff5, x, sizeof x ); }
  { UC x[] = { 0 };    memcpy( machine_id = mem + 0xFfffe, x, sizeof x ); }
}

static int keyboard_input_with_echo(){
  bput(al, fgetc(stdin));
}

static int display_output(){
  fputs( cp437tounicode( bget(dl) ), stdout );
  bput(al,bget(dl)); if(bget(al)=='\t')bput(al,' ');
}

static int display_string(){
  f=wget(dx);
  while(mem[f]!='$')fputs( cp437tounicode( mem[f++] ), stdout );
  bput(al,'$');
}

static int get_date(){
  time_t t=time(NULL);struct tm*tm=localtime(&t);
  wput(cx,tm->tm_year);
  wput(dh,tm->tm_mon);
  wput(dl,tm->tm_mday);
  wput(al,tm->tm_wday);
}
  
static int get_time(){
  struct timeval tv;gettimeofday(&tv,0);
  time_t t=time(NULL);struct tm*tm=localtime(&t);
  bput(ch,tm->tm_hour);
  bput(cl,tm->tm_min);
  bput(dh,tm->tm_sec);
  bput(dl,tv.tv_usec/10);
}

static int open_file(){
  U mode = bget(al);
  FILE *f = fopen(mem + ds_(dx), (mode & 7) == 0? "r":
				 (mode & 7) == 1? "w": "rw");
  if(  f  ){
    U handle = next_handle ++;
    handles[ handle ] = f;
    wput(ax, handle);
    clc();
    return 0;
  }
  wput(ax, 0);
  stc();
}

static int close_file_handle(){
  U handle = wget(bx);
  fclose( handles[ handle ] );
  handles[ handle ] = 0;
  -- next_handle;
}

static int read_file(){
  U handle = wget(bx);
  U count = fread(mem + ds_(dx), 1, wget(cx), handles[ handle ]);
  if(  count  ){
    wput(ax, count);
    clc();
    return 0;
  }
  wput(ax, 5); //access denied
  stc();
}

static int write_file(){
  U handle = wget(bx);
  if(trace>1){printf("writing %d bytes from %x to handle %d\n",
                    wget(cx), wget(dx), handle); }
  U count = fwrite(mem + wget(dx), 1, wget(cx), handles[ handle ]);
  if(  count == wget(cx)  ){
    clc();
    return 0;
  }
  wput(ax, 5); //access denied
  stc();
}

static int move_file_pointer(){
  U handle = wget(bx);
  U whence = bget(al);
  fseek( handles[ handle ], qget(cx,dx), whence == 0? SEEK_SET:
					 whence == 1? SEEK_CUR:
					 whence == 2? SEEK_END: 0);
  U pos = ftell( handles[ handle ] );
  qput(dx, ax, pos);
}

static int dos( UC vv[7] ){
  if(trace>1){puts("DOS");}
  switch(bget(ah)){
  CASE 0x01: return keyboard_input_with_echo();
  CASE 0x02: return display_output();
  CASE 0x09: return display_string();

  CASE 0x2A: return get_date();
  CASE 0x2C: return get_time();

  CASE 0x3C: // create file
  CASE 0x3D: return open_file();
  CASE 0x3E: return close_file_handle();
  CASE 0x3F: return read_file();
  CASE 0x40: return write_file();
  CASE 0x42: return move_file_pointer();
  CASE 0x44: // ioctl
  CASE 0x4B: // load/execute program
  CASE 0x4C: exit(bget(al));
  CASE 0x5B: // create new file
    ;
  }
}
