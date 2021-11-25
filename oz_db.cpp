
#include "oz_db.h"

void oz_db_free_task_ISR(TimerHandle_t xTimer)
{
    auto database = (oz_db *)pvTimerGetTimerID(xTimer);
    database->_free_database();
}

oz_db::oz_db(std::map<String, String> default_value)
{
    if (default_value.size() > 0)
        this->_default = default_value;

    this->_is_open = false;

    this->_free_timer = xTimerCreate(
        "oz_db_free_task",
        pdMS_TO_TICKS(OZ_DB_ERASE_TIMER),
        pdFALSE,
        (void *)this,
        oz_db_free_task_ISR);
}

String oz_db::get_all_db()
{
    if (!this->_is_open)
        if (this->_load_database() != OZDB_ERR_OK)
        {
            log_e("Error load database");
            return String();
        }

    String databaseStr = String();

    size_t err = serializeJson(*this->_Jdb, databaseStr);

    if (err == 0)
    {
        log_e("Seserialize error");
        return String();
    }

    return databaseStr;
}

int oz_db::set_all_db(String dbstr)
{
    this->_free_database();

    this->_Jdb = new DynamicJsonDocument(OZ_DB_JSONDOCUMENT_DIMENSION);

    DeserializationError error = deserializeJson(*this->_Jdb, dbstr);

    if (error)
    {
        delete this->_Jdb;
        this->_Jdb = nullptr;

        log_e("Deserialize error [%s] ", error.c_str());
        return OZDB_ERR_DESERIALIZATION;
    }

    this->_is_open = true;
    return this->commit();
}

int oz_db::clear_db()
{
    i_oz_fs *file = this->_get_file();
    file->clear();
    return this->_free_database();
}

String oz_db::get_base_parameter(String Name)
{
    if (!this->_is_open)
        if (this->_load_database() != OZDB_ERR_OK)
        {

            log_e("Error load database");
            return String();
        }

    if ((*this->_Jdb)[OZ_DB_CONST_BASE].containsKey(Name))
    {
        String ret = (*this->_Jdb)[OZ_DB_CONST_BASE][Name];
        return ret;
    }
    else
    {
        auto default_value = this->_default.find(Name);

        if (default_value == this->_default.end())
        {
            log_w("Parameter (%s) not exist", Name);
            return String();
        }

        return String(default_value->second);
    }
}

String oz_db::get_base_parameter_f(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int len = vsnprintf(nullptr, 0, format, args) + 1;
    char *buffer = new char[len];
    vsnprintf(buffer, len, format, args);
    va_end(args);
    String ret = this->get_base_parameter(buffer);
    delete[] buffer;
    return ret;
}

oz_db_error oz_db::set_base_parameter(String Name, String Value)
{
    if (this->_default.find(Name) == this->_default.end())
    {
        log_w("Parameter (%s) not exist", Name);
        return OZDB_ERR_PARM_NOT_EXIST;
    }

    if (!this->_is_open)
        if (this->_load_database() != OZDB_ERR_OK)
        {
            log_e("Error load database");
            return OZDB_ERR_OPEN_DB;
        }

    (*this->_Jdb)[OZ_DB_CONST_BASE][Name] = Value;

    return OZDB_ERR_OK;
}

String oz_db::get_internal_parameter(String Name) 
{
    if (!this->_is_open)
        if (this->_load_database() != OZDB_ERR_OK)
        {

            log_e("Error load database");
            return String();
        }

    if ((*this->_Jdb)[OZ_DB_CONST_INTERNAL].containsKey(Name))
    {
        String ret = (*this->_Jdb)[OZ_DB_CONST_INTERNAL][Name];
        return ret;
    }

    log_d("Inrenal setting not found");
    return String();
}

