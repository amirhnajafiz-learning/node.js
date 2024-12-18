#include "filemap.h"

// file map is a custome struct to keep file targets and redirections.
struct FileMap
{
    char original_filename[PATH_MAX];
    char tmp_filename[PATH_MAX];
};
