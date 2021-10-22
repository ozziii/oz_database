
#include "oz_db.h"

void oz_db_save_task_ISR(TimerHandle_t xTimer)
{
    auto database = (oz_db *)pvTimerGetTimerID(xTimer);
    database->_save_database();
}

void oz_db_free_task_ISR(TimerHandle_t xTimer)
{
    auto database = (oz_db *)pvTimerGetTimerID(xTimer);
    database->_free_database();
}

oz_db::oz_db(std::map<const char *, const char *> default_value)
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

    this->_save_timer = xTimerCreate(
        "oz_db_save_task",
        pdMS_TO_TICKS(OZ_DB_SAVE_TIMER),
        pdFALSE,
        (void *)this,
        oz_db_save_task_ISR);
}

String oz_db::get_base_parameter(const char *Name)
{
    if (!this->_is_open)
        if (!this->_load_database())
        {
            OZDB_LOGE("Error load database");
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
            OZDB_LOGW("Parameter (%s) not exist", Name);
            return String();
        }

        return String(default_value->second);
    }
}

oz_db_error oz_db::set_base_parameter(const char *Name, const char *Value)
{
    if (this->_default.find(Name) == this->_default.end())
    {
        OZDB_LOGW("Parameter (%s) not exist", Name);
        return OZDB_ERR_PARM_NOT_EXIST;
    }

    if (!this->_is_open)
        if (!this->_load_database())
        {
            OZDB_LOGE("Error load database");
            return OZDB_ERR_OPEN_DB;
        }

    (*this->_Jdb)[OZ_DB_CONST_BASE][Name] = Value;

    xTimerStart(this->_save_timer, 0);

    return OZDB_ERR_OK;
}

JsonArray oz_db::get_plugin()
{
    if (!this->_is_open)
        if (!this->_load_database())
        {
            OZDB_LOGE("Error load database");
            return JsonArray();
        }

    return (*this->_Jdb)[OZ_DB_CONST_PLUGIN];
}

int oz_db::set_plugin(const char *plugins)
{
    if (!this->_is_open)
        if (!this->_load_database())
        {
            OZDB_LOGE("Error load database");
            return OZDB_ERR_OPEN_DB;
        }

    DynamicJsonDocument Jplugins(4000);

    if (deserializeJson(Jplugins, plugins))
    {
        OZDB_LOGE("Deserialization error");
        return OZDB_ERR_DESERIALIZATION;
    }

    (*this->_Jdb)[OZ_DB_CONST_PLUGIN] = Jplugins;

    xTimerStart(this->_save_timer, 0);

    return OZDB_ERR_OK;
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
        OZDB_LOGE("Error on open file");
        delete file;
        return OZDB_ERR_OPEN_FILE;
    }

    // TODO select DB DIMENSION
    this->_Jdb = new DynamicJsonDocument(4000);

    String db_str = file->read();

    if (db_str.length() > 0)
    {
        DeserializationError error = deserializeJson(*this->_Jdb, db_str);

        if (error)
        {
            delete this->_Jdb;
            delete file;
            OZDB_LOGE("Deserialize error [%s] ", error.c_str());
            return 0;
        }
    }
    else
    {
        OZDB_LOGD("File is Empty");
    }

    delete file;

    xTimerStart(this->_free_timer, 0);
    this->_is_open = true;

    return OZDB_ERR_OK;
}

int oz_db::_save_database()
{
    if (!this->_is_open)
    {
        OZDB_LOGE("Error db not open");
        OZDB_ERR_OPEN_DB;
    }

    String newDB = String();

    size_t err = serializeJson(*this->_Jdb, newDB);

    if (err == 0)
    {
        OZDB_LOGE("Seserialize error");
        return OZDB_ERR_SERIALIZATION;
    }

    i_oz_fs *file = this->_get_file();
    err = file->begin("/db.json");

    if (err != OZFS_ERR_OK)
    {
        OZDB_LOGE("Error on open file");
        delete file;
        return OZDB_ERR_OPEN_FILE;
    }

    file->print(newDB);
    delete file;

    OZDB_LOGD("Database Saved!");
    return OZDB_ERR_OK;
}

int oz_db::_free_database()
{
    OZDB_LOGD("Free database memory");
    this->_is_open = false;
    this->_Jdb->clear();
    delete this->_Jdb;
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