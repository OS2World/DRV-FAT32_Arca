:userdoc.
:docprof toc=123456.
:title.FAT32.IFS

.** Note to translators
.* The main purpose of .nameit symbols is to be able to change key terms (e.g.
.* for work in progress, or re-branding) without the need for extensive work
.* on the rest of the text. However, replacing a symbol with some fixed value
.* may not be adequate everywhere in some languages because of singular/plural
.* differences, grammatical cases, genders, etc. If that is the case, it is
.* recommended that you stick to using symbols so as not to defeat their
.* purpose, but add additional ones, as strictly needed, with some sort of
.* system as in the examples below. Use of one symbol or another to get the
.* most appropriate wording in your language is entirely at your own
.* discretion.
.*
.* Examples:
.* .nameit symbol='set_a' value='Just nuts'
.* .nameit symbol='set_b' value='Bolts only'
.* .nameit symbol='set_c' value='Nuts and bolts'
.*
.* .nameit symbol='fp1' value='this fine product'
.* .nameit symbol='fpn' value='these fine products'
.*
.* Please do not translate the license.
.* The FSF does not approve license translations as officially valid. The
.* reason is that checking them would be difficult and expensive (needing the
.* help of bilingual lawyers in other countries).
.*
.* See https://www.gnu.org/licenses/translations.en.html
.*
.* Feel free to add that link above the .im tag with whatever descriptive text
.* might be appropriate for your language. Alternatively, provide a direct link
.* to the selected translation, but please do not incorporate such translations
.* in this file or replace the gpl3.ipf. See suggested text in License, below.
.**

.nameit symbol=an text='Arca Noae'
.nameit symbol=aos text=ArcaOS
.nameit symbol=aosup text=ARCAOS
.nameit symbol=os2 text=OS/2
.nameit symbol=ecs text=eComStaton
.nameit symbol=f32 text=FAT32
.nameit symbol=nl text=Netlabs
.* rev 2018-08-10-001
.* ----------------------------------------------------------------------------
:h1 res=001 name=intro
    x=left y=bottom width=100% height=100%.About the &an. &f32. filesystem driver

:p.The :hp2.&an. &f32. filesystem driver:ehp2. for &os2. and derivative
operating systems is based on the &nl. &f32. project, but with several important
differences, namely that this distribution focuses solely on providing a native
&f32. filesystem driver (IFS) and not any type of "universal FAT replacement"
driver. As such, no FAT12/16/VFAT or exFAT support is provided by the
:hp2.&an. &f32. filesystem driver:ehp2.. Additionally, a number of older and/or
extraneous files have been omitted from this distribution.

:p.This guide is meant as an overview to using the
:hp2.&an. &f32. filesystem driver:ehp2. and addresses the following main topics
&colon.

:p.:ul compact.
:li.:link reftype=hd res=002.License:elink.
:li.:link reftype=hd res=005.Acknowledgements:elink.
:li.:link reftype=hd res=010.Installaton:elink.
:li.:link reftype=hd res=040.Components:elink.
:li.:link reftype=hd res=060.&f32. filesystem:elink.
:li.:link reftype=hd res=090.Types of &f32.-formatted media:elink.
:li.:link reftype=hd res=800.Solving problems:elink.
:li.:link reftype=hd res=900.References:elink.
:eul.
:p.

.* ----------------------------------------------------------------------------
:h2 res=002 name=license
    x=left y=bottom width=100% height=100%.License

.** Please feel free to uncomment and translate the following note in non-English versions
.*
.* Note to <your language> speakers:
.*
.* FSF does not allow official use of license translations due to verification
.* costs, and also to prevent legal harm as a possible consequence of the use
.* of unverified translations. As a simple, non-legally binding convenience for
.* you, an on-line version of the GPLv3 in your language is linked from here.
.*
.* A <your language> version of the following text can be read at <link>. This
.* is the original, legally binding LGPLv2.1: 
.**
.im lgpl21.ipf

.* ----------------------------------------------------------------------------
:h2 res=005 name=credits
    x=left y=bottom width=100% height=100%.Acknowledgements

:p.We wish to thank the following people for their contributions to this
project&colon.

:dl tsize=30 break=none.
:dt.:hp2.Henk Kelder:ehp2.
:dd.original author 
:dt.:hp2.Brian Smith:ehp2.
:dd.contributor 
:dt.:hp2.KO Myung-Hun:ehp2.
:dd.main developer for the &nl. &f32. IFS driver 
:dt.Lars Edmann
:dd.developer
:dt.:hp2.Alfredo Fern ndez D¡az:ehp2.
:dd.creator of the &nl. WarpIN Script for &f32. and contributor 
:dt.:hp2.David Graser:ehp2.
:dd.creator of the original &f32..INF file
:dt.:hp2.Valery V. Sedletski:ehp2.
:dd.&nl. &f32. IFS version 0.10 developer
:dt.:hp2.Andy Willis:ehp2.
:dd.creator of the &an. fork of the &nl. project
:dt.:hp2.Gregg Young:ehp2.
:dd.developer/contributor to the &an. fork
:dt.:hp2.Steven Levine:ehp2.
:dd.developer/contributor to the &an. fork
:dt.:hp2.Lewis Rosenthal:ehp2.
:dd.documentation for the &an. fork
:dt.:hp2.Jan van Wijk:ehp2.
:dd.technical (and other) proofreading of this guide
:edl.
:p.

.* ----------------------------------------------------------------------------
:h1 res=010 name=installation
    x=left y=bottom width=100% height=100%.Installation

:p.This section addresses the following topics&colon.

:p.:ul compact.
:li.:link reftype=hd res=012.Distribution:elink.
:li.:link reftype=hd res=014.Prerequisites:elink.
:li.:link reftype=hd res=016.First-time installaton:elink.
:li.:link reftype=hd res=030.De-installation:elink.
:li.:link reftype=hd res=036.Updating the &an. &f32. filesystem driver:elink.
:eul.
:p.

.* ----------------------------------------------------------------------------
:h2 res=012 name=distribution
    x=left y=bottom width=100% height=100%.Distribution

:p.The :hp2.&an. &f32. filesystem driver:ehp2. is distributed as a US English
WarpIN package, provided with &aos. and available as part of an &an. software
subscription. See the &an. website for more details&colon.

:p.https&colon.//www.arcanoae.com/

:nt.When provided as part of a subscription, the WarpIN package is a self-
extracting executable.
:ent.
:p.

.* ----------------------------------------------------------------------------
:h2 res=014 name=prerequisites
    x=left y=bottom width=100% height=100%.Prerequisites

:p.The :hp2.&an. &f32. filesystem driver:ehp2. has been designed to install on
as broad a range of systems as possible. The minimum system requirements
are&colon.

:ul compact.
:li.&os2. Warp 4 FixPak 13 or higher, &ecs. 1.x, 2.x, &aos. 5.x.
:li.WarpIN 1.0.19 or higher
:eul.
:p.

.* ----------------------------------------------------------------------------
:h2 res=016 name=first-time
    x=left y=bottom width=100% height=100%.First-time installation

:p.Start the installer and follow the prompts. When the installation is
complete, reboot to load the &f32. filesystem driver.

:p.Several optional components are included in the package. Please see the
relevant sections of this guide for a description of each of these.

:caution.
For systems with the &nl. &f32. filesystem driver currently installed, it is
necessary to first uninstall (de-install) that driver (usually via WarpIN)
before installing the :hp2.&an. &f32. filesystem driver:ehp2.. This is due to
conflicting filenames and the need to not mix versions between distributions.
Further, if the &nl. &f32. filesystem driver is uninstalled (de-installed) via
WarpIN :hp1.after:ehp1. the installation of the :hp2.&an. &f32. filesystem
driver:ehp2., files from the latter package may be unintentionally deleted,
resulting in a broken installation, and possibly, a non-bootable system.
:ecaution.
:p.

