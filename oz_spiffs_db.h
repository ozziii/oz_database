#ifndef OZ_SPIFFS_DB_H
#define OZ_SPIFFS_DB_H

#define OZ_SPIFFS_DB_BACKUP_EXT   ".back"

#include <i_oz_fs.h>
/**
 *
 *
 *
 *
 *
 *
 */
class oz_spiffs_db : public i_oz_fs
{
public:
    int begin(const char *DbName) override;
    String read() override;
    int print(String Database) override;

protected:
    unsigned char generate_crc(String str);
    bool check_crc(String &str);
    String get_database_if_valid();
    String _db_name;
    File _file;
};

#endif