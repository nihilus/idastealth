IDAStealth v1.3.3, created 06/28/2011, Jan Newger

CONTENTS OF THIS FILE
---------------------
  * Installation
  * Configuration
  * Usage
  * Known issues
  * Compiling
  * License
  * Changelog
  * Files

WINDOWS XP SP3 SETUP
----------------------------------------------------------
HyperVisor: VMWare Player Free - Fedora
Windows XP 32-bit SP3 - Build: 2600.xpsp.080413-2111 : Service Pack 3
Visual Studio 2010 Professional - Microsoft Visual C++ 2010
Windows DDK - 7600.16385.1 - C:\WinDDK\7600.16385.1

- WTL Version 8.0 - C:\WTL80
Download: https://sourceforge.net/projects/wtl/files/WTL%208.0/WTL%208.0%20Final/

- libboost 1.43
Download: https://sourceforge.net/projects/boost/files/boost-binaries/1.43.0/
C:\boost\libboost_filesystem-vc100-mt-sgd-1_43.lib
C:\boost\libboost_date_time-vc100-mt-sgd-1_43.lib
C:\boost\libboost_regex-vc100-mt-sgd-1_43.lib
C:\boost\libboost_serialization-vc100-mt-sgd-1_43.lib
C:\boost\libboost_system-vc100-mt-sgd-1_43.lib
C:\boost\libboost_thread-vc100-mt-sgd-1_43.lib
C:\boost\libboost_wserialization-vc100-mt-sgd-1_43.lib

- ddkbuild v7.4
Download: https://www.osronline.com/OsrDown.cfm/ddkbuild_v74r43.zip
Install Directory: C:\ddkbuild\ddkbuild.cmd

- OSR Driver Loader V3.0
OSRLOADER.exe

- Environment Variables

-- Visual Studio must find ddkbuild.cmd in its PATH
Solution: TODO

-- System variables
The path to c:\ddkbuild was added to the system PATH variable

- Solution configuration for IDAStealth\HideDebugger
-- HideDebugger
Property Pages\Configuration Properties\C/C++\Preprocessor\Prerocessor Definitions
Added _SCL_SECURE_NO_WARNINGS

-- VC++ Directories
--- Include Directories: Added C:\boost\boost_1_43_0
--- Library Directories: Added C:\boost;c:\boost\boost_1_43_0}\bin.v2\libs;C:\boost
--- Source  Directories: Added C:\boost

- Solution configuration for IDAStealth\IDAStealth
--- Property Pages\Configuration Properties\C/C++\Preprocessor\Prerocessor Definitions
Added _SCL_SECURE_NO_WARNINGS

--- Include Directories: Added C:\boost\boost_1_43_0;c:\boost;C:\WTL80\include;C:\WTL80;../../../../include;$(IncludePath)
--- Library Directories: Added C:\boost;C:\boost\boost_1_43_0\libs;C:\boost_1_54_0\libs;../../../../lib/x86_win_vc_32;C:\Program\boost\boost_1_54_0\stage\lib;$(LibraryPath)
--- Source  Directories: Added c:\boost;$(SourcePath)

- Solution configuration for IDAStealth\IDAStealth
--- Property Pages\Configuration Properties\Debugging\Environment   WXBASE=c:\WinDDK\7600.16385.1
--- Property Pages\Configuration Properties\NMake
---    General\Build Command Line: ddkbuild.cmd -WXP free .
---    General\rebuild All Command Line: ddkbuild.cmd -WXP free . -cZ
---    IntelliSense\IncludeSearchPath: $(W7BASE)\inc\ddk\;$(NMakeIncludeSearchPath)

- Solution configuration for StealthDriver.W7
--- Property Pages\Configuration Properties\Debugging\Environment   WXBASE=c:\WinDDK\7600.16385.1


INSTALLATION
------------

a) IDAStealth
Copy both, HideDebugger.dll and IDAStealth.plw to your IDA plugin
directory.
Optionally, you can copy the provided HideDebugger.ini from \sample_config
to the IDAStealth configuration directory (%APPDATA%\IDAStealth).
The example config includes a profile to hide the IDA debugger from the
newest version of Themida and ASProtect, respectively.

b) IDAStealthRemote
Copy both, IDAStealthRemote.exe and HideDebugger.dll to any directory.
No further installation is required.


CONFIGURATION
-------------

a) IDAStealth
The plugin is configured via the GUI, but you can also directly edit the
configuration file, which can be found at
%APPDATA%\IDAStealth\HideDebugger.ini
The file is created upon startup, so it's not necessary to create it manually.

b) IDAStealthRemote
The server doesn't use any persistent configuration.
The stealth options are transmitted via TCP by the client side IDA plugin.
The TCP port can be configured via command line, i.e.
IDAStealthRemote.exe <port>


USAGE
-----

a) IDAStealth
The plugin is started as usual from the IDA plugins menu.
Note that this menu only appears if a file has been loaded into IDA.
The plugin automatically detects if IDA uses remote debugging and will
try to connect to the IDAStealthRemote server if that's the case.
The configuration options are automatically transferred, so the plugin
behaves exactly the same as if it was started with the local debugger.