.* ----------------------------------------------------------------------------
.* :h3 res=020 name=optional_components
.*     x=left y=bottom width=100% height=100%.Optional components
.* 
.* :p.Components included in the :hp2.&an. &f32. filesystem driver:ehp2.
.* distribution but not automatically selected for installation are discussed here.
.* 
.* ----------------------------------------------------------------------------
.** Note concerning Korean support
.* These files are severely outdated and in need of review prior to shipment
.* and making them available for installation. When this work is completed,
.* this section should be uncommented and reworked, accordingly.
.* 
.* The following text should not be translated until the English is rewritten.
.*
.* :h3 res=020 name=korean
.*     x=left y=bottom width=100% height=100%.Korean Support for non-Korean Systems
.*
.* :p.In addition to the &f32. files and necessary changes to CONFIG.SYS,
.* selecting this package will install COUNTRY.KOR, an alternative COUNTRY.SYS
.* driver, but will :hp1.not:ehp1. modify CONFIG.SYS to utilize it.
.* 
.* :p.To enable Korean support, locate the following line in CONFIG.SYS&colon.
.* 
.* You have to do so manually, and in case of problems, the fallback Alt+F1,F2 
.* solution will still point to the original COUNTRY.SYS and thus will be safe. See :link reftype=hd refid=41.
.* FILES IN THIS VERSION):elink.. It also 
.* 
.* :p.adds a Korean text dealing with FAT32 information. 
.* 
.* :p.copies a CACHEF32.CMD file to the hard drive which contains the 
.* following script&colon. 
.* 
.* :p.-chcp 949 
.* 
.* :p.-CACHEF32.EXE /y   
.* 
.* :p.adds the following entry to the startup.cmd file found in the root 
.* directory 
.* 
.* :p.-cmd /C CACHEF32.CMD 
.* 
.* :p.:hp2.Note&colon.  :ehp2.The reason cmd /c is used is because 4os2 does not support CHCP 
.* correctly. 
.* .br 
.* 
.* :p.:hp2.IMPORTANT!  :ehp2.If you install Korean support for non-Korean systems, you&apos.ll 
.* have to manually edit your CONFIG.SYS and change the COUNTRY settings from 
.* 
.* :p.COUNTRY=xyz,?&colon.&bsl.OS2&bsl.SYSTEM&bsl.COUNTRY.SYS 
.* 
.* :p.to 
.* 
.* :p.COUNTRY=xyz,<Install Path>&bsl.COUNTRY.KOR 
.* 
.* :p.where you have to substitute xyz and <Install Path> with appropriate values
.* . In case of problems, the fallback Alt+F1,F2 solution will still point to the 
.* original COUNTRY.SYS and thus will be safe. See :link reftype=hd refid=41.&osq.Files in this version&osq.:elink.
.* . 
.* 
.* :p.:hp2.Note&colon.  :ehp2.Korean filenames are displayed correctly only in CP949, but 
.* filenames are manipulated correctly regardless of current codepage.   
.**
:p.

.* ----------------------------------------------------------------------------
:h2 res=030 name=de-installation
    x=left y=bottom width=100% height=100%.De-installation

:p.If the :hp2.&an. &f32. filesystem driver:ehp2. was previously installed using
WarpIN, follow this procedure to remove it from the system entirely&colon.

:dl tsize=3 break=none.
:dt.1.
:dd.Start WarpIN.
:dt.2.
:dd.Under the :hp1.Application:ehp1. column, find the first :hp2.&f32.:ehp2.
entry in the list, and confirm that the :hp1.Vendor:ehp1. column states
:hp2.&an.:ehp2..
:dt.3.
:dd.Select one or more :hp2.&an. &f32. filesystem driver:ehp2. packages to de-install.
:dt.4.
:dd.Select :hp1.Package:ehp1. | :hp1.De-install package...:ehp1. from the main
menu.  A confirmation should be presented, advising as to the changes which
will be made to the system.
:dt.5.
:dd.Select :hp2.OK:ehp2. to begin the process.
:nt.It is likely that you will encounter locked files during the process
(typically, the cache daemon will be loaded and running). Select the option to
unlock these files and remove them.:ent.
:dt.6.
:dd.When the WarpIN procedure is complete, and if you chose to allow the
changes to CONFIG.SYS to be made, open CONFIG.SYS in a text editor to adjust the
PATH and LIBPATH statements.
:edl.

:p.Reboot the system.
:p.

.* ----------------------------------------------------------------------------
:h2 res=036 name=updating
    x=left y=bottom width=100% height=100%.Updating the driver

:p.Updating the :hp2.&an. &f32. filesystem driver:ehp2. is a simple matter of
downloading the latest installation package from Arca Noae and executing the
self-extracting package.
:p.

.* ----------------------------------------------------------------------------
:h1 res=040 name=contents
    x=left y=bottom width=100% height=100%.FAT32 Components

:p.This distribution of the :hp2.&an. &f32. filesystem driver:ehp2. includes the
following files&colon.

:dl tsize=22 break=none.
:dt.:hp2.fat32.inf:ehp2.
:dd.This file.
:dt.:hp2.FAT32.IFS:ehp2.
:dd.:link reftype=hd res=042.Installable filesystem driver:elink..
:dt.:hp2.CACHEF32.EXE:ehp2.
:dd.:link reftype=hd res=043.Cache daemon:elink..
:dt.:hp2.UFAT32.DLL:ehp2.
:dd.:link reftype=hd res=044.Utility DLL:elink. to support disk checking and
formatting routines.
:dt.:hp2.QEMUIMG.DLL:ehp2.
:dd.Support DLL for accessing disk images (to be removed at a later date).
:dt.:hp2.FAT32CHK.EXE:ehp2.
:dd.Standalone &f32. :link reftype=hd res=045.disk check utility:elink..
:dt.:hp2.F32CHK.EXE:ehp2.
:dd.Disk check helper for :link reftype=hd res=045.autocheck:elink. during
system startup.
:dt.:hp2.F32MON.EXE:ehp2.
:dd.&f32. :link reftype=hd res=050.activity monitor:elink..

.** These files require review. References here are placeholders.
.* :dt.:hp2.COUNTRY.KOR:ehp2.
.* :dd. A patched version of the &osq.country.sys&osq. from WSEB fixpak 2.
.* Strangely, &osq.country.sys&osq. of WSeB contains the wrong information for
.* Korean, especially DBCS lead byte infos. Without this, Korean cannot use
.* filenames consisting of Korean chars.  To use, rename the
.* &osq.country.sys&osq. found in OS2&bsl.SYSTEM directory and replace it with
.* the &osq.country.kor&osq., renaming it &osq.country.sys&osq.. 
.* :dt.:hp2.CACHEF32.CMD:ehp2.
.* :dd.Script for loading Korean Codepage and CACHEF32.EXE
.**
:edl.

.* ----------------------------------------------------------------------------
:h2 res=042 name=fat32_ifs
    x=left y=bottom width=100% height=100%.FAT32 IFS

:p.This is the installable filesystem (IFS) driver itself. Its syntax is
&colon.   
:cgraphic.


IFS =  ÄÄÂÄÄÄÄÄÄÄÄÄÂÄÂÄÄÄÄÄÄÄÄÂÄ FAT32.IFS ÄÄÂÄÄÄÄÄÄÄÄÄÄÄÂÄ´
         ÀÄ drive ÄÙ ÀÄ path ÄÙ              ÀÄ options ÄÙ


:ecgraphic.

:p.Supported options for the :hp2.&an. &f32. filesystem driver:ehp2. are
:hp1.not:ehp1. case-sensitive and may appear in any order.

:dl tsize=24 break=none.
:dt.:hp2./AC&colon.:ehp2.:hp3.volumes:ehp3.:hp2.|*:ehp2.
:dd.(May also be specified as :hp2./AUTOCHECK:ehp2..) Specifies the &f32.
volumes (by drive letter) to be automatically checked during system start. If
:hp2./AC&colon.*:ehp2. is specified, CHKDSK will examine each &f32. volume for
the presence of a dirty flag. If set, CHKDSK will be run against that volume.

