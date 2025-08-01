#ifndef FTP_H
#define FTP_H

#include "include/types.h"

// FTP client functions (placeholders)
int ftp_connect(const char* host, uint16_t port, const char* username, const char* password);
int ftp_list_directory(int connection_id, const char* path);
int ftp_download_file(int connection_id, const char* remote_path, const char* local_path);
int ftp_upload_file(int connection_id, const char* local_path, const char* remote_path);
int ftp_disconnect(int connection_id);

#endif // FTP_H