String oz_db::get_internal_parameter_f(const char *format, ...) 
{
    va_list args;
    va_start(args, format);
    int len = vsnprintf(nullptr, 0, format, args) + 1;
    char *buffer = new char[len];
    vsnprintf(buffer, len, format, args);
    va_end(args);
    String ret = this->get_internal_parameter(buffer);
    delete[] buffer;
    return ret;
}

oz_db_error oz_db::set_internal_parameter(String Name, String Value)
{
    if (!this->_is_open)
        if (this->_load_database() != OZDB_ERR_OK)
        {
            log_e("Error load database");
            return OZDB_ERR_OPEN_DB;
        }

    (*this->_Jdb)[OZ_DB_CONST_INTERNAL][Name] = Value;

    return OZDB_ERR_OK;
}

JsonArray oz_db::get_plugin()
{
    if (!this->_is_open)
        if (this->_load_database() != OZDB_ERR_OK)
        {
            log_e("Error load database");
            return JsonArray();
        }

    return (*this->_Jdb)[OZ_DB_CONST_PLUGIN];
}

int oz_db::set_plugin(const char *plugins)
{
    if (!this->_is_open)
        if (this->_load_database() != OZDB_ERR_OK)
        {
            log_e("Error load database");
            return OZDB_ERR_OPEN_DB;
        }

    DynamicJsonDocument Jplugins(OZ_DB_JSONDOCUMENT_DIMENSION);

    if (deserializeJson(Jplugins, plugins))
    {
        log_e("Deserialization error");
        return OZDB_ERR_DESERIALIZATION;
    }

    (*this->_Jdb)[OZ_DB_CONST_PLUGIN] = Jplugins;

    return OZDB_ERR_OK;
}

int oz_db::commit()
{
    return this->_save_database();
}

oz_db::~oz_db()
{
    delete this->_Jdb;
}

int oz_db::_load_database()
{
    i_oz_fs *file = this->_get_file();
    int err = file->begin("/db.json");

    if (err != OZFS_ERR_OK)
    {
        log_e("Error on open file");
        delete file;
        return OZDB_ERR_OPEN_FILE;
    }

    this->_Jdb = new DynamicJsonDocument(OZ_DB_JSONDOCUMENT_DIMENSION);

    String db_str = file->read();

    if (db_str.length() > 0)
    {
        DeserializationError error = deserializeJson(*this->_Jdb, db_str);

        if (error)
        {
            delete this->_Jdb;
            delete file;
            log_e("Deserialize error [%s] ", error.c_str());
            return 0;
        }
    }
    else
    {
        log_d("File is Empty");
    }

    delete file;

    xTimerStart(this->_free_timer, 0);
    this->_is_open = true;

    log_v("Open Database");
    return OZDB_ERR_OK;
}

int oz_db::_save_database()
{
    if (!this->_is_open)
    {
        log_e("Error db not open");
        return OZDB_ERR_OPEN_DB;
    }

    String newDB = String();

    size_t err = serializeJson(*this->_Jdb, newDB);

    if (err == 0)
    {
        log_e("Seserialize error");
        return OZDB_ERR_SERIALIZATION;
    }

    i_oz_fs *file = this->_get_file();
    err = file->begin("/db.json");

    if (err != OZFS_ERR_OK)
    {
        log_e("Error on open file");
        delete file;
        return OZDB_ERR_OPEN_FILE;
    }

    file->print(newDB);
    delete file;

    log_v("Database Saved!");
    return OZDB_ERR_OK;
}

int oz_db::_free_database()
{
    this->_is_open = false;
    if (this->_Jdb != nullptr)
    {
        this->_Jdb->clear();
        delete this->_Jdb;
        this->_Jdb = nullptr;
    }

    log_v("Db free database memory Actual ram: %u", ESP.getFreeHeap());
    return OZDB_ERR_OK;
}

i_oz_fs *oz_db::_get_file()
{
#if OZ_DATABASE_TYPE == OZ_SPIFFS_DB
    return new oz_spiffs_db();
#endif
#if OZ_DATABASE_TYPE == OZ_SD_DB

#endif
}