:p.:hp2.Example&colon.:ehp2.

:xmp.
    :hp2./AC&colon.qrst:ehp2.
:exmp.

:dt.:hp2./CACHE&colon.:ehp2.:hp3.nnnn:ehp3.
:dd.Specifies the cache size in kilobytes. If omitted, the default cache size
is 1024KB. Maximum cache size is 2048KB. Note that read and write caching
requires the cache daemon to be loaded and active.
:dt.:hp2./EAS:ehp2.
:dd.Support extended attributes (EAs) on all &f32. volumes (this is a global
option). The default is to :hp1.not:ehp1. support extended attributes on &f32.
volumes for reading or writing.

:nt.Before adding EA support to a volume which did not previously have any
extended attributes stored on it, it is necessary to run CHKDSK /F against it
(or select the context menu item :hp1.Check disk...:ehp1. for the drive object
associated with the &f32. volume and check the :hp1.Write corrections to
disk:ehp1. box).
:ent.

:dt.:hp2./H:ehp2.
:dd.Allocates cache memory into memory space above 16MB.
:dt.:hp2./LARGEFILES:ehp2.
:dd.Support large files (size &gt.2GB). The &f32. maximum file size as
specified is 4GB. This option is recommended for &aos. and &ecs.
:dt.:hp2./MONITOR:ehp2.
:dd.Start collecting data for :link reftype=hd res=050.F32MON:elink. at IFS
load time.

:nt.The internal buffer for monitoring is limited to 4096 bytes. Thus, in order
to record boot activity, :link reftype=hd res=050.F32MON:elink. should be
started as soon as possible (from STARTUP.CMD, etc.).
:ent.

:dt.:hp2./Q:ehp2.
:dd.Quiet; the startup banner and messages are not displayed.
:dt.:hp2./RASECTORS&colon.:ehp2.:hp3.nnn:ehp3.
:dd.Specifies the minimum number of
:link refid=ras reftype=fn.Read Ahead Sectors:elink. (maximum 128). Applies to
all &f32. volumes.
:edl.

:p.:hp2.Example&colon.:ehp2.

:p.By default, the :hp2.&an. &f32. filesystem driver:ehp2. allocates a cache
size of 2048KB, loads quietly into memory above 16MB, autochecks all
available &f32. volumes during boot, and supports files up to the &f32. 4GB
limit&colon.

:xmp.
    IFS=&lt.path&gt.\FAT32.IFS /cache&colon.2048 /h /q /ac&colon.* /largefiles
:exmp.
:p.

.* ----------------------------------------------------------------------------
:h2 res=043 name=cachef32
    x=left y=bottom width=100% height=100%.CACHEF32

:p.This is a daemon process which provides the following services&colon. 

:ul compact.
:li.Start the :link refid=lazywrite reftype=fn.lazy write:elink. daemon.
:li.Set cache and read ahead parameters. 
:li.Load a codepage for the
:link reftype=hd res=074.Unicode translate table:elink. for longname support. 
:li.Display the status of the running daemon.
:eul.

:p.Supported :hp2.CACHEF32:ehp2. options are :hp1.not:ehp1. case-sensitive and
may appear in any order.

:dl tsize=18 break=none.
:dt.:hp2./?:ehp2.
:dd.Displays command line help. 
:dt.:hp2./Q:ehp2.
:dd.Terminates (unloads) the :hp2.CACHEF32:ehp2. daemon.
:dt.:hp2./N:ehp2.
:dd.Runs :hp2.CACHEF32:ehp2. and terminates, without starting the daemon.
:dt.:hp2./D&colon.:ehp2.:hp3.nnnnnn:ehp3.
:dd.Sets the :link refid=diskidle reftype=fn.DISKIDLE:elink. value.
:dt.:hp2./B&colon.:ehp2.:hp3.nnnnnn:ehp3.
:dd.Sets the :link refid=bufferidle reftype=fn.BUFFERIDLE:elink. value.
:dt.:hp2./M&colon.:ehp2.:hp3.nnnnnn:ehp3.
:dd.Sets the :link refid=maxage reftype=fn.MAXAGE:elink. value.
:dt.:hp2./R&colon.:ehp2.:hp3.d&colon.:ehp3.:hp2.,:ehp2.:hp3.nnn:ehp3.
:dd.Specifies the minimum number of
:link refid=ras reftype=fn.Read Ahead Sectors:elink. (maximum 128) for drive
:hp1.d&colon.:ehp1. to :hp1.nnn:ehp1.. 
:dt.:hp2./L&colon.ON|OFF:ehp2.
:dd.Sets :link refid=lazywrite reftype=fn.lazy writing:elink. ON or OFF
(default is ON).
:dt.:hp2./P&colon.1|2|3|4:ehp2.
:dd.Sets the priority for the lazy writer. 1 is lowest (idle, the default); 4
is highest (foreground server). Performance generally increases with higher
priority, but at the expense of overall system response. 1 or 2 are typical
values.
:dt.:hp2./Y:ehp2.
:dd.Assume yes for any options requiring a response.
:dt.:hp2./S:ehp2.
:dd.Do not display normal startup messages (Silent mode).
:dt.:hp2./CP&colon.:ehp2.:hp3.cp:ehp3.
:dd.Load specified codepage for the
:link reftype=hd res=074.Unicode translate table:elink. for longname support.

:caution.
Do not change codepage if open files exist with filenames containing
extended ASCII characters as data loss may occur.
:ecaution.

:dt.:hp2./F:ehp2.
:dd.Forces :hp2.CACHEF32:ehp2. to be loaded even if no &f32. partition is found
when the daemon is started. This is useful when the only &f32. volumes are
removable and not attached when the daemon is started.
:edl.

:p.:hp2.Example&colon.:ehp2.

:p.By default, the :hp2.&an. &f32. filesystem driver:ehp2. forces the
:link refid=lazywrite reftype=fn.lazy write daemon:elink. to start at regular
priority, sets :link refid=maxage reftype=fn.MAXAGE:elink.&colon. to 50,000ms,
:link refid=bufferidle reftype=fn.BUFFERIDLE:elink. to 250ms, and
:link refid=diskidle reftype=fn.DISKIDLE:elink. to 5000ms in CONFIG.SYS, which
makes these settings apply to the entire system, persistent across reboots
&colon.

:xmp.
    CALL=&lt.path&gt.\CACHEF32.EXE /f /p&colon.2 /m&colon.50000 /b&colon.250 /d&colon.5000
:exmp.
:p.


.* ----------------------------------------------------------------------------
:h2 res=044 name=ufat32
    x=left y=bottom width=100% height=100%.UFAT32

:p.This is the &f32. filesystem utility DLL, which provides support for disk
checking and formatting.

.* ----------------------------------------------------------------------------
:h3 res=045 name=chkdsk
    x=left y=bottom width=100% height=100%.Disk checking

:p.The :hp2.UFAT32:ehp2. disk checking routine is loaded by &os2.'s CHKDSK.COM
when a disk check is requested for a &f32. volume.

:p.Supported options and limitations for checking &f32. volumes&colon.

:dl tsize=8 break=none.
:dt.:hp2./F:ehp2.
:dd.Attempt to fix problems which are identified. (Currently, :hp2.UFAT32:ehp2.
only fixes lost clusters and incorrect free space count.)
:dt.:hp2./C:ehp2.
:dd.Convert lost clusters to files automatically if the volume was in an
inconsistent state at the time of system boot (without confirmation).
:dt.:hp2./V&colon.1|2:ehp2.
:dd.Show fragmented files (1) or (2), show details for all files checked.
(Default is 2.)
:dt.:hp2./P:ehp2.
:dd.Write additional messages to stdout. This is useful when a PM application
(PMCHKDSK) is being used, which then parses the output and updates a progress
bar or other control(s) with realtime status.
:edl.
:p.

