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

#define OZ_DB_CONST_INTERNAL "internal"
#define OZ_DB_CONST_BASE "base"
#define OZ_DB_CONST_PLUGIN "plugin"

#define OZ_DB_ERASE_TIMER 40000 // ms
#define OZ_DB_SAVE_TIMER   5000   // ms
#define OZ_DB_MIN_SIZE 1024


#define OZ_DB_JSONDOCUMENT_DIMENSION 8192

enum oz_db_error
{
  OZDB_ERR_OK = 0,
  OZDB_ERR_OPEN_DB = 1,
  OZDB_ERR_PARM_NOT_EXIST = 2,
  OZDB_ERR_DESERIALIZATION = 3,
  OZDB_ERR_SERIALIZATION = 4,
  OZDB_ERR_OPEN_FILE = 5,
  OZDB_ERR_CLEAR_FILE = 6
};

/**
 * @b oz_db database class provide to store in file all parameter definite in oz_db_base_parameter (oz_db_config.h) 
 * 
 *          { 
 *            "base":{"pram1":"value1","pram2":"value2",......},
 *            "internal":{"pram1":"value1","pram2":"value2",......},
 *            "plugin":[
 *                      {"pram1":"value1","pram2":"value2",.....},
 *                      {"pram1":"value1","pram2":"value2","pram3":"value3",......}
 *                     ]
 *         }
 * 
 */
class oz_db
{
public:

  oz_db(std::map<String, String> default_value);



  /**
   * @b get_all_db() : get all database string for save local copy 
   *
   * 
   *
   * @return 
   *
   */
  String get_all_db();


  /**
   * @b set_all_db() : Set all database string for backup
   *
   * 
   *
   * @return 
   *
   */
  int set_all_db(String dbstr);


  /**
   * @b clear_db() : clear all database file
   *
   * 
   *
   * @return 
   *
   */
  int clear_db();

  /**
   * @b get_base_parameter() : get parameter value by string key if parametr was foud in database, else get default value
   *
   * @param Name  key string
   *
   * @return parameter in database if parameter is not in database return
   *         default. On error or parameter not exist return empty String
   *
   */
  String get_base_parameter(String Name);

  /**
   * @b get_base_parameter_f() : get parameter value by string key if parametr was foud in database, else get default value
   *
   * @param format  key string
   *
   * @return parameter in database if parameter is not in database return
   *         default. On error or parameter not exist return empty String
   *
   */
  String get_base_parameter_f(const char *format, ...);

  /**
   * @b set_base_parameter : Set parameter value in database file
   *
   * @param Name   key string
   * @param Value  value to strore in db
   *
   * @return  if insert ok return OZDB_ERR_OK else return error code (oz_db_error)
   *
   */
  oz_db_error set_base_parameter(String Name, String  Value);

  /**
   * @b get_internal_parameter() : get internal parameter value by string key if parametr was foud in database, else get default value
   *
   * @param Name  key string
   *
   * @return parameter in database if parameter is not in database return
   *         default. On error or parameter not exist return empty String
   *
   */
  String get_internal_parameter(String Name);

  /**
   * @b get_internal_parameter_f() : get internal parameter value by string key if parametr was foud in database, else get default value
   *
   * @param format  key string
   *
   * @return parameter in database if parameter is not in database return
   *         default. On error or parameter not exist return empty String
   *
   */
  String get_internal_parameter_f(const char *format, ...);

  /**
   * @b set_internal_parameter : Set internal parameter value in database file
   *
   * @param Name   key string
   * @param Value  value to strore in db
   *
   * @return  if insert ok return OZDB_ERR_OK else return error code (oz_db_error)
   *
   */
  oz_db_error set_internal_parameter(String Name, String  Value);



  /**
   * @b get_plugin   Get setted plugin
   *
   * 
   * @return  JsonArray
   * 
   */
  JsonArray get_plugin();

  /**
   * @b set_plugin   Put in database all configured plugin
   *
   * @param plugins String in Json Format LIKE [{"key":"value","key2":"value"},{"keyA":"value","keyA2":"value","keyA3":"value"},{"keyB":"value","keyB2":"value"}]
   *
   * @return  OZDB_ERR_OK if success : error code if an error occurred
   */
  int set_plugin(const char *plugins);

  /**
   * @b commit   Put in database all configured plugin
   *
   * @return  OZDB_ERR_OK if success : error code if an error occurred
   */
  int commit();

  /**
   * @b destructor
   * 
   * 
   */
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
  TimerHandle_t _free_timer;
  DynamicJsonDocument *_Jdb;
  std::map<String,String> _default;
};

#endif