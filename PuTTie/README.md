# PuTTie 💚

PuTTY plus *pointless frippery* and tremendous amounts of *bloat*[[1](#r1)][[2](#r2)][[3](#r3)][[4](#r4)][[5](#r5)][[9](#r9)]

# Table of contents

 1. [What is PuTTie, and how is it different?](#what-is-puttie-and-how-is-it-different)  
 2. [Screenshot](#screenshot)  
 3. [List of newly introduced features](#list-of-newly-introduced-features)  
 4. [Jump list integration](#jump-list-integration)  
 5. [Thanks to](#thanks-to)  
 6. [Built with & tested on](#built-with--tested-on)  
 7. [How to build](#how-to-build)  
 8. [TODO/wish list](#todowish-list)  
 9. [Pull requests policy](#pull-requests-policy)  
10. [References](#references)

[//]: ## "{{{ What is PuTTie, and how is it different?"
## What is PuTTie, and how is it different?

Unlike the other extant and past PuTTY forks, PuTTie is meticulously
engineered such as to minimise the amount and quality of changes required
to the actual PuTTY ~~pigsty~~ ~~incoherent heap of rotten, fermenting
bat manure~~ source code, yet without sacrificing in range of features or
the lack of quality of their implementation.  
  
This is additionally required as PuTTY's upstream is ~~an insecure, socially
inept, belligerent, obstinate, covetous arsehole~~ particularly hostile to
both users, change, reality, contributors, as well as maintainers of PuTTY
forks. As PuTTie has shown to have, over the course of its existence since
late June 2018, very low maintenance cost, it is much less likely to succumb
to the inevitability of bit rot and loss of maintainers.
  
Still not convinced? Here are six (6) *extremely convincing* reasons why
you should use PuTTie:  
  
1. It's really cute!

2. It has a green heart next to its really cute name because PuTTie loves you!

3. It can totally dance the Melbourne shuffle!

4. kade, nadia, roarie and arab like PuTTie:  
   ![kade, nadia, roarie and arab like PuTTie!](https://github.com/lalbornoz/PuTTie/raw/master/PuTTie/PuTTie_endorsements.png "kade, nadia, roarie and arab like PuTTie!")

5. ~~It~~ ~~stabilises your Estradiol and Progesterone levels~~ ~~kindly tells
   your hypothalamus to be nice~~ gives your Estrogen Receptors a hug so they
   won't feel underappreciated when they're like doing a lot of hard work
   and adapting to and/or driving (if you don't actually have the whole
   reproductive bits and stuff) highly dynamic hormonal circumstances and
   totally prevents PMS/PMDD as long as you use it all day long for three (3)
   days without sleeping or eating!

6. The following people/deities have found PuTTie to have been critical to
   their immeasurable contributions to {woman,man,non-binary-,egg-}kind in general:  
  
   kade, nadia, roarie, Brigadier-Sergeant Corporal Erran Morad, Tupac Shakur, Rainer Maria Rilke, Big L,
   Michael Ende, Taktlo$$, Omar Souleyman, King Kool Savas, Christian J. Kiewiet, David N. Cutler
   Sr., Usāmah b. Muḥammad b. Awaḍ bin Lādin, Kāẓim Finjān al-Ḥammāmi, Aḥmad
   al-Asīr, James Thomas "Baby Huey" Ramey, Federico García Lorca, Miguel de Unamuno y Jugo,
   Fyodor Dostoevsky, Immanuel Kant, Ivan the Terrible, Geoffrey Chaucer, the bloke what made Sir
   Gawain and the Green Knight but he said I'm not supposed to share his name, Moses Šem-Ṭо̄v
   de León, Šəlо̄mо̄ b. Yəhūḏā b. Gābīrо̄l, Abū Muḥammad ʿAlī b. Ḥazm,
   Abū Ḥāmid al-Ġazālī, Abū al-Ṭayyib Aḥmad al-Mutanabbī, ʿubādah al-Qazzāz,
   Abū ʿUthman ʿAmr al-Jāḥiẓ of Baṣrah, ʿAbd al-Malik al-Aṣmaʿī, Heraclitus
   of Ephesos, Parmenides of Elea, Ecclesiastes, none of the Biblical Patriarchs because most
   of them either never existed anyway and/or were appropriated from old Mesopotamia for the
   purposes of ramming divine sovereignty of a lower Canaanite deity down everyone's throats,
   Sīn-lēq-unnínni, Mursili I, despite his anger issues, Šubši-mašrā-šakkan, bonus
   points for having a badass name, ʿammu Rāpī the Amorite, utilising the ST100 ancestor of
   VT100, Šarrukīn I of Akkad, all of the Anunnaki, except for Ellil because he doesn't know
   how be nice to people, Ištar the Lioness, Goddess of Transness and stuff, Gudea of Lagaš,
   Wendy Carlos, and Cardi B.
  
   **Don't you want to be like them?** (except for Ellil because he's an arsehole)

[Back to top](#table-of-contents)

[//]: "}}}"
[//]: ## "{{{ Screenshot"
## Screenshot

![Screenshot](https://github.com/lalbornoz/PuTTie/raw/master/PuTTie/PuTTie.png "Screenshot")

[Back to top](#table-of-contents)

[//]: "}}}"
[//]: ## "{{{ List of newly introduced features"
## List of newly introduced features

1. **Background {BMP,EMF,GIF,ICO,JPEG,PNG,TIFF,WMF}[[10](#r10)] images**, similarly to[[6](#r6)]  
   Absolute positioning, centered, fit w/ optional padding, scaled, or tiled, with configurable opacity  
   Melbourne shuffle slideshow with configurable frequency, updated automatically on file changes in images directory
2. **Trans-arency**, similarly to the mintty[[7](#r7)] implementation  
   Optionally in conjunction with background images, configurable opacity on {focus,focus loss} behaviour
3. **Clickable URLs**, similarly to the mintty[[7](#r7)] implementation  
   Configurable URL regular expressions via pcre2[[12](#r12)] and ``ShellExecute()``[[24](#r24)]  
   Configurable ``ShellExecute()`` browser application[[24](#r24)]  
   Configurable mouse motion and LMB click modifiers:
   \<Ctrl\>, \<Alt\>, \<RightAlt|AltGr\>, \<RightCtrl\>,
   and optionally as well as additionally \<Shift\> or \<RightShift\>  
   Configurable underline and/or reverse video on highlight/click, resp. behaviour  
   Regular expressions may contain/match on whitespaces, etc. pp.  
   Extend/shrink URL selection for URLs spanning multiple lines prior to opening URL
   with mouse wheel and configurable (defaults to \<Alt\>) modifier key
4. **Change font size with \<Ctrl\> + \<Mouse wheel\>**, similarly to the mintty[[7](#r7)] and PuTTYTray[[11](#r11)] implementations  
5. **Always on top**, similarly to the PuTTYTray[[11](#r11)] implementation
6. **Minimise to system tray[[2](#r2)]**, similarly to[[11](#r11)]
7. **Store configuration in disk files[[1](#r1)]**, similarly to[[11](#r11)]  
   Selectable w/ command-line option ``--file`` in plink, pscp, psftp, pterm, putty & puttytel  
   Additionally, file storage may be initialised from the registry w/ the command-
   line option ``--file=registry``  
   User interfaces are provided within putty & puttytel to clean up, clear, copy, delete, migrate,
   rename, and move sessions, host CAs, host keys, and the persistent Pageant private key and
   jump lists and global options from/to all storage backends (ephemeral, file, registry)
8. **Store configuration in ephemeral storage**  
   Selectable w/ command-line option ``--ephemeral`` in plink, pscp, psftp, pterm, putty & puttytel  
   Additionally, ephemeral storage may be initialised from disk files or the registry w/ the command-
   line options ``--ephemeral=file`` and ``--ephemeral=registry``, respectively  
   User interfaces are provided within putty & puttytel to clean up, clear, copy, delete, migrate,
   rename, and move sessions, host CAs, host keys, the persistent Pageant private key and
   jump lists and global options from/to all storage backends (ephemeral, file, registry)
  
   N.B. The random seed file ``PUTTY.RND`` stored beneath ``%LOCALAPPDATA%`` is still read from,
   if present, on startup and written to on exit due to security concerns, even when ephemeral storage
   is selected.
9. **Persist Pageant private key file names across startups**  
   A list of private key files is stored in file- or registry-based storage to persist key files
   added to Pageant across startups. This list is updated on adding or removing keys and may additionally
   be set from the command line by passing any number of path- or filenames to Pageant as arguments.
  
   This feature may be turned on or off (default) from Pageant's system tray menu.
10. **Launch Pageant at Windows startup**  
   This feature may be turned on or off (default) from Pageant's system tray menu.
11. **GUI, configuration & storage internationalisation**  
   Text strings in the GUI, configuration & storage thereof, including sessions, etc. may contain anything that
   encodes from/to valid UTF-16/UTF-8 and vice versa; UTF-16 is used at and due to Windows API boundaries,
   UTF-8 internally as well as in file contents.
12. **Cache passwords**  
   Caches SSH2 passwords in memory across session restarts and session duplication. This is disabled by default
   and can be enabled in the Frippery panel.  
   WARNING: If and while enabled, this will cache passwords in memory insecurely. Consider not using this on
   shared computers.

[Back to top](#table-of-contents)

[//]: "}}}"
[//]: ## "{{{ Jump list integration"
## Jump list integration

PuTTie fixes bugs (particularly [[25](#r25)]) in PuTTy's jump list integration implementation. Owing to
how shortcuts are specified to interact with jump lists, PuTTie shortcuts must have the Application User
Model ID[[26](#r26)] common to both PuTTie and PuTTY set on them. A jump list-compatible shortcut creation
tool is provided for this purpose. Follow the following steps in order to create such a shortcut:  
  
1. Extract the contents of the PuTTie release archive into a directory that will neither change
   name nor location in the future, such as ``%APPDATA%\PuTTie`` or ``%LOCALAPPDATA%\PuTTie``, etc.
   If this directory is renamed and/or moved, steps 2. and 3. must be repeated.

2. Run ``create_shortcut.exe`` from within this directory. A message box will be displayed on exit
   or failure. If it exits successfully, a shortcut named ``PuTTie.lnk`` by default will now exist
   in the same directory; the tool takes two optional arguments, corresponding to the file or path
   name of the shortcut and the file or path name to ``putty.exe``.

3. The shortcut created in 2. may now be pinned to the Start menu via the context menu.

[Back to top](#table-of-contents)

[//]: "}}}"
[//]: ## "{{{ Thanks to"
## Thanks to

Thanks to <a href="https://github.com/gtwy">James Watt</a> and
<a href="https://github.com/SamKook">SamKook</a> for helping fixing lots of bugs!

[Back to top](#table-of-contents)

[//]: "}}}"
[//]: ## "{{{ Built with & tested on"
## Built with & tested on

1. MinGW w/ GCC v11.3.0 & cmake v3.23.2 on Cygwin v3.4.2-1 & Windows 8.1 x64, resp.
2. MinGW w/ GCC v10-win32 20220324 & cmake v3.24.2 on Ubuntu v22.10 & wine-7.0 (Ubuntu 7.0~repack-8)

[Back to top](#table-of-contents)

[//]: "}}}"
[//]: ## "{{{ How to build"
## How to build

```shell
$ git clone --recurse https://github.com/lalbornoz/PuTTie

#
# Release build, create release archive, no build parallelisation/max. 4 jobs, resp.:
$ ./PuTTie/build.sh -c -R
$ ./PuTTie/build.sh -c -R -j 4

#
# Debug build, create release archive, no build parallelisation/max. 4 jobs, resp.:
$ ./PuTTie/build.sh -c -d -R
$ ./PuTTie/build.sh -c -d -R -j 4

#
# Release build, default to file as storage backend, create release archive, no build parallelisation/max. 4 jobs, resp.:
$ ./PuTTie/build.sh -B file -c -R
$ ./PuTTie/build.sh -B file -c -R -j 4

#
# Help screen:
$ ./PuTTie/build.sh -h
usage: ./PuTTie/build.sh [-B <backend>] [-c] [--clang] [-d] [-D] [--dbg-svr <fname> [..]] [--dbg-cli <fname>] [-h] [-i] [-j <jobs>] [-R] [-t <target>]
       -B <backend>..........: set default storage backend to either of ephemeral, file, or registry (default)
       -c....................: clean cmake(1) cache file(s) and output directory/ies before build
       --clang...............: regenerate compile_commands.json
       -d....................: select Debug (vs. Release) build (NB: pass -c when switching between build types)
       -D....................: select Debug w/o debugging console (vs. Release) build (for usage w/ --dbg-{svr,cli} (NB: pass -c when switching between build types)
       --dbg-svr <fname> [..]: run <fname> w/ optional arguments via wine & local gdbserver on port 1234
       --dbg-cli <fname>.....: debug <fname> w/ MingW gdb previously launched w/ --dbg-svr
       -h....................: show this screen
       -i....................: {clean,install} images {pre,post}-build
       -j <jobs>.............: set cmake(1) max. job count
       -R....................: create release archive (implies -i)
       -t <target>...........: build PuTTY <target> instead of default target
```

[Back to top](#table-of-contents)

[//]: "}}}"
[//]: ## "{{{ TODO/wish list"
## TODO/wish list

1. **New feature**: Direct{2D,Write} backend, similarly to[[21](#r21)]
2. **New feature**: MS Visual C++ build environment support bits via Winelib & provide installer package(s) & images
3. **New feature**: ssh:// protocol handler registration, similarly to[[19](#r19)]
4. **New feature**: ReGIS[[22](#r22)] and/or Sixel[[23](#r23)] terminal graphics support

[Back to top](#table-of-contents)

[//]: "}}}"
[//]: ## "{{{ Pull requests policy"
## Pull requests policy

Pull requests are accepted & welcomed, unless you're Simon Tatham, Owen Dunn,
Ben Harris, or Jacob Nevins.

[Back to top](#table-of-contents)

[//]: "}}}"
[//]: ## "{{{ References"
## References

``Wed, 20 Jun 2018 11:11:13 +0200  [1]`` <a href="https://www.chiark.greenend.org.uk/~sgtatham/putty/wishlist/config-locations.html" id="r1">PuTTY wish config-locations</a>  
``Wed, 20 Jun 2018 11:11:14 +0200  [2]`` <a href="https://www.chiark.greenend.org.uk/~sgtatham/putty/wishlist/system-tray.html" id="r2">PuTTY wish system-tray</a>  
``Wed, 20 Jun 2018 11:11:15 +0200  [3]`` <a href="https://www.chiark.greenend.org.uk/~sgtatham/putty/wishlist/transparency.html" id="r3">PuTTY wish transparency</a>  
``Wed, 20 Jun 2018 11:11:16 +0200  [4]`` <a href="https://www.chiark.greenend.org.uk/~sgtatham/putty/wishlist/url-launching.html" id="r4">PuTTY wish url-launching</a>  
``Wed, 20 Jun 2018 11:11:17 +0200  [5]`` <a href="https://www.chiark.greenend.org.uk/~sgtatham/putty/wishlist" id="r5">PuTTY Known Bugs and Wish List, heading: Non-wish list</a>  
``Wed, 20 Jun 2018 11:11:18 +0200  [6]`` <a href="http://web.archive.org/web/20161013192410/http://www.covidimus.net/projects/putty/2005-10-6/putty-trans.diff" id="r6">putty-trans.diff</a>  
``Wed, 20 Jun 2018 11:11:19 +0200  [7]`` <a href="https://github.com/mintty/mintty" id="r7">mintty/mintty: The Cygwin Terminal – [...]</a>  
``Wed, 20 Jun 2018 11:11:20 +0200  [8]`` <a href="https://github.com/FauxFaux/PuTTYTray/issues/278" id="r8">Project status · Issue #278 · FauxFaux/PuTTYTray</a>  
``Wed, 20 Jun 2018 11:11:21 +0200  [9]`` <a href="https://tartarus.org/~simon/putty-snapshots/htmldoc/AppendixD.html" id="r9">PuTTY hacking guide</a>  
``Thu, 21 Jun 2018 13:51:41 +0200 [10]`` <a href="https://docs.microsoft.com/en-us/dotnet/framework/winforms/advanced/using-image-encoders-and-decoders-in-managed-gdi" id="r10">Using Image Encoders and Decoders in Managed GDI+ | Microsoft</a>  
``Thu, 21 Jun 2018 14:37:54 +0200 [11]`` <a href="https://puttytray.goeswhere.com" id="r11">PuTTYTray</a>  
``Fri, 27 Aug 2021 18:27:53 +0200 [12]`` <a href="https://github.com/PhilipHazel/pcre2" id="r12">GitHub - PhilipHazel/pcre2: PCRE2 development is now based here.</a>  
``Fri, 22 Jun 2018 14:51:14 +0200 [13]`` <a href="http://www.9bis.net/kitty" id="r13">KiTTY - Welcome</a>  
``Fri, 22 Jun 2018 14:56:33 +0200 [14]`` <a href="http://web.archive.org/web/20161013192410/http://www.covidimus.net/projects/putty" id="r14">[...] PuTTY Transparency</a>  
``Fri, 22 Jun 2018 14:58:09 +0200 [15]`` <a href="http://web.archive.org/web/20120505105249/http://cprogramming.hu/transputty/" id="r15">Putty - transparent "eye-candy" patch</a>  
``Fri, 22 Jun 2018 15:00:08 +0200 [16]`` <a href="http://web.archive.org/web/20150214071803/http://groehn.net/nutty/" id="r16">Nutty – SSH Client for Windows</a>  
``Fri, 22 Jun 2018 15:02:18 +0200 [17]`` <a href="http://www.extraputty.com/" id="r17">ExtraPuTTY | Fork of PuTTY</a>  
``Sun, 24 Jun 2018 15:15:52 +0200 [18]`` <a href="https://www.chiark.greenend.org.uk/~sgtatham/putty/team.html" id="r18">PuTTY Team Members</a>  
``Sun, 24 Jun 2018 19:49:00 +0200 [19]`` <a href="https://github.com/FauxFaux/PuTTYTray/issues/203" id="r19">Support ssh:// URI · Issue #203 · FauxFaux/PuTTYTray</a>  
``Sun, 24 Jun 2018 19:57:24 +0200 [20]`` <a href="https://github.com/FauxFaux/PuTTYTray/wiki/Other-forks-of-PuTTY" id="r20">Other forks of PuTTY · FauxFaux/PuTTYTray Wiki</a>  
``Sun, 24 Jun 2018 21:19:05 +0200 [21]`` <a href="https://ice.hotmint.com/putty/d2ddw.html" id="r21">iceiv+putty</a>  
``Mon, 25 Jun 2018 14:59:14 +0200 [22]`` <a href="https://en.wikipedia.org/wiki/ReGIS" id="r22">ReGIS - Wikipedia</a>  
``Mon, 25 Jun 2018 14:59:15 +0200 [23]`` <a href="https://en.wikipedia.org/wiki/Sixel" id="r23">Sixel - Wikipedia</a>  
``Fri, 27 Aug 2021 18:42:02 +0200 [24]`` <a href="https://docs.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shellexecutea" id="r24">ShellExecuteA function (shellapi.h) - Win32 apps | Microsoft Docs</a>  
``Mon, 30 Jan 2023 16:49:06 +0100 [25]`` <a href="https://www.chiark.greenend.org.uk/~sgtatham/putty/wishlist/win-jumplist-trouble.html" id="r25">PuTTY bug win-jumplist-trouble</a>  
``Mon, 30 Jan 2023 16:55:44 +0100 [26]`` <a href="https://github.com/MicrosoftDocs/win32/blob/docs/desktop-src/shell/appids.md" id="r26">win32/appids.md at docs · MicrosoftDocs/win32</a>  

[Back to top](#table-of-contents)

[//]: "}}}"
  
<!--
  vim:tw=0
  -->