.* ----------------------------------------------------------------------------
:h4 res=046 name=chkdsk_functions
    x=left y=bottom width=100% height=100%.Disk check operation

:p.The disk check process performs the following operations&colon.

:ul compact.
:li.Compares all copies of the FATs.
:li.Checks the file allocation for each file.
:li.Displays, and if /F is specified, attempts to repair lost clusters. 
:li.Displays, and if /F is specified, attempts to repair cross-linked files. 
:li.Checks free space, and if /F is specified, attempts to correct an incorrect
value. 
:li.Checks for lost extended attributes (if /EAS option is specified for
:link reftype=hd res=042.&f32. IFS:elink.).
:eul.

:p.Two ancillary executables are included and used by &os2.'sCHKDSK&colon.

:dl tsize=16 break=none.
:dt.:hp2.FAT32CHK:ehp2.
:dd.Standalone &f32. :link reftype=hd res=045.disk check utility:elink..
:dt.:hp2.F32CHK:ehp2.
:dd.Disk check helper for :link reftype=hd res=042.autocheck:elink. during
system startup.
:edl.

:p.Both applications, when run standalone, accept the same options as &os2.'s
CHKDSK.
:p.

.* ----------------------------------------------------------------------------
:h4 res=047 name=shutdown
    x=left y=bottom width=100% height=100%.Clean vs dirty shutdown

:p.Properly shutting down an &os2. system clears the dirty flag on each
mounted volume, resulting in a normal boot when the system is restarted. When a
problem occurs which makes it impossible to shut down cleanly (power, hardware
failure, system hang), the dirty flag is left set, resulting in either an
automatic disk check (if so configured for the given installable filesystem
driver) :hp1.or:ehp1. the inability to mount the volume after boot until
checked.

:p.If CHKDSK is unable to correct problems found on a corrupt &f32. filesystem,
the dirty flag is left set, and the &f32. filesystem driver will deny access to
all files and directories on the volume. In such cases, it may be necessary to
check the volume from Windows in order to correct the problems and clear the
dirty flag.
:p.

.* ----------------------------------------------------------------------------
:h3 res=048 name=format
    x=left y=bottom width=100% height=100%.Formatting

:p.The UFAT32 format routine is called by &os2.'s FORMAT.COM with the
/FS&colon.&f32. option or when &f32. is selected from the PMFORMAT dropdown
list under &aos.

:p.Supported options for formatting &f32. volumes (these are command line
parameters for FORMAT.COM)&colon.

:dl tsize=22 break=none.
:dt.:hp2./?:ehp2.
:dd.Displays command line help. 
:dt.:hp2./R&colon.:ehp2.:hp3.nnnn:ehp3.
:dd.Number of reserved sectors.
:dt.:hp2./V&colon.:ehp2.:hp3.volume label:ehp3.
:dd.Set volume label (11 characters maximum).
:dt.:hp2./P:ehp2.
:dd.Write additional messages to stdout. This is useful when a PM application
(PMFORMAT) is being used, which then parses the output and updates a progress
bar or other control(s) with realtime status.
:dt.:hp2./FS&colon.&f32.:ehp2.
:dd.Format using the &f32. filesystem.
:dt.:hp2./C&colon.:ehp2.:hp3.nnnn:ehp3.
:dd.Specify the :link refid=cluster_size reftype=fn.cluster size:elink. in KB.
:edl.
:p.

.* ----------------------------------------------------------------------------
:h2 res=050 name=f32mon
    x=left y=bottom width=100% height=100%.F32MON

:p.This command line utility will log (most) &f32. actions to the screen and to
&f32..LOG in the current directory. :hp2.F32MON:ehp2. will degrade performance
while active, and is meant to be used as a troubleshooting aid only.

:p.When the :hp2./MONITOR:ehp2. option is specified for
:link reftype=hd res=042.&f32. IFS:elink., :hp2.F32MON:ehp2. must be started to
log (and optionally display) the captured data. The IFS's internal buffer is
4096 bytes, and old data will be lost without :hp2.F32MON:ehp2..

:p.Syntax&colon.

:p.:hp2.F32MON [:ehp2.:hp3.tracemask:ehp3.:hp2.] [/S]:ehp2.

:note.:hp3.tracemask:ehp3. defaults to 1. 

:p.Supported :hp3.tracemask:ehp3. values are&colon.

:dl tsize=4 break=none.
:dt.:hp2.1:ehp2.
:dd.Show all calls to FS_:hp1.xxxxx:ehp1. entry points and their return values.
:dt.:hp2.2:ehp2.
:dd.Show additional (internal) function calls.
:dt.:hp2.4:ehp2.
:dd.Show cache-related (:link refid=lazywrite reftype=fn.lazy write:elink.)
function calls.
:dt.:hp2.8:ehp2.
:dd.Show internal memory handling (malloc &amp. free) memory calls.
:dt.:hp2.16:ehp2.
:dd.Show FS_FINDFIRST/FS_FINDNEXT calls.
:dt.:hp2.32:ehp2.
:dd.Show additional calls.
:dt.:hp2.64:ehp2.
:dd.Show extended attribute handling.
:edl.

:p.The above values may be summed to request multiple groups.

:p.:hp2.Example&colon.:ehp2.

:p.To display both FS_:hp1.xxxxx:ehp1. calls and cache-related information,
use a tracemask value of 5 (1 + 4)&colon.

:xmp.
    F32MON 5
:exmp.

:p.:hp2.F32MON:ehp2. supports one additional parameter&colon.

:dl tsize=4 break=none.
:dt.:hp2./S :ehp2.
:dd.Silent mode. No output sent to the screen, only to the FAT32.LOG.
:edl.

:nt.To terminate :hp2.F32MON:ehp2., press Ctrl-C in the session where it is
running. FAT32.LOG will be closed and may be copied or moved as necessary.
:ent.
:p.

.* ----------------------------------------------------------------------------
:h1 res=060 name=filesystem
    x=left y=bottom width=100% height=100%.&f32. filesystem

:p.The :hp2.&f32. filesystem:ehp2. was developed by Microsoft for Windows 95
OSR2. Since its inception, &f32. has become a well known and much used
filesystem for removable media as well as a logical choice for sharing data
between different operating systems in multiboot configurations.
:p.

.* ----------------------------------------------------------------------------
:h2 res=062 name=fat32_general
    x=left y=bottom width=100% height=100%.General considerations

:p.&f32. has the following volume and filesystem characteristics&colon.

:nt.See :link reftype=hd res=070.&f32. under &os2.:elink. for &os2.-specific
limitations and differences.
:ent.

:dl tsize=24 break=none.
:dt.:hp2.Maximum
.br
volume size:ehp2.
:dd.2TB (512-byte sectors); 8TB (2KB sectors / 32KB clusters); 16TB (4KB
sectors / 64KB clusters)
:dt.:hp2.Maximum
.br
file size:ehp2.
:dd.4GB - 1 byte
:dt.:hp2.File size
.br
granularity:ehp2.
:dd.1 byte
:dt.:hp2.Maximum
.br
number of files:ehp2.
:dd.268,173,300 (32KB clusters)
:dt.:hp2.Maximum
.br
filename length:ehp2.
:dd.255
:dt.:hp2.Maximum
.br
directory depth:ehp2.
:dd.211 levels (OS dependent)
:dt.:hp2.Timestamps
.br
stored:ehp2.
:dd.last access date, modification date/time, creation date/time
:dt.:hp2.Date range:ehp2.
:dd.1980-01-01 to 2099-12-31
:dt.:hp2.Date/time
.br
resolution:ehp2.
:dd.2 seconds (last modified), 10ms (creation), 1 day (access)
:dt.:hp2.DOS Attributes:ehp2.
:dd.Read-only, Hidden, System, Volume, Directory, Archive
:dt.:hp2.Case sensitive:ehp2.
:dd.No
:dt.:hp2.Case retentive:ehp2.
:dd.Yes
:dt.:hp2.Symlink support:ehp2.
:dd.No
:dt.:hp2.Undelete support:ehp2.
:dd.Operating system specific (Trashcan, etc.).
:edl.
:p.

