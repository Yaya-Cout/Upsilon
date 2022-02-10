#include "archive.h"
#include "extapp_api.h"
#include "../global_preferences.h"

#include <string.h>
#include <stdlib.h>

#ifndef DEVICE
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace External {
namespace Archive {

#ifdef DEVICE

struct TarHeader
{                              /* byte offset */
  char name[100];               /*   0 */
  char mode[8];                 /* 100 */
  char uid[8];                  /* 108 */
  char gid[8];                  /* 116 */
  char size[12];                /* 124 */
  char mtime[12];               /* 136 */
  char chksum[8];               /* 148 */
  char typeflag;                /* 156 */
  char linkname[100];           /* 157 */
  char magic[8];                /* 257 */
  char uname[32];               /* 265 */
  char gname[32];               /* 297 */
  char devmajor[8];             /* 329 */
  char devminor[8];             /* 337 */
  char padding[167];            /* 345 */
} __attribute__((packed));

static_assert(sizeof(TarHeader) == 512);

bool isSane(const TarHeader* tar) {
  return !memcmp(tar->magic, "ustar  ", 8) && tar->name[0] != '\x00' && tar->name[0] != '\xFF';
}

bool isExamModeAndFileNotExecutable(const TarHeader* tar) {
  return GlobalPreferences::sharedGlobalPreferences()->isInExamMode() && (tar->mode[4] & 0x01) == 0;
}

bool fileAtIndex(size_t index, File &entry) {
  if (index == -1)
    return false;
  
  const TarHeader* tar = reinterpret_cast<const TarHeader*>(0x90200000);
  unsigned size = 0;

  // Sanity check.
  if (!isSane(tar) || isExamModeAndFileNotExecutable(tar)) {
    return false;
  }

  /**
   * TAR files are comprised of a set of records aligned to 512 bytes boundary
   * followed by data.
   */
  
  for(;;) {
    // Calculate the size
    size = 0;
    for (int i = 0; i < 11; i++)
      size = size * 8 + (tar->size[i] - '0');
    
    // Check if we found our file.
    if (index == 0) {
      // If yes, check for sanity and for exam mode stuff
      if (!isSane(tar) || isExamModeAndFileNotExecutable(tar)) {
        return false;
      }

      // File entry found, copy data out.
      entry.name = tar->name;
      entry.data = reinterpret_cast<const uint8_t*>(tar) + sizeof(TarHeader);
      entry.dataLength = size;
      entry.isExecutable = (tar->mode[4] & 0x01) == 1;

      return true;
    } else {
      // move to the next TAR header.
      unsigned stride = (sizeof(TarHeader) + size + 511);
      stride = (stride >> 9) << 9;
      tar = reinterpret_cast<const TarHeader*>(reinterpret_cast<const char*>(tar) + stride);

      // Sanity check.
      if (!isSane(tar)) {
        return false;
      }
    }
    index--;
  }
  
  // Achievement unlock: How did we get there ?
  return false;
}

extern "C" void (* const apiPointers[])(void);
typedef uint32_t (*entrypoint)(const uint32_t, const void *, void *, const uint32_t);

uint32_t executeFile(const char *name, void * heap, const uint32_t heapSize) {
  File entry;
  if(fileAtIndex(indexFromName(name), entry)) {
    if(!entry.isExecutable) {
      return 0;
    }
    uint32_t ep = *reinterpret_cast<const uint32_t*>(entry.data);
    if(ep >= 0x90200000 && ep < 0x90800000) {
      return ((entrypoint)ep)(API_VERSION, apiPointers, heap, heapSize);
    }
  }
  return -1;
}

int indexFromName(const char *name) {
  File entry;

  for (int i = 0; fileAtIndex(i, entry); i++) {
    if (strcmp(name, entry.name) == 0) {
      return i;
    }
  }

  return -1;
}

size_t numberOfFiles() {
  File dummy;
  size_t count;

  for (count = 0; fileAtIndex(count, dummy); count++);

  return count;
}

bool executableAtIndex(size_t index, File &entry) {
  File dummy;
  size_t count;
  size_t final_count = 0;

  for (count = 0; fileAtIndex(count, dummy); count++) {
    if (dummy.isExecutable) {
      if (final_count == index) {
        entry.name = dummy.name;
        entry.data = dummy.data;
        entry.dataLength = dummy.dataLength;
        entry.isExecutable = dummy.isExecutable;
        return true;
      }
      final_count++;
    }
  }
  
  return false;
}

size_t numberOfExecutables() {
  File dummy;
  size_t count;
  size_t final_count = 0;

  for (count = 0; fileAtIndex(count, dummy); count++)
    if (dummy.isExecutable)
      final_count++;

  return final_count;
}



#else

bool fileAtIndex(size_t index, File &entry) {
  // Check if index is inferior or equal to the number of files
  if (index >= numberOfFiles())
    return false;

  DIR *d = opendir(".");
  if (d) {
    size_t nb = 0;
    dirent *file;
    while ((file = readdir(d)) != NULL) {
      // Allow only files
      if (file->d_type != (unsigned char)8)
        continue;
      // If the index is equal to the actual file
      if (nb == index) {
        // Set name
        entry.name = strdup(file->d_name);
        // Initialize default values
        entry.data = nullptr;
        entry.dataLength = 0;

        struct stat info;
        if (stat(entry.name, &info) != 0) {
          // Close the directory and return false
          closedir(d);
          return false;
        }

        unsigned char* content = new unsigned char[info.st_size]; // FIXME: This creates sometimes a memory leak
        if (content == NULL) {
          // Close the directory and return false
          closedir(d);
          return false;
        }
        // Open file
        FILE *fp = fopen(entry.name, "rb");
        if (fp == NULL) {
          // Desallocate memory
          delete content;
          // Close the directory and return false
          closedir(d);
          return false;
        }
        // Copy content of file to content variable
        fread(content, info.st_size, 1, fp);
        // Close file
        fclose(fp);
        // Save data to their corresponding variables
        entry.data = content;
        entry.dataLength = info.st_size;
        // Check if file is executable
        entry.isExecutable = (!access(entry.name, X_OK));
        // Close the directory and quit the function because the file has been found
        closedir(d);
        return true;
      }
      // Increase the position
      nb++;
    }
  }
  // This code shouldn't be executed
  closedir(d); // Close the directory
  assert(false); // This should already have been checked at start of this function
  return false;
}

bool executableAtIndex(size_t index, File &entry) {
  if (index >= numberOfFiles() + (size_t)(numberOfExecutables() == 0))
    return false;

  File dummy;
  size_t count;
  size_t final_count = 0;

  for (count = 0; fileAtIndex(count, dummy); count++) {
    // Desallocate memory to prevent memory leak
    delete[] dummy.data;
    free(const_cast<char *>(dummy.name));
    if (dummy.isExecutable) {
      if (final_count == index) {
        // TODO: Execute app, and add filename without memory leak
        entry.name = "Built-in";
        entry.data = NULL; // Don't store the data because of the memory leak
        // entry.name = dummy.name;
        // entry.data = dummy.data;
        entry.dataLength = dummy.dataLength;
        entry.isExecutable = dummy.isExecutable;
        return true;
      }
      final_count++;
    }
  }
  // If no app was found, use the build-in app
  if (final_count == 0 ) {
    entry.name = "Built-in";
    entry.data = NULL;
    entry.dataLength = 0;
    entry.isExecutable = true;
    return true;
  }
  return false;
}

size_t numberOfExecutables() {
  File dummy;
  size_t count;
  size_t final_count = 0;

  for (count = 0; fileAtIndex(count, dummy); count++) {
    if (dummy.isExecutable) {
      final_count++;
    }
    // Desallocate memory to prevent memory leak
    delete[] dummy.data;
    free(const_cast<char *>(dummy.name));
  }

  if (final_count == 0) {
    return 1;
  }
  return final_count;
}

extern "C" void extapp_main(void);

uint32_t executeFile(const char *name, void * heap, const uint32_t heapSize) {
  extapp_main();
  return 0;
}

int indexFromName(const char *name) {
  File entry;
  // Enable returnFilename because of the memory is desallocated after
  for (int i = 0; fileAtIndex(i, entry); i++) {
    delete[] entry.data;
    if (strcmp(name, entry.name) == 0) {
      // Desallocate memory to prevent memory leak
      free(const_cast<char *>(entry.name));
      return i;
    }
    // Desallocate memory to prevent memory leak
    free(const_cast<char *>(entry.name));
  }

  return -1;
}

size_t numberOfFiles() {
  DIR *d = opendir(".");
  int nb = 0;
  if (d) {
    dirent *file;
    while ((file = readdir(d)) != NULL) {
        // Allow only files
        if (file->d_type != (unsigned char)8)
          continue;
        nb++;
    }
  }
  closedir(d);
  return nb;
}

#endif

}
}
