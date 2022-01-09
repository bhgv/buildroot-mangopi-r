// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <map>
#include <iostream>
#include <string>
#include <cstdio>
#include <boost/algorithm/string/case_conv.hpp>

//#include <dirent.h> // used by scandir()
//#include "GnashSystemIOHeaders.h" // used by unlink()
//#include <fcntl.h>  // used by asyncmode()

#include <sys/socket.h>
#include <sys/un.h>

//#include "VM.h"
#include "log.h"
#include "fn_call.h"
#include "as_object.h"
#include "Global_as.h"
#include "usocketio.h"
//#include "Array_as.h"  // used by scandir()
#include "as_function.h"

extern "C" {
#include <errno.h>
#include <string.h>

#define D() //printf("%s> %s:%d\n", __FILE__, __func__, __LINE__)

typedef struct {
    int sock;
    int data_sock;

    int data_ready;
    int is_run;

    char* buf;
    int   buf_len;
    int   buf_total;

    int   is_eof;
} sock_listen_trd_par;


void *sock_listen_trd(void *par)
{
    int ret;
    char buffer[512];

    if (!par)
        return NULL;

    sock_listen_trd_par* _ctl_struc = (sock_listen_trd_par*)par;

    int connection_socket = _ctl_struc->sock;
    if (connection_socket < 0)
        return NULL;

    _ctl_struc->data_sock = -1;

    _ctl_struc->buf       = NULL;
    _ctl_struc->buf_len   = 0;
    _ctl_struc->buf_total = 0;

    _ctl_struc->is_eof    = 0;

    memset(buffer, 0, sizeof(buffer));

    int data_socket;

    for (; _ctl_struc->is_run;) {
        data_socket = accept(connection_socket, NULL, NULL);
        _ctl_struc->is_eof = 0;
        if (data_socket > -1) {
            if (_ctl_struc->data_sock > -1)
                ::close(_ctl_struc->data_sock);

            _ctl_struc->data_sock = data_socket;
            _ctl_struc->data_ready = 0;
        }
        for (; data_socket > -1 && _ctl_struc->is_run;) {
            ret = recv(data_socket, buffer, sizeof(buffer), 0);
            if (ret < 0) {
                _ctl_struc->data_ready = 0;
                close(data_socket);
                return NULL;
            }

            if (ret == 0) {
                _ctl_struc->is_eof = 1;
                _ctl_struc->data_ready = 1;
                break;
            }

            _ctl_struc->data_ready = 0;

            while (_ctl_struc->buf_len + ret + 1 > _ctl_struc->buf_total) {
                _ctl_struc->buf_total += 512;
                _ctl_struc->buf = (char*)realloc(_ctl_struc->buf, _ctl_struc->buf_total);
            }
            memcpy(&_ctl_struc->buf[_ctl_struc->buf_len], buffer, ret);
            _ctl_struc->buf[_ctl_struc->buf_len + ret] = 0;

            if (ret < sizeof(buffer)) {
                _ctl_struc->data_ready = 1;
                //break;
            }

        }
        if (data_socket > -1)
            close(data_socket);
        _ctl_struc->data_sock = -1;

        if (!_ctl_struc->is_run) {
            break;
        }
    }    
    return NULL;
}
} // extern "C"


using namespace std;

namespace gnash
{

static const int BUFSIZE = 1024;

as_value usocketio_socket(const fn_call& fn);
as_value usocketio_close(const fn_call& fn);

as_value usocketio_connect(const fn_call& fn);
as_value usocketio_listen(const fn_call& fn);

as_value usocketio_write_clt(const fn_call& fn);
as_value usocketio_read_clt(const fn_call& fn);

as_value usocketio_read_srv(const fn_call& fn);
as_value usocketio_write_srv(const fn_call& fn);

as_value usocketio_is_eof(const fn_call& fn);

LogFile& dbglogfile = LogFile::getDefaultInstance();

// TODO: Document this class !!
class USocketIO : public Relay
{
public:
    USocketIO();
    ~USocketIO();

    bool connect(); 
    bool listen();

    bool socket(const std::string &filespec, const std::string &mode);
    int close();

    int write(const std::string &str);
    int read(std::string &str);

    int read_srv(std::string &str);
    int write_srv(const std::string &str);

    bool is_eof();

private:
    int                _stream;
    struct sockaddr_un _addr;

    string             _name;

    int                _is_serv;
    pthread_t          _trd;

    sock_listen_trd_par* _ctl_struc;