.* ----------------------------------------------------------------------------
:h2 res=070 name=fat32_os2
    x=left y=bottom width=100% height=100%.&f32. under &os2.

:p.Utilizing the :hp2.&an. &f32. filesystem driver:ehp2. for &os2., the
following volume and filesystem characteristics apply&colon.

:nt.See :link reftype=hd res=060.&f32. general considerations.:elink. for
&f32. characteristics under Windows.
:ent.

:dl tsize=26 break=none.
:dt.:hp2.Maximum
.br
volume size:ehp2.
:dd.2TB (&os2. volume size limit)
:dt.:hp2.Timestamps
.br
stored:ehp2.
:dd.last access date, modification date/time, creation date/time
:dt.:hp2.Supports
.br
Extended Attributes:ehp2.
:dd.Yes, through the use of &odq.:hp1. EA. SF:ehp1.&cdq. files and the
:link refid=mark_byte reftype=fn.mark byte:elink..
:dt.:hp2.Maximum length of
.br
Extended Attributes:ehp2.
:dd.2GB - 1 byte (stored as regular file; the
:link refid=mark_byte reftype=fn.mark byte:elink. consumes 1 byte in the
directory entry for the file)
:dt.:hp2.Supports booting:ehp2.
:dd.No
:dt.:hp2.Supports swapfile
.br
(SWAPPER.DAT):ehp2.
:dd.No
:dt.:hp2.Supports CHKDSK:ehp2.
:dd.Limited; repair only possible for lost clusters, incorrect free space
count, and cross-linked files.
:dt.:hp2.Long filenames
.br
in DOS and WinOS/2
.br
sessions:ehp2.
:dd.No; filenames are converted to their short forms (8.3 format, with a tilde
&odq.&tilde.&cdq. and sequence number used to represent a shortened long name).
See :link reftype=hd res=082.DOS and WinOS/2:elink. for details.
:dt.:hp2.kLIBC symlink
.br
support:ehp2.
:dd.No
:dt.:hp2.Undelete support:ehp2.
:dd.Yes, via DELETE directory.
:edl.
:p.

.* ----------------------------------------------------------------------------
:h3 res=074 name=char_sets
    x=left y=bottom width=100% height=100%.Character sets

:p.&os2. utilizes a system of :hp2.codepages:ehp2. to describe human-readable
characters on the screen and as used in filesystems. Each codepage consists of
an encoding table which translates bytes into human-readable characters. The
computer uses whichever codepage is currently active to determine what
character a given byte value represents. Each codepage is referenced by a
specific number, e.g., 850. The first codepage listed on the
:hp2.CODEPAGE=:ehp2. line in CONFIG.SYS is the primary codepage, and is
generally the one in use most of the time.

:p.The secondary codepage (listed after the comma on the :hp2.CODEPAGE=:ehp2.
line in CONFIG.SYS) is an optional alternate codepage which may be activated on
a per-process basis. Swithing between these two codepages is accomplished
through the use of the :hp2.CHCP:ehp2. command within a specific windowed or
full-screen command prompt.

:p.Windows long filenames are stored in :hp2.Unicode:ehp2.. Unicode is an
alternative method for describing characters on the screen and in filesystems
which encompasses nearly all human-readable characters in a single set, without
the need to select (and/or switch) codepages. (Short - 8.3 - filenames are
stored in the active codepage.)

:p.Because &f32. was developed primarily for Windows systems, it, too, utilizes
Unicode for the storage of file and directory names in the filesystem.

:p.The :hp2.&an. &f32. filesystem driver's:ehp2.
:link reftype=hd res=043.cache driver:elink. is responsible for setting the
proper codepage to use for its Unicode translate table through the use of the
:hp2./CP&colon.:ehp2. option. The codepage set with this option must match the
active codepage for any files accessed during that session.

:p.When changing codepages (via the :hp2.CHCP:ehp2. command mentioned above),
it is necessary to run :link reftype=hd res=043.CACHEF32:elink. with the
:hp2./CP&colon.:ehp2.:hp3.nnnn:ehp3. option to load the new table.

:nt.A codepage change (via the :hp2.CHCP:ehp2. command) is only valid for the
session in which the change has been made. Once ended, or switched to a
different session, the primary codepage is in force. Likewise, a Unicode
translate table change to the matching codepage (via
:hp2.CACHEF32 /CP&colon.:ehp2.:hp3.nnnn:ehp3.) is also only valid for the
session in which the change has been made.
:ent.

:caution.
Do not change codepage if open files exist with filenames containing
extended ASCII characters as data loss may occur.
:ecaution.
:p.

.* ----------------------------------------------------------------------------
:h3 res=078 name=xattrs
    x=left y=bottom width=100% height=100%.Extended Attributes

:p.Extended Attribute (EA) support in the
:hp2.&an. &f32. filesystem driver:ehp2. is disabled by default, and must be
explicity enabled with the :hp2./EAS:ehp2. option on the
:link reftype=hd res=042.FAT32 IFS:elink. line in CONFIG.SYS. 

:p.EA support is implemented by the :hp2.&an. &f32. filesystem driver:ehp2. as
follows&colon. 

:p.Where EAs are to be &odq.attached&cdq. to a given file or directory, a
matching file is created consisting of that file or directory name with
&odq.:hp1. EA. SF:ehp1.&cdq. appended to it.

:p.:hp2.Example&colon.:ehp2.

:xmp.
    &odq.MyFile.txt&cdq. exists on a local JFS volume with 32 bytes
    of EAs associated with it. Upon copying this file to a
    &f32. volume when EA support has been enabled in the
    &f32. filesystem driver, the file is copied to the &f32.
    volume and its EAs are stored in &odq.MyFile EA. SF&cdq..
:exmp.

:p.These EA files have their DOS filesystem attributes set hidden, read-only,
and system. to keep them safe and hidden.

:nt.&os2. will not list these files (the filesystem driver hides them),
however they may be visible to other operating systems. The usual precautions
apply when dealing with these EA files as when dealing with any &os2. files
with EAs attached.
:ent.

:p.In addition to creating an EA file for each file or directory with extended
attributes associated with it, a special byte is set in the directory entry
which acts as a flag for the filesystem driver to know that an EA file is
associated with the main file or directory. This is called the 
:link refid=mark_byte reftype=fn.mark byte:elink..

:p.:hp2.Cost vs Benfit:ehp2.

:p.Enabling EA support will impose a minor performance penalty for operations
such as directory listings.

:p.Because &os2. (the Workplace Shell in particular) relies heavily upon
the use of extended attributes to store file and directory metadata, having the
ability to maintain EAs across filesystems generally provides for a more
consistent user experience. In addition, REXX .CMD files are
&odq.tokenized&cdq. each time they are executed. When extended attribute
support is available in the filesystem from which these scripts are run, the
REXX is tokenized in the script's EAs, improving performance.

:nt.Before adding EA support to a volume which did not previously have any
extended attributes stored on it, it is necessary to run CHKDSK /F against it
(or select the context menu item :hp1.Check disk...:ehp1. for the drive object
associated with the &f32. volume and check the :hp1.Write corrections to
disk:ehp1. box).
:ent.
:p.

.* ----------------------------------------------------------------------------
:h3 res=082 name=dos_winos2
    x=left y=bottom width=100% height=100%.DOS and WinOS/2

:p.DOS and WinOS/2 sessions require special handling for filesystems supporting
long names and for filesystems utilizing Unicode.

:p.As mentioned :link reftype=hd res=074.previously:elink., the long filename
is stored in Unicode and the short (DOS-compatible, 8.3) filename is stored
using the currently active codepage. The
:hp2.&an. &f32. filesystem driver:ehp2. tracks these names and keeps them
synchronized between sessions, utilizing the shortened versions for the DOS or
WinOS/2 session, using the following convention&colon.