IMPORTANT:
The option "Randomize driver name" should be used with caution!
When using this option, you must be sure to NOT have two or more instances
of this driver running at the same time, because there is no way for
IDAStealth to check if another instance already started this driver.
Otherwise your system might crash!

b) IDAStealthRemote
Just run the executable as you would when doing remote debugging with IDA.
If IDAStealth is enabled, it will automatically work with remote debugging.
Make sure, that the remote debugging server is running.


KNOWN ISSUES
------------

The plugin was only thoroughly tested on Windows XP SP3 32-bit.
It is designed only for 32-bit applications and doesn't work
with 64-bit applications.
However, it should also work with Vista/Win7 32-bit, but it wasn't
thoroughly tested on these systems.
In the remote scenario, the "Swallow DBG_PRINTEXCEPTION" technique
doesn't work.
The technique "Improved NtClose" doesn't work on 64-bit operating systems.
In this case just pass the exception back to the debuggee using IDA.
For a list of current issues, visit the IDAStealth issue tracker
at https://newgre.net/trac.


COMPILING
---------

Both projects are VS 2008 solutions and compile out of the box, given
that WTL[1], boost[2] and the IDA SDK headers are in the include path.
The RDTSC driver needs ddkbuild[3] and the Win7 WDK[4]. The driver as well as
the plugin itself make use of the diStorm disassembler library[5].


LICENSE
-------

IDAStealth can be freely used without any restrictions.
For the diStorm license, see the accompanying license file.


CHANGELOG
---------

06/28/2010 - v1.3.3

  * Bugfix: The plugin GUI could crash on Win7 X64 systems
  * Bugfix: If any of the SEH debugging support features was used an
    "internal error 30191" would be raised in IDA as soon as the exception
	occurred in the debuggee
  * Bugfix: Injection of the stealth dll failed if the size of the import
    directory was (intentionally) set to a wrong value
  * Improved: Added profile for the newest version of VMProtect (v2.09)

09/27/2010 - v1.3.2

  * Bugfix: SEH monitoring was not working with IDA versions < v5.7
  * Bugfix: The debug registers could be overwritten by a SEH handler if the
    respective thread never called SetThreadContext before the SEH handler
	was invoked

08/23/2010 - v1.3.1

  * Bugfix: The NtClose hook could cause access violations in some situations
  * Bugfix: In some cases, consecutive calls to GetThreadContext could reveal
    the actual values of the debug registers even when advanced hardware
	breakpoint protection was enabled
  * Improved: The user can give custom names to the stealth and RDTSC emulation
    driver, respectively
  * Some minor fixes and improvements

07/07/2010 - v1.3

  * Added: Added support for the ProcessDebugObjectHandle as well as
    the ProcessDebugFlags parameters to NtQueryInformationProcess hooks
  * Added: The debugger can be automatically halted in the top-level SEH
    handler, or when a new context has been applied by the OS after returning
	from a SEH handler
  * Added: Profile for VMProtect has been added
  * Some minor fixes and improvements

02/15/2010 - v1.2.1

  * Bugfix: DoS in SetThreadContext if supplied context was not readable or
    flags were not writeable
  * Bugfix: Context emulation always used the id of the current thread no
    matter what thread handle was actually given
  * Bugfix: Incorrect handling of ProcessDebugObjectHandle in hook of
    NtQueryinformationProcess in stealth driver
  * Bugfix: Possible dead-lock in context emulation
  * Bugfix: IDAStealth would try to connect to the RemoteStealth server if
    Windbg was selected and would always try to inject the stealth dll for
	any win32 application regardless which debugger module was used
  * Bugfix: 0xC000007B error when starting .NET app which was compiled with /clr:pure
  * Bugfix: Inter-process communication could fail if process id was reused
    between debugger runs ("Error while restoring NT headers...")
  * Bugfix: Tick-delta of zero would cause an exception in HideDebugger.dll
  * Improved: Context emulation now hooks the corresponding Nt* APIs instead
    of the kernel32 functions
  * Improved: GetTickCount + RDTSC increase internal counter by a random value
    from specified interval

12/15/2009 - v1.2

  * Bugfix: RDTSC driver handling; driver service was not deleted in some rare cases
  * Bugfix: RDTSC driver mode was broken due to recent BSOD fix
  * Improved: IDAStealth can hide from Themida with ultra anti debugging settings
  * Added: New stealth driver

11/24/2009 - v1.1.1

  * Bugfix: Old RDTSC driver version slipped into the last release.
    The new one is now included
  * Improved: To increase overall stealth, the NT Headers are restored to
    their original state after the dll has been injected
  * Added: Profile for yoda's Protector added

