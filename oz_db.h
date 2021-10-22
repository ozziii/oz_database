#ifndef OZ_DB_H
#define OZ_DB_H

#include <map>
#include "ArduinoJson.h"

#include "i_oz_fs.h"

#define OZ_SPIFFS_DB (0)
#define OZ_SD_DB     (1)

#define OZ_DATABASE_TYPE  OZ_SPIFFS_DB

#if OZ_DATABASE_TYPE == OZ_SPIFFS_DB
#include "oz_spiffs_db.h"
#endif
#if OZ_DATABASE_TYPE == OZ_SD_DB

#endif


#define OZDB_LOG_LEVEL_NONE (0)
#define OZDB_LOG_LEVEL_VERBOSE (1)
#define OZDB_LOG_LEVEL_DEBUG (2)
#define OZDB_LOG_LEVEL_INFO (3)
#define OZDB_LOG_LEVEL_WARN (4)
#define OZDB_LOG_LEVEL_ERROR (5)

#define OZDB_DEBUG_LEVEL OZDB_LOG_LEVEL_ERROR

#define OZDB_LOG_FORMAT_E "[E][OZDB] %s \r\n";
#define OZDB_LOG_FORMAT_W "[W][OZDB] %s \r\n";
#define OZDB_LOG_FORMAT_I "[I][OZDB] %s \r\n";
#define OZDB_LOG_FORMAT_D "[D][OZDB] %s \r\n";
#define OZDB_LOG_FORMAT_V "[V][OZDB] %s \r\n";

#define OZDB_LOG_FORMAT(letter, format) "[" #letter "][OZDB] " format "\r\n"

#if OZDB_DEBUG_LEVEL >= OZDEBUG_LOG_LEVEL_VERBOSE
#define OZDB_LOGV(format, ...) log_v(OZDB_LOG_FORMAT(V, format), ##__VA_ARGS__)
#else
#define OZDB_LOGV(...)
#endif

#if OZDB_DEBUG_LEVEL >= OZDEBUG_LOG_LEVEL_DEBUG
#define OZDB_LOGD(...) log_v(OZDB_LOG_FORMAT(D, format), ##__VA_ARGS__)
#else
#define OZDB_LOGD(...)
#endif

#if OZDB_DEBUG_LEVEL >= OZDEBUG_LOG_LEVEL_INFO
#define OZDB_LOGI(...) log_v(OZDB_LOG_FORMAT(I, format), ##__VA_ARGS__)
#else
#define OZDB_LOGI(...)
#endif

#if OZDB_DEBUG_LEVEL >= OZDEBUG_LOG_LEVEL_WARN
#define OZDB_LOGW(...) log_v(OZDB_LOG_FORMAT(W, format), ##__VA_ARGS__)
#else
#define OZDB_LOGW(...)
#endif

#if OOZDB_DEBUG_LEVEL >= OZDEBUG_LOG_LEVEL_ERROR
#define OZDB_LOGE(...) log_v(OZDB_LOG_FORMAT(E, format), ##__VA_ARGS__)
#else
#define OZDB_LOGE(...)
#endif


#define OZ_DB_CONST_BASE "base"
#define OZ_DB_CONST_PLUGIN "plugin"

#define OZ_DB_ERASE_TIMER 30000 // ms
#define OZ_DB_SAVE_TIMER 2000   // ms
#define OZ_DB_MIN_SIZE 1024

enum oz_db_error
{
  OZDB_ERR_OK = 0,
  OZDB_ERR_OPEN_DB = 1,
  OZDB_ERR_PARM_NOT_EXIST = 2,
  OZDB_ERR_DESERIALIZATION = 3,
  OZDB_ERR_SERIALIZATION = 4,
  OZDB_ERR_OPEN_FILE = 5
};

/**
 * @b oz_db database class provide to store in file all parameter definite in oz_db_base_parameter (oz_db_config.h) 
 *          and store all plugin definition in format [{pram1 parm2 ...},{pram1 parm2 ...},{pram1 parm2 ...}....]
 * 
 */
class oz_db
{
public:

  oz_db(std::map<const char *, const char *> default_value);


  /**
   * @b get_base_parameter() : get parameter value by string key if parametr was foud in database, else get default value
   *
   * @param Name  key string
   *
   * @return parameter in database if parameter is not in database return
   *         default. On error or parameter not exist return empty String
   *
   */
  String get_base_parameter(const char *Name);

  /**
   * @b set_base_parameter : Set parameter value in database file
   *
   * @param Name   key string
   * @param Value  value to strore in db
   *
   * @return  if insert ok return OZDB_ERR_OK else return error code (oz_db_error)
   *
   */
  oz_db_error set_base_parameter(const char *Name, const char *Value);

  /**
   * @b get_plugin   Get setted plugin
   *
   *
   *
   * @return  JsonArray
   *
   */
  JsonArray get_plugin();

  /**
   * @b set_plugin   Put in database all configured plugin
   *
   *
   * @return  OZDB_ERR_OK if success : error code if an error occurred
   */
  int set_plugin(const char *plugins);

  ~oz_db();

  /**
   * @b _load_database private method NOT USE
   */
  int _load_database();

  /**
   * @b _save_database private method NOT USE
   */
  int _save_database();

  /**
   * @b _free_database private method NOT USE
   */
  int _free_database();

protected:
  i_oz_fs * _get_file();

  bool _is_open = false;
  TimerHandle_t _free_timer, _save_timer;
  DynamicJsonDocument *_Jdb;
  std::map<const char *, const char *> _default;
};

#endif