    std::string _filespec;
};

static void
attachInterface(as_object& obj)
{
    Global_as& gl = getGlobal(obj);
    
    obj.init_member("socket", gl.createFunction(usocketio_socket));
    obj.init_member("close", gl.createFunction(usocketio_close));

    obj.init_member("connect", gl.createFunction(usocketio_connect));
    obj.init_member("listen", gl.createFunction(usocketio_listen));

    obj.init_member("write", gl.createFunction(usocketio_write_clt));
    obj.init_member("read", gl.createFunction(usocketio_read_clt));

//    obj.init_member("read_srv", gl.createFunction(usocketio_read_srv));
//    obj.init_member("write_srv", gl.createFunction(usocketio_write_srv));

    obj.init_member("eof", gl.createFunction(usocketio_is_eof));
}

static as_value
usocketio_ctor(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);
    obj->setRelay(new USocketIO());

    if (fn.nargs > 0) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::stringstream ss; fn.dump_args(ss);
            log_aserror("new USocketIO(%s): all arguments discarded",
                        ss.str().c_str());
            );
    }

    return as_value();
}


USocketIO::USocketIO()
{
    _stream = -1;
    _is_serv = -1;
    _trd = (pthread_t)-1;
    _ctl_struc = NULL;
}

USocketIO::~USocketIO()
{
//    GNASH_REPORT_FUNCTION;
    if (_trd != (pthread_t)-1) {
        if (_ctl_struc)
            _ctl_struc->is_run = 0;
        ::pthread_join(_trd, 0);
        _trd = (pthread_t)-1;
    }
    if (_ctl_struc) {
        if (_ctl_struc->buf)
            ::free(_ctl_struc->buf);
        ::free(_ctl_struc);
        _ctl_struc = NULL;
    }
    close();
    _is_serv = -1;
}

bool
USocketIO::connect()
{
//    GNASH_REPORT_FUNCTION;
    if (_stream && _is_serv == -1) {
        //::strncpy(_addr.sun_path, _name.c_str(), sizeof(_addr.sun_path) - 1);
        int ret = ::connect(_stream, (const struct sockaddr *) &_addr, sizeof(_addr));
        if (ret == -1) {
            //close();
            return false;
        }
        _is_serv = 0;
    }

    return true;
}

bool
USocketIO::socket(const string &filespec, const string &mode)
{
//    GNASH_REPORT_FUNCTION;
    _stream = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (_stream == -1) {
        return false;
    }
    ::memset(&_addr, 0, sizeof(_addr));
    _addr.sun_family = AF_UNIX;

    _name = filespec;
    ::strncpy(_addr.sun_path, filespec.c_str(), sizeof(_addr.sun_path) - 1);
    return true;
}

int
USocketIO::read(string &str)
{
//    GNASH_REPORT_FUNCTION;
    int ret = -1;
    if (_is_serv == 0) {
        if (_stream) {
            char buf[BUFSIZE];
            memset(buf, 0, BUFSIZE);    
            ret = ::recv(_stream, buf, BUFSIZE, 0);
            if (ret >= 0) {
                str = buf;
            }
        }
    } else if (_is_serv == 1) {
        return read_srv(str);
    }
    return ret;
}

int
USocketIO::read_srv(string &str)
{
//    GNASH_REPORT_FUNCTION;
    int ret = -1;
    if (_stream && _is_serv == 1 && _ctl_struc && _ctl_struc->buf && _ctl_struc->data_ready) {
        char* buf = _ctl_struc->buf;
        ret = _ctl_struc->buf_len;

        _ctl_struc->buf_total = 0;
        _ctl_struc->buf_len = 0;
        _ctl_struc->buf = NULL;
        
        str = buf;
        ::free(buf);
    }
    return ret;
}

int
USocketIO::write(const string &str)
{
//    GNASH_REPORT_FUNCTION;
    if (_is_serv == 0) {
        if (_stream) {
            int ret = ::send(_stream, str.c_str(), str.size(), 0);
            if (ret == -1) {
                return -1;
            }
            return ret;
        }
    } if (_is_serv == 1) {
        return write_srv(str);
    }
    return -1;
}

int
USocketIO::write_srv(const string &str)
{
//    GNASH_REPORT_FUNCTION;
    int ret = -1;
    if (_ctl_struc && _ctl_struc->data_sock > -1) {
        ret = ::send(_ctl_struc->data_sock, str.c_str(), str.length(), 0);
        //::close(_ctl_struc->data_sock);
        //_ctl_struc->data_sock = -1;
    }    
    return ret;
}

int
USocketIO::close()
{
//    GNASH_REPORT_FUNCTION;
    if (_stream) {
        if (_ctl_struc) {
            _ctl_struc->is_run = 0;
            //::close(_ctl_struc->data_sock);
            //_ctl_struc->data_sock = -1;
        }
        int ret = ::close(_stream);
        _stream = -1;
        return ret;
    }
    return -1;
}

