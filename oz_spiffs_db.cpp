#include "oz_spiffs_db.h"

#include <SPIFFS.h>

#include "esp_spiffs.h"

int oz_spiffs_db::begin(const char *DbName)
{
    if (!esp_spiffs_mounted(NULL))
        if (!SPIFFS.begin())
            return OZFS_ERR_BEGIN_FS;

    if (DbName[0] == '/')
    {
        this->_db_name = DbName;
    }
    else
    {
        return OZFS_ERR_FILENAME;
    }

    return OZFS_ERR_OK;
}

String oz_spiffs_db::read()
{
    if (this->_db_name.length() == 0)
        return String();

    if (SPIFFS.exists(this->_db_name))
    {
        String database = this->get_database_if_valid();
        if (database.length() > 0)
            return database;
        else
            SPIFFS.remove(this->_db_name);
    }

    String back_filename = this->_db_name + OZ_SPIFFS_DB_BACKUP_EXT;

    if (SPIFFS.exists(back_filename))
    {
        SPIFFS.rename(back_filename, this->_db_name);

        String database = this->get_database_if_valid();
        if (database.length() > 0)
            return database;
    }

    return String();
}

int oz_spiffs_db::print(String Database)
{
    if (this->_db_name.length() == 0)
        return OZFS_ERR_FILENAME;

    if (SPIFFS.exists(this->_db_name))
    {
        String back_filename = this->_db_name + OZ_SPIFFS_DB_BACKUP_EXT;
        SPIFFS.remove(back_filename);
        SPIFFS.rename(this->_db_name, back_filename);
    }

    File file = SPIFFS.open(this->_db_name, "w");
    if (!file)
        return OZFS_ERR_OPEN_FILE;

    
    unsigned char crc = this->generate_crc(Database);
    Database += (char) crc;

    size_t printed_size = file.print(Database);
    file.close();

    if (printed_size)
        return OZFS_ERR_OK;
    else
        return OZFS_ERR_SAVEFILE;
}

unsigned char oz_spiffs_db::generate_crc(String str)
{
    unsigned char crc = 0;
    for (unsigned char c : str)
    {
        crc += c;
    }
    crc = map(crc,0,255,32,254);
    if(crc == 127) crc = 126;
    return crc;
}

bool oz_spiffs_db::check_crc(String &str)
{
    unsigned char crc = str[str.length() - 1];
    str.remove(str.length() - 1);
    return crc == generate_crc(str);
}

String oz_spiffs_db::get_database_if_valid()
{
    File file = SPIFFS.open(this->_db_name);
    if (file)
    {
        String database_str = file.readString();
        file.close();
        bool ceck = check_crc(database_str);

        if (database_str.length() > 0 && ceck)
        {
            return database_str;
        }
    }

    return String();
}