:ul compact.
:li.Dots (used as :hp1.filename:ehp1.:hp2..:ehp2.:hp1.extension:ehp1.
separators under DOS) are limited to one; extra dots are removed from the
filename, preserving the last one.
:li.If necessary, the filename is converted to uppercase, truncated to six
(or fewer) characters, appending a tilde &odq.&tilde.&cdq. and a sequence
number. Thus, &odq.&tilde.1&cdq. is appended to each non-duplicate filename.
Filenames which would otherwise result in duplicated names are given sequence
numbers of &odq.&tilde.2&cdq., &odq.&tilde.3&cdq., etc., shortening the number
of long filename characters to allow for additional digits in the sequence
number.
:note.This method differs - though is not incompatible with -
:link refid=windows_shortnames reftype=fn.the method used by Windows:elink.
to calculate short filenames when five (or more) files exist which would result
in duplicate short names.
.br
:li.Filename extensions are truncated to no more than three characters,
converted to uppercase.

:eul.

:p.:hp2.Example 1&colon.:ehp2.

:xmp.
    &odq.MySuperLongFilename.WithLongExtension&cdq. exists on a &f32.
    volume. Viewing this file from a DOS prompt, it is listed
    as &odq.MYSUPE&tilde.1.WIT&cdq.. The same shortened filename
    is listed from Windows File Manager, in lower or uppercase,
    depending upon the option setting (default is lowercase).
:exmp.

:p.:hp2.Example 2&colon.:ehp2.

:xmp.
    &odq.My.Super.Long.Filename.With.Extra.Dots&cdq. exists on a &f32.
    volume. Viewing this file from a DOS prompt, it is listed
    as &odq.MYSUPE&tilde.1.DOT&cdq..
    :exmp.

:p.:hp2.Example 3&colon.:ehp2.

:xmp.
    &odq.MySuperLongFilename.WithLongExtension&cdq. and
    &odq.My.Super.LongFilenameWithLong.Extension&cdq. both exist on a
    &f32. volume. Viewing thes file from a DOS prompt, these
    are listed as &odq.MYSUPE&tilde.1.WIT&cdq. and &odq.MYSUPE&tilde.1.WIT&cdq.,
    respectively.
:exmp.

:p.:hp2.Example 4&colon.:ehp2.

:table cols='24 14'.
:row.
:c.Long name
:c.Short name
:row.
:c.This is a test 1.txt
:c.THISIS&tilde.1.TXT
:row.
:c.This is a test 2.txt
:c.THISIS&tilde.2.TXT
:row.
:c.This is a test 3.txt
:c.THISIS&tilde.3.TXT
:row.
:c.This is a test 4.txt
:c.THISIS&tilde.4.TXT
:row.
:c.This is a test 5.txt
:c.THISIS&tilde.5.TXT
:row.
:c.This is a test 6.txt
:c.THISIS&tilde.6.TXT
:row.
:c.This is a test 7.txt
:c.THISIS&tilde.7.TXT
:row.
:c.This is a test 8.txt
:c.THISIS&tilde.8.TXT
:row.
:c.This is a test 9.txt
:c.THISIS&tilde.9.TXT
:row.
:c.This is a test 10.txt
:c.THISI&tilde.10.TXT
:etable.
:exmp.

:caution.
Moving or renaming a file with a long name from a DOS or WinOS/2 session will
result in losing the file's long name, i.e., in the example above, renaming
&odq.MYSUPE&tilde.1.WIT&cdq. to &odq.MYSUPER.TXT&cdq. will result in the long
name being lost so that under an &os2. session, the file is also listed as
&odq.MYSUPER.TXT&cdq..
:p.
Renaming a file with EAs attached (in a DOS or WinOS/2 session) will result in
the EAs being lost. However, EAs are preserved when editing the file and are
properly duplicated when copying the file. See
:link reftype=hd res=078.Extended Attributes:elink. for more about how EAs are
stored on &f32. volumes.
:ecaution.

.* ----------------------------------------------------------------------------
:h3 res=084 name=undelete
    x=left y=bottom width=100% height=100%.Undelete support

:p.&os2.'s UNDELETE functionality is fully compatible with &f32. volumes. To
enable this operating system feature, it is only necessary to add the specified
&f32. drive letter, path, and amount of space to be used on the volume to store
deleted files to the DELDIR environment variable. To enable this for all
sessions by default, edit the :hp2.SET DELDIR=:ehp2. line in CONFIG.SYS.

:nt text='Note 1'.DELDIR requires a drive letter and path for each volume
listed. Thus, it is not possible to set this variable globally for volumes
which do not have their drive letters assigned and for which the drive letter
at the time of mounting may not be known.
:ent.

:nt text='Note 2'.When DELDIR has been set in the master environment, DOS and
WinOS/2 sessions are similarly protected when deleting files. Files deleted
from these sessions and subsequently undeleted will be restored with their
EAs (if origially associated) intact.
:ent.

:p.When a file is undeleted, its case preservation is lost. The undeleted file
will be restored to the directory from which it was deleted, with its filename
in uppercase. This is a limitation of &os2., and not of any particular
filesystem driver.

.* ----------------------------------------------------------------------------
:h3 res=086 name=system_files
    x=left y=bottom width=100% height=100%.System-critical files and booting

:p.It is not possible to boot from a &f32. volume. This is a kernel limitation
of &os2., as the operating system kernel itself cannot recognize a &f32.
filesystem in order to read it prior to loading the
:link reftype=hd res=042.&f32. IFS:elink..

:p.It is not possible to place the &os2. swapfile (SWAPPER.DAT) on a &f32.
volume.
:p.

.* ----------------------------------------------------------------------------
:h1 res=090 name=media
    x=left y=bottom width=100% height=100%.Types of FAT32-formatted media

:p.&f32. is useful for a variety of media, including
:link reftype=hd res=092.memory (RAM) disks:elink.,
:link reftype=hd res=094.internal (fixed) disk drives:elink., and
:link reftype=hd res=096.external (removable) disk or flash drives:elink..

:p.Each type of media has its own characteristics to be considered for any
filesystem. This section describes some particular issues to bear in mind when
using &f32. with these different media types.

:p.In general, media to be formatted as &f32. should be partitioned (not large
floppy type, which is not partitioned).

:p.The &f32. filesystem is not suitable for optical media, such as CD, DVD, or
Blu-Ray.

:p.:ul compact.
:li.:link reftype=hd res=092.RAM disks:elink.
:li.:link reftype=hd res=094.Fixed media:elink.
:li.:link reftype=hd res=096.Removable media:elink.
:eul.
:p.

.* ----------------------------------------------------------------------------
:h2 res=092 name=ram_disks
    x=left y=bottom width=100% height=100%.RAM disks

:p.RAM disks are generally useful for fast read/write access, temporary
storage, such as web browser cache and swap space for specific applications
(though not the
:link reftype=hd res=086.&os2. swapfile:elink., as discussed previously).

:p.Because &f32. requires a minimum volume size of 32MB, RAM disks configured
to mimic floppy drives are not suitable for &f32.

:p.When using this type of virtual media, caching has been shown to negatively
impact performance.

.* ----------------------------------------------------------------------------
:h2 res=094 name=internal_disks
    x=left y=bottom width=100% height=100%.Fixed media

:p.Both mechanical hard disk drives (HDDs) and solid state drives (SSDs) are
well suited for &f32., subject to established volume size restrictions and
any other medium-specific limitations imposed by &os2., such as &os2.'s
inability to properly access Advanced Format (AF) disks which lack proper
sector translation.

:p.When considering a &f32. volume to be created on an an internal drive, be
aware of &os2.'s 2TB volume and device size limit. While the &f32.
specification states that 8TB (and even 16TB) volumes are possible, these are
not compatible with &os2.. Transferring an existing drive or volume which
exceeds the 2TB limit to an &os2. system will result in the inability to
properly access the data, even if the device and volume may be recognized by
&os2.