bool
USocketIO::listen()
{
//    GNASH_REPORT_FUNCTION;
    if (_stream && _is_serv == -1 && _trd == (pthread_t)-1) {
        unlink(_addr.sun_path);

        int ret = ::bind(_stream, (const struct sockaddr *) &_addr, sizeof(_addr));
        if (ret == -1) {
            return false;
        }
        ret = ::listen(_stream, 10);
        if (ret == -1) {
            //printf(">>>  Errno = %d: %s\n", errno, strerror(errno));
            return false;
        }

        if (_ctl_struc) {
            if (_ctl_struc->buf)
                ::free(_ctl_struc->buf);
            if (_ctl_struc->data_sock > -1)
                ::close(_ctl_struc->data_sock);
            ::free(_ctl_struc);
        }
        _ctl_struc = (sock_listen_trd_par*)::malloc(sizeof(sock_listen_trd_par));
        _ctl_struc->sock = _stream;
        _ctl_struc->data_sock = -1;

        pthread_attr_t attr;

        ::pthread_attr_init(&attr);
        ::pthread_create(&_trd, &attr, sock_listen_trd, (void*)_ctl_struc);
        
        _is_serv = 1;
        return true;
    }
    return false;
}


as_value
usocketio_socket(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    USocketIO* ptr = ensure<ThisIsNative<USocketIO> >(fn);
    assert(ptr);
    
    if (fn.nargs < 2) {
	IF_VERBOSE_ASCODING_ERRORS(
	    std::stringstream ss; fn.dump_args(ss);
	    log_aserror("USocketIO.fopen(%s): need two arguments", ss.str().c_str());
	    );
	return as_value(false);
    }

    string filespec = fn.arg(0).to_string();
    string mode = fn.arg(1).to_string();
    return as_value(ptr->socket(filespec, mode));

}

bool
USocketIO::is_eof()
{
    bool ret = true;
//    GNASH_REPORT_FUNCTION;
    if (_stream) {
        if (_ctl_struc && _ctl_struc->is_run != 0 && _ctl_struc->data_sock > -1) {
            ret = _ctl_struc->is_eof != 0;
        }
    }
    return ret;
}

as_value
usocketio_close(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    USocketIO* ptr = ensure<ThisIsNative<USocketIO> >(fn);
    assert(ptr);
    
    return as_value(ptr->close());
}

as_value
usocketio_read_clt(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    USocketIO* ptr = ensure<ThisIsNative<USocketIO> >(fn);
    assert(ptr);

    string str;
    int count = ptr->read(str);
    
    if (count < 0) {
        return as_value(false);
    } else {
        return as_value(str.c_str());
    }
}

as_value
usocketio_read_srv(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    USocketIO* ptr = ensure<ThisIsNative<USocketIO> >(fn);
    assert(ptr);

    string str;
    int count = ptr->read_srv(str);

    if (count < 0) {
        return as_value(false);
    } else {
        return as_value(str.c_str());
    }
}

as_value
usocketio_write_clt(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    USocketIO* ptr = ensure<ThisIsNative<USocketIO> >(fn);
    assert(ptr);
    string str = fn.arg(0).to_string();
    return as_value(ptr->write(str));
}

as_value
usocketio_write_srv(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    USocketIO* ptr = ensure<ThisIsNative<USocketIO> >(fn);
    assert(ptr);    
    string str = fn.arg(0).to_string();
    return as_value(ptr->write_srv(str));
}

as_value 
usocketio_connect(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    USocketIO* ptr = ensure<ThisIsNative<USocketIO> >(fn);
    assert(ptr);
    //bool b = toBool(fn.arg(0), getVM(fn));
    return as_value(ptr->connect());
}

as_value
usocketio_listen(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    USocketIO* ptr = ensure<ThisIsNative<USocketIO> >(fn);
    assert(ptr);
    return as_value(ptr->listen());
}

as_value
usocketio_is_eof(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    USocketIO* ptr = ensure<ThisIsNative<USocketIO> >(fn);
    assert(ptr);
    return as_value(ptr->is_eof());
}

extern "C" {

void
usocketio_class_init(as_object& where, const ObjectURI& /* uri */)
{
    //	GNASH_REPORT_FUNCTION;
    Global_as& gl = getGlobal(where);

    as_object* proto = createObject(gl);
    attachInterface(*proto);
    as_object* cl = gl.createClass(&usocketio_ctor, proto);
    
    where.init_member("USocketIO", cl);
}
} // end of extern C


} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
