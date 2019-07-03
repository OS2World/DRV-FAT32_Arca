This is the initial release of the Arca Noae FAT32 driver for OS/2 and
derivative operating systems. This distribution provides a native FAT32
filesystem driver (IFS), allowing read/write access to files and directories
created by other operating systems, namely, Microsoft Windows.

CAPABILITIES

Read, write, create, delete, check, format, label, rename.

REQUIREMENTS

All required libraries are included in the package. No additional supporting
files are required beyond what is generally available in a base OS/2 Warp 4
distribution.

The driver should install and work on OS/2 Warp 4 FixPak 13 or higher,
eComStaton 1.x, 2.x, and ArcaOS 5.x.

For installation, WarpIN 1.0.19 or higher is required.

KNOWN ISSUES AND PROGRAM LIMITATIONS

See the wiki at: https://www.arcanoae.com/wiki/fat32 for the latest
information.


HISTORY

v5.0.0
- Initial release of the Arca Noae fork, based on Netlabs trunk r359.

NOTICES

Arca Noae FAT32 Filesystem Driver
(c) 2018 Arca Noae, LLC

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, version 2.1 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.


Source code is available via Subversion or on the World Wide Web
at: https://svn.arcanoae.com/fat32/


The original source code was written by Henk Kelder <henk.kelder@cgey.nl>.

Henk kindly gave the copyright of the source code to netlabs.org in
January 2002.

OS/2 Device Driver Kit Copyright (c) 1992, 2000-2001 IBM Corporation
All rights reserved.

FORMAT is based on Fat32Format, (c) Tom Thornhill
GPL (no strict version)
http://www.ridgecrop.demon.co.uk/index.htm?fat32format.htm

QEMUIMG.DLL is based on QEMU code, (c) Fabrice Bellard and contributors
GPLv2
http://www.qemu.org

zlib is (c) Mark Adler, Jean-loup Gailly
BSD 3-clause license

Valery V. Sedletski <_valerius@mail.ru>
http://osfree.org
