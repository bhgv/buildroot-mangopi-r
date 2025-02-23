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

#ifndef GNASH_FILEIO_PLUGIN_H
#define GNASH_FILEIO_PLUGIN_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif


namespace gnash {

struct ObjectURI;
class as_object;

extern "C" {

void usocketio_class_init(as_object& global, const ObjectURI& uri);  

}

} // end of gnash namespace

// __FILEIO_PLUGIN_H__
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:

/*
echo "set EnableExtensions on" > ~/.gnashrc
export GNASH_PLUGINS=/usr/lib/gnash/plugins
cd /opt
gnash *&

chmod 777 /tmp/gn_tst.sck
echo 787 | nc -uU /tmp/gn_tst.sck
socat - UNIX-CLIENT:/tmp/gn_tst.sck
*/
