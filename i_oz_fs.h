#ifndef I_OZ_FS_H
#define I_OZ_FS_H


#include <FS.h>


enum i_oz_fs_error {
  OZFS_ERR_OK = 0,
  OZFS_ERR_BEGIN_FS = 1,
  OZFS_ERR_OPEN_FILE = 2,
  OZFS_ERR_FILE_NOT_OPEN = 3,
  OZFS_ERR_FILENAME = 4,
  OZFS_ERR_SAVEFILE = 5
};





/**
 *
 *
 *
 *
 *
 *
 */
class i_oz_fs
{
public:

  virtual int begin(const char * db_name) = 0;

  virtual String read() = 0;

  virtual int print(String value) = 0;

  virtual int clear() = 0;

  virtual ~i_oz_fs() = default;
};

#endif