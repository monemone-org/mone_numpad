// user_config.h


typedef union {
  uint32_t raw;
  struct {
    // is_win_mode: this affect Youtube shortcuts
    //      0 by default
    bool     is_win_mode :1; 
  };
} user_config_t;


extern user_config_t user_config;

void read_user_config(void);
void save_user_config(void);