:p.Caching can dramatically improve performance to mechanical and hybrid 
fixed disk drives, with larger caches (2048KB) recommended for modern systems.
Caching should also improve performance to internal solid state drives, as the
cache memory is &odq.closer&cdq. the CPU than the NAND flash memory in the SSD
itself.

:p.Larger volume sizes require larger cluster sizes. Larger clusters, in turn,
result in more slack (wasted space per file) on the disk, which may be offset
by the benefit of better performance afforded by the larger cluster sizes.
Today, it is rarely advisable to select a smaller cluster size to reduce slack
and conserve disk space. If additional space is required, additional volumes
should be considered, instead, or perhaps slicing a single, 2TB volume into
several smaller volumes using smaller cluster sizes. This may be useful, for
example, when the files to be written to the &f32. volume are generally
smaller than the required cluster size and would thus consume a considerably
greater amount of space (perhaps more than double) as they might under a
smaller cluster size. In short, select the volume and cluster size to fit the
size and type of data to be stored.
:p.

.* ----------------------------------------------------------------------------
:h2 res=096 name=removables
    x=left y=bottom width=100% height=100%.Removable media

:p.Probably the greatest use for the &f32. filesystem on &os2. is to share
data between other operating systems, as &f32. is recognized by most. This is
generally accomplished through the use of removable media, where data is
written under one operating system (or a completely different computer) and
read from another. Because the :hp2.&an. &f32. filesystem driver:ehp2. is
fully capable of reading and writing to the &f32. filesystem, 2-way data
portability is possible udner &os2.

:p.Removable media as it pertains to &f32. includes mechanical hard disk drives
(HDDs), solid state drives (SSDs), and flash media devices, attached to the
computer via :link refid=esata reftype=fn.eSATA:elink. or
:link refid=usb reftype=fn.USB:elink..

:p.The same considerations should be given to removable HDDs and SSDs as
discussed :link reftype=hd res=094.previously:elink.. Caching plays an even
greater role in performance as bandwidth of the connection narrows. The largest
cache possible should be used for these devices to leverage memory closest the
CPU.

:p.USB flash drives (also known by other common terms such as
&odq.USB sticks&cdq., &odq.thumb drives&cdq., or &odq.jump drives&cdq.) are the
most common type of removable media today, available in a wide array of sizes
(both storage size and physical size), shapes, and colors.

:nt.Not all USB flash drives are of good quality. Performance and reliability
can and does vary widely between manufacturers and batches. Whenever possible,
choose quality components from recognized brands, and be wary of long-lost,
just found again media, which may indeed have been lost on the way to the
recycler. Data lost as a result of poor quality media is not the fault of the
:hp2.&an. &f32. filesystem driver:ehp2..
:ent.

:p.USB flash media should be partitioned, just like fixed media as discussed
:link reftype=hd res=094.previously:elink.. For volumes not exceeding 32GB, the
default cluster size of 16KB should be sufficient. For larger volumes, 32KB is
required. As with HDD and SSD devices, due to the limited bandwidth of USB,
caching is essential for good performance, and the larger the cache, the
better.
:p.

.* ----------------------------------------------------------------------------
:h1 res=800 name=trouble
    x=left y=bottom width=100% height=100%.Solving problems

:p.The following tips should help diagnose and recover from various issues when
using the :hp2.&an. &f32. filesystem driver:ehp2.&colon.

:p.:hp2.Poor read/write performance:ehp2.

:p.For RAM disks, caching is usually unnecessary, and can even negatively
impact performance. While read-ahead cache size is determined by the IFS, and
cannot be disabled, it :hp1.is:ehp1. possible to disable the
:link refid=lazywrite reftype=fn.lazy write:elink. daemon by specifying
:hp2./L&colon.OFF:ehp2. on the :link reftype=hd res=043.CACHEF32:elink. line in
CONFIG.SYS (globally) or by running :link reftype=hd res=043.CACHEF32:elink.
from a command prompt in the affected session and specifying the
:hp2./L&colon.OFF:ehp2. option.

:p.:hp2.Inability to mount a &f32. volume:ehp2.

:p.If a &f32. volume is recognized, but access to the data is denied, the
volume may be in need of a :link reftype=hd res=046.disk check:elink.. If the
volume is on a removable device, try removing and reinserting the device. If
the problem persists, further analysis may be required.

:p.:hp2.EA support is enabled, but extended attributes are not being written to
one or more &f32. volumes:ehp2.

:p.Before adding EA support to a volume which did not previously have any
extended attributes stored on it, it is necessary to run CHKDSK /F against it
(or select the context menu item :hp1.Check disk...:ehp1. for the drive object
associated with the &f32. volume and check the :hp1.Write corrections to
disk:ehp1. box).

:p.:hp2.Errors are reported during system startup reporting :ehp2.

:p.Ensure that the :hp2.&an. &f32. filesystem driver:ehp2. is the only &f32.
filesystem driver installed on the system. Files from this distribution are
not interchangeable with those from the &nl. distribution, and will conflict.

:p.Ensure that all :hp2.&an. &f32. filesystem driver:ehp2. files are from the
same release. This may be done  by starting WarpIN, selecting one or more
:hp2.&an. &f32. filesystem driver:ehp2. packages, and selecting
:hp1.Package:ehp1. | :hp1.Verify integrity...:ehp1. from the main menu.

:p.Search for conflicting files in the PATH and verify that the entires in
CONFIG.SYS are pointing to the correct locations on the
:link reftype=hd res=042.&f32. IFS:elink. and the
:link reftype=hd res=043.CACHEF32:elink. lines.

:p.:hp2.Inserting a &f32.-formatted USB flash drive shows a 32MB empty device.
:ehp2.  

:p.This may generally occur for one of two reasons&colon.

:ul compact.
:li.Media is unpartitioned (large floppy) and exceeds the 2GB large floppy
limit for &os2.'s USB mass storage device (MSD) driver; or
:li.Media is partitioned but lacks an LVM signature which is required by later
releases of &os2. Warp 4, MCP2, ACP, ACP2, &ecs., and &aos..
:eul.

:p.In both cases, the safest solution is to use a suitable utility to create a
partition on the device, :hp1.after ensuring that all data to be saved has been
copied to a safe location using a different operating system or suitable
utility:ehp1.. A suitable utility for partitioning will properly add the
necessary LVM signature and create the desired volume.

:p.In the latter case, it may be possible to simply add the LVM signature
without disturbing the existing partition data, but this method is not entirely
reliable. The best course of action is the one with the least risk, so back up
first before making changes in order to avoid data loss.

:p.:hp2.During system boot, LINALLOC FAILED RC=32776 (or other linalloc error 
is reported:ehp2.

:p.Ensure that the :hp2./H:ehp2. option has been specified on the
:link reftype=hd res=042.&f32. IFS:elink. line in CONFIG.SYS to allocate cache
memory into memory space above 16MB.
  
:p.Also, verify that the :hp2.EARLYMEMINIT=TRUE:ehp2. kernel directive is
present in CONFIG.SYS, as this gives device and filesystem drivers access to
the memory above 16MB early in the boot cycle.

:p.:hp2.A problem listed here cannot be resolved through a suggested method or
some other errant behavior is observed.:ehp2.

:p.Review any information available on the
:hp2.&an. &f32. filesystem driver:ehp2. wiki at&colon.

:p.https&colon.//www.arcanoae.com/wiki/fat32/

:p.If you have a valid &aos. Support and Maintenance subscription or an &an.
&os2. &amp. &ecs. Drivers and Software subscription, open a trouble ticket at
&colon.

:p.https&colon.//mantis.arcanoae.com/
:p.

.* ----------------------------------------------------------------------------
:h1 res=900 name=references
    x=left y=bottom width=100% height=100%.References

:p.To find out more about &f32., please visit the following&colon.

:ul compact.
:li.&an. &f32. filesystem driver wiki&colon. https&colon.//www.arcanoae.com/wiki/fat32/
:li.Description of the FAT32 File System http&colon.//support.microsoft.com/kb/154997/
:li.MS-DOS&colon. Directory and Subdirectory Limitations&colon. https&colon.//jeffpar.github.io/kbarchive/kb/039/Q39927/
:li.Overview of FAT, HPFS, and NTFS File Systems&colon. http&colon.//support.microsoft.com/kb/100108/
:li.Volume and file size limits of FAT file systems&colon. https&colon.//web.archive.org/web/20060307082555/http&colon.//www.microsoft.com/technet/prodtechnol/winxppro/reskit/c13621675.mspx
:li.A Brief and Incomplete History of FAT32&colon. http&colon.//www.microsoft.com/technet/technetmag/issues/2006/07/WindowsConfidential/
:eul.
:p.

.* ----------------------------------------------------------------------------
.* Footnotes
:fn id=ras.
:p.The number of sectors to be read per read action and placed in the cache
(Read Ahead Sectors). By default, this value is calculated as twice the number
of sectors per cluster (SPC * 2). The maximum value for this option is 128
(SPC * 128).

:nt.The actual sector I/O per read action is determined by the IFS. For
accessing the File Allocation Table (FAT), single sector reads are performed;
cluster reads are used for file and directory I/O. The RASECTORS option defines
the :hp1.minimum:ehp1. number of sectors the IFS will read from disk and store
in the cache.
:ent.
:efn.

:fn id=mark_byte.
:p.To enhance directory scan performance, each file with extended attributes
(EAs) associated with it is &odq.marked&cdq. by the
:link reftype=hd res=042.&f32. IFS:elink.. An apparently-unused byte at offset
0x0C in the directory entry is set to 0x40 for files with non-critical EAs, to 
0x80 for files with critical EAs, and to 0x00 for files without EAs. 

:p.This byte is not normally read or modified by any known version of Windows
itself, nor by Windows CHKDSK, SCANDISK, or DEFRAG, however, it is possible
that other programs running under Windows or another operating system could
modify it (or even use it for some other purpose).

:p.Should the value in the mark byte be set to 0x00 for a file which actually
:hp1.does:ehp1. have EAs, these EAs will no longer be found using
DosFindFirst/Next system calls. Other &os2. system calls for retrieving EAs
(DosQueryPathInfo, DosQueryFileInfo, and DosEnumAttribute) do not depend on
the mark byte and should be unaffected.

:p.Should the the value in the mark byte be set to 0x40 or 0x80 for a file
which indeed has :hp1.no:ehp1. associated with it, directory scan performance
will be diminished, but no other negative impact should be observed.

:p.The next CHKDSK pass should correct both situations, if necessary.
:p.
:efn.

:fn id=diskidle.
:p.Sets the amount of time (in milliseconds) that a disk device must be idle
before it can accept data from cache memory. The minimum amount of disk idle
time :hp1.must be greater:ehp1. than the value specified for the
BUFFERIDLE parameter.

:p.:hp2.Example&colon.:ehp2.

:p.To set the disk idle time to 2000 milliseconds, and have that value apply
globally, persistent across system restarts, enter the following in CONFIG.SYS
&colon.

:xmp.
    CALL=&lt.path&gt.\CACHEF32.EXE /d&colon.2000
:exmp.
:p.
:efn.

:fn id=bufferidle.
:p.Sets the amount of time (in milliseconds) that the cache buffer can be idle 
before the data it contains may be written to the disk. The minimum amount of
cache buffer idle time :hp1.must be less:ehp1. than the value specified for the
DISKIDLE parameter.

:p.:hp2.Example&colon.:ehp2.

:p.To set the cache buffer idle time to 1000 milliseconds, and have that value
apply globally, persistent across system restarts, enter the following in
CONFIG.SYS&colon.

:xmp.
    CALL=&lt.path&gt.\CACHEF32.EXE /b&colon.1000
:exmp.
:p.
:efn.

:fn id=maxage.
:p.Specifies the maximum amount of time (in milliseconds) before cached data
must be written to the disk. The maximum amount of time specified here
:hp1.must be greater:ehp1. than the value specified for the
DISKIDLE parameter.

:p.:hp2.Example&colon.:ehp2.

:p.To have frequently written data that has been in cache memory no longer than 
4000 milliseconds written to disk, and have that value apply globally,
persistent across system restarts, enter the following in CONFIG.SYS&colon.

:xmp.
    CALL=&lt.path&gt.\CACHEF32.EXE /M&colon.4000
:exmp.
:p.
:efn.

:fn id=lazywrite.
:p.Specifies whether the contents of cache memory are written immediately to 
disk or only during disk idle time. :hp2.ON:ehp2. enables writing of cached
content during disk idle time. :hp2.OFF:ehp2. forces immediate writing to disk.
By default, lazy writing is ON.

:p.:hp2.Example&colon.:ehp2.

:p.To disable lazy write and have that value apply globally, persistent across
system restarts, enter the following in CONFIG.SYS&colon.

:xmp.
    CALL=&lt.path&gt.\CACHEF32.EXE /L&colon.OFF
:exmp.
:p.
:efn.

:fn id=cluster_size.
:p.The following are the default cluster sizes for various volume sizes on
&f32.&colon.

:cgraphic.
 Cluster size   Volume sizes
 ___________________________

 512 bytes      32MB -  64MB
   1KB          64MB - 128MB
   2KB         128MB - 256MB
   4KB         256MB -   8GB
   8KB           8GB -  16GB
  16KB          16GB -  32GB
  32KB          32GB -   2TB
:ecgraphic.
:efn.

:fn id=windows_shortnames.
:p.When five (or more) files exist which would result in duplicate short
names under a Windows operating system, the following convention is applied to
the fifth (and subsequent files, if existing)&colon.
:ul compact.
:li.Only the first :hp1.two:ehp1. characters of the long filename are used.
:li.The next :hp1.four:ehp1. characters of the short filename are determined by
mathematically manipulating the remaining characters of the long filename.
:li.&odq.&tilde.1&cdq. (or another sequence number, to avoid duplication of
another short filename) is appended to the &odq.calculated&cdq. short filename.
:eul.

:p.This approach supposedly improves performance when a large number of similar
long filenames exist.

:p.:hp2.Example&colon.:ehp2.

:table cols='24 14'.
:row.
:c.Long name
:c.Short name
:row.
:c.This is a test 1.txt
:c.THISIS&tilde.1.TXT
:row.
:c.This is a test 2.txt
:c.THISIS&tilde.2.TXT
:row.
:c.This is a test 3.txt
:c.THISIS&tilde.3.TXT
:row.
:c.This is a test 4.txt
:c.THISIS&tilde.4.TXT
:row.
:c.This is a test 5.txt
:c.THA1CA&tilde.1.TXT
:row.
:c.This is a test 6.txt
:c.THA1CE&tilde.1.TXT
:etable.
:efn.

:fn id=esata.
:p.In the case of eSATA, the term &odq.removable&cdq. does not always apply. While
the eSATA specification provides for the ability to hot plug devices, this
support is not universal between hardware manufacturers and operating systems.
&os2. does not currently support hot plugging of eSATA devices, thus, for the
purposes of using these units under &os2., they should be considered fixed
media, and should be connected and powered on at system boot in order to be
properly accessed.
:efn.

:fn id=usb.
:p.Proper access to USB-attached mass storage devices such as hard disk drive
enclosures and flash drives requires a capable USB stack and a USB mass storage
device (MSD) class driver.

:warning.
In addition, it is essential that the quality and condition of the cable (if
required) and any connectors (both in the computer itself and on the cable or
flash drive) be in good working order, providing a tight, positive connection
or inability to access the device or loss of data may occur.
:ewarning.
:efn.


:euserdoc.
