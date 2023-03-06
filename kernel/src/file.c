#include "klib.h"
#include "file.h"

#define TOTAL_FILE 128

file_t files[TOTAL_FILE];

static file_t *falloc() {
  // Lab3-1: find a file whose ref==0, init it, inc ref and return it, return NULL if none
  //TODO();
  for (int i = 0; i < TOTAL_FILE; i++)
	if (files[i].ref == 0) {
		files[i].type = TYPE_NONE;
		files[i].ref++;
		return &files[i];
	}
  return NULL;
}

file_t *fopen(const char *path, int mode) {
  file_t *fp = falloc();
  inode_t *ip = NULL;
  if (!fp) goto bad;
  // TODO: Lab3-2, determine type according to mode
  // iopen in Lab3-2: if file exist, open and return it
  //       if file not exist and type==TYPE_NONE, return NULL
  //       if file not exist and type!=TYPE_NONE, create the file as type
  // you can ignore this in Lab3-1
  int open_type;
  if (!(mode & O_CREATE)) open_type = TYPE_NONE;
  else {
	if (mode & O_DIR) open_type = TYPE_DIR;
	else open_type = TYPE_FILE;
  }
  ip = iopen(path, open_type);
  if (!ip) goto bad;
  int type = itype(ip);
  // TODO: Lab3-2, if type is not DIR, go bad if mode&O_DIR
  if (type != TYPE_DIR && (mode & O_DIR)) goto bad;
  // TODO: Lab3-2, if type is DIR, go bad if mode WRITE or TRUNC
  if (type == TYPE_DIR && ((mode & O_WRONLY) || (mode & O_RDWR) || (mode & O_TRUNC)))
	goto bad;
  // TODO: Lab3-2, if mode&O_TRUNC, trunc the file
  if (mode & O_TRUNC) itrunc(ip);
  if (type == TYPE_FILE || type == TYPE_DIR) {
    fp->type = TYPE_FILE; // file_t don't and needn't distingush between file and dir
    fp->inode = ip;
    fp->offset = 0;
  } else if (type == TYPE_DEV) {
    fp->type = TYPE_DEV;
    fp->dev_op = dev_get(idevid(ip));
    iclose(ip);
    ip = NULL;
  } else assert(0);
  fp->readable = !(mode & O_WRONLY);
  fp->writable = (mode & O_WRONLY) || (mode & O_RDWR);
  //assert(fp->inode);
  return fp;
bad:
  if (fp) fclose(fp);
  if (ip) iclose(ip);
  return NULL;
}

int fread(file_t *file, void *buf, uint32_t size) {
  // Lab3-1, distribute read operation by file's type
  // remember to add offset if type is FILE (check if iread return value >= 0!)
  if (!file->readable) return -1;
  //TODO();
  int num = 0;
  if (file->type == TYPE_FILE)
	num = iread(file->inode, file->offset, buf, size);
  else num = file->dev_op->read(buf, size);
  if (num == -1) return -1;
  if (file->type == TYPE_FILE) file->offset += num;
  return num;
}

int fwrite(file_t *file, const void *buf, uint32_t size) {
  // Lab3-1, distribute write operation by file's type
  // remember to add offset if type is FILE (check if iwrite return value >= 0!)
  if (!file->writable) return -1;
  //TODO();
  int num = 0;
  if (file->type == TYPE_FILE)
	num = iwrite(file->inode, file->offset, buf, size);
  else num = file->dev_op->write(buf, size);
  if (num == -1) return -1;
  if (file->type == TYPE_FILE) file->offset += num;
  return num;
}

uint32_t fseek(file_t *file, uint32_t off, int whence) {
  // Lab3-1, change file's offset, do not let it cross file's size
  if (file->type == TYPE_FILE) {
    //TODO();
	switch (whence) {
	  case SEEK_SET: file->offset = off; break;
	  case SEEK_END: file->offset = isize(file->inode) + off; break;
	  case SEEK_CUR: file->offset += off; break;
	  default: assert(0); break;
	}
	return file->offset;
  }
  return -1;
}

file_t *fdup(file_t *file) {
  // Lab3-1, inc file's ref, then return itself
  //TODO();
  file->ref++;
  return file;
}

void fclose(file_t *file) {
  // Lab3-1, dec file's ref, if ref==0 and it's a file, call iclose
  //TODO();
  //assert(file->inode);
  file->ref--;
  if (file->ref == 0 && file->type == TYPE_FILE) iclose(file->inode);
}