11/14/2009 - v1.1

  * Bugfix: OpenProcess failed on XP when started from a restricted user account
  * Bugfix: Bound imports directory is only cleared if necessary
  * Bugfix: DBG_PRINT DoS due to improper parameter checking
  * Bugfix: BSOD in RDTSC driver
  * Added: Remote debugging support
  * Added: Profiles support
  * Added: Exceptions with unknown exception code can be automatically passed
    to the debuggee
  * Added: Inline hooks can be forced to use absolute jumps
  * Improved: GUI has been redesigned to be more usable
  * Improved: AWESOME gfx :)
  * Changed: HideDebugger.ini is now located in the user's directory at:
    %APPDATA%\IDAStealth\HideDebugger.ini
  * Improved: Whole project compiles with WL4 and "treat warnings as error"

03/25/2009 - v1.0

  * Bugfix: API hook of GetThreadContext erroneously returned the complete
    context even if the flags specified that only the DRs should be returned.
    This interfered with newer Armadillo versions
  * Improved: GetTickCount hook now mimics the original API algorithm and
    allows for controlling the increasing delta
  * Added: RDTSC emulation driver with optional driver name randomization to
    increase stealthiness. Read these notes carefully before using this feature

09/15/2008 - v1.0 Beta 3

  * Bugfix: NtQuerySystemInformation hook possibly returned wrong error code
    when handling SystemKernelDebuggerInformation query
  * Bugfix: NtQueryObject hook mistakenly assumed that all object names are
    zero terminated strings
  * Improved: NtQueryInformationProcess considers the case that the debuggee
    itself might act as a debugger (see Tuts4You baord)
  * Improved: Exception triggered by NtClose is now blocked in the first place
  * Added: Countermeasures against anti-attach techniques

09/02/2008 - v1.0 Beta 2

  * Bugfix: Due to improper checking of input parameters in the
    NtQuerySystemInformation hook, the debugged process could raise an
	exception, finally unveiling the existence of IDA Stealth
  * Bugfix: Hiding of possibly existing kernel debugger now working correctly
  * Bugfix: Fake parent process and Hide IDA from process list are no longer
    mutual exclusive
  * Bugfix: NtQueryInformationProcess hook accepted too small input buffers
  * Bugfix: NtQueryInformationProcess hook erroneously assumed the process
    handle to be always that of the current process
  * Bugfix: Exception caused by closing an invalid handle is now properly
    hidden from the debugged process by using SEH or Vectored exception handling
  * Bugfix: NtSetInformationThread wasn't hooked at all due to a typo
  * Bugfix: Added checks to hook functions so they behave as expected when an
    invalid handle is passed. Affected functions:
    + NtSetInformationThread
    + SuspendThread
    + SwitchDesktop
    + NtTerminateThread
    + NtTerminateProcess
  * Bugfix: RtlGetVersion returned wrong platform ID and build number
  * Added: Console version of IDA is also hidden from process list

07/24/2008 - v1.0 Beta 1

  * Bugfix: Multiple minor bugfixes
  * Added: Fake OS version
  * Added: Disable NtTerminateThread/NtTerminateProcess

07/14/2008 - v1.0 Alpha 4

  * Bugfix: Injection of stealth dll could fail in some cases

07/13/2008 - v1.0 Alpha 3

  * Added: Multiple stealth techniques (OpenProcess, DBG_PRINTEXCEPTION,
    hardware breakpoint protection, hide IDA process and windows, to name but a few)
  * Improved: Overall stealth: xADT as well as Extreme Debugger Detector 0.5
    are unable to detect an attached debugger (except for RDTSC based tests and
	scanning the HDD for various tools)
  * Bugfix: Plugin didn't correctly de-register from debug callback and
    crashed with newly created databases

07/06/2008 - v1.0 Alpha 2

  * Bugfix: Injection of stealth dll failed if IMAGE_DIRECTORY_ENTRY_IAT of
    process was zero, so the plugin didn't work with most packed executables
  * Bugfix: NtQueryInformationProcess didn't work (CheckRemoteDebuggerPresent
    was implicitly affected)

07/04/2008 - v1.0 Alpha

  * First alpha release, some features still missing, needs testing, major bugs
  * Known Bugs:
    + Problems when modifying import directory of packed executables
	  (error 0xC000007B)


FILES
-----

\bin\IDAStealth       --- the plugin and the stealth dll
    \IDAStealthRemote --- the remote server and the stealth dll
\distorm              --- the license of the diStorm disassembler library
\sample_config        --- sample configuration file with pre defined profiles
                          for Themida and ASProtect
\src                  --- source code
    \HideDebugger     --- stealth dll
    \IDAStealth       --- IDA plugin
    \IDAStealthRemote --- remote server
    \IniFileAccess    --- utility class to read/write ini files
    \NInjectLib       --- library to inject dlls into a process
    \RDTSCEmu         --- kernel mode driver to fake RDTSC return values


[1] WTL - http://wtl.sourceforge.net/
[2] boost - http://www.boost.org/
[3] ddkbuild - http://ddkwizard.assarbad.net/
[4] WDK - http://www.microsoft.com/whdc/devtools/wdk/default.mspx
[5] diStorm - http://ragestorm.net/distorm/