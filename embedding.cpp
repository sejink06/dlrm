#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <iostream>
#include <string>
#include <fstream>
#include <unistd.h>
#include <cstdio>
#include <fcntl.h>
#include <vector>
#include <arpa/inet.h>

//PYBIND11_MAKE_OPAQUE(std::vector<float, std::allocator<float>>);

namespace py = pybind11;
using namespace std;


char *buf;
string str;
string str2;
//char buf2[64] = {0,};
float buf2[16] = {0,};
vector<float> buf3 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//int fd;
char *path_;
bool direct_;

#define BLK_SIZE 512
#define BUF_SIZE 10485760

vector<float> emb_read(off_t offset, size_t size)
{
    int fd;
    size_t new_size = size % BLK_SIZE ? (size / BLK_SIZE + 1) * BLK_SIZE : size;
    off_t new_offset = offset % BLK_SIZE ? (offset / BLK_SIZE) * BLK_SIZE : offset;
    if (BLK_SIZE - offset % BLK_SIZE < 64)
        new_size += BLK_SIZE;
    //cout << "path: " << path_ << endl;
    if (direct_)
        fd = open(path_, O_RDONLY | O_DIRECT);
    else
        fd = open(path_, O_RDONLY); 

    int ret = pread(fd, buf, new_size, new_offset);
    if (ret != new_size)
    {
        cout << "pread error" << endl;
        cout << "size: " << size << endl;
        cout << "offset: " << offset << endl;
        cout << "ret: " << ret << endl;
        cout << "fd: " << fd << endl;
        perror("a");
    }
    
    close(fd);

    int i;
    
    for (i = 0; i < 16; i++)
        memcpy(&buf3[i], buf + (offset % BLK_SIZE) + 4 * i, 4);

    return buf3;
}

void set_direct_io(char *path, bool direct)
{
    int ret;
    int length = strlen(path);
    path_ = (char *)malloc(length + 1);
    memcpy(path_, path, length + 1);
    direct_ = direct;
    /*if (direct)
        fd = open(path, O_RDONLY | O_DIRECT);
    else
        fd = open(path, O_RDONLY); */
    ret = posix_memalign((void **)&buf, BLK_SIZE, BLK_SIZE * 2);
        if (ret != 0)
            cout << "posix_memalign failed" << endl;  
}

/*void file_close()
{
    close(fd);
}*/



PYBIND11_MODULE(embedding, m)
{
    m.doc() = "pybind11 module for embedding lookup";
    m.def("set_direct_io", &set_direct_io);
    m.def("emb_read", &emb_read);
   // m.def("file_close", &file_close);
}