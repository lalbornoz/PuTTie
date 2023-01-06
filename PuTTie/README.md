# PuTTie 💚
PuTTY plus *pointless frippery* and tremendous amounts of *bloat*[[1](#r1)][[2](#r2)][[3](#r3)][[4](#r4)][[5](#r5)][[9](#r9)]

## What is PuTTie, and how is it different?

Unlike the other extant and past PuTTY forks, PuTTie is meticulously
engineered such as to minimise the amount and quality of changes required
to the actual PuTTY ~~pigsty~~ ~~incoherent heap of rotten, fermenting
bat manure~~ source code, yet without sacrificing in range of features or
the quality of their implementation.  
  
This is additionally required as PuTTY's upstream is ~~an insecure, socially
inept, belligerent, obstinate, covetous arsehole~~ particularly hostile to
both users, change, reality, contributors, as well as maintainers of PuTTY
forks. As PuTTie has, empirically speaking as well over the course of its
existence since late June 2018, very low maintenance cost, it is much less
likely to succumb to the inevitability of bit rot and loss of maintainers.
  
Still not convinced? Here are seven (7) *extremely convincing* reasons why
you should use PuTTie:  
  
1. It's really cute!

2. It has a really cute name!

3. It has a green heart next to its really cute name because PuTTie loves you!

4. It can totally dance the Melbourne shuffle!

5. kade, nadia, roarie and arab like PuTTie:  
   ![kade, nadia, roarie and arab like PuTTie!](https://github.com/lalbornoz/PuTTie/raw/master/PuTTie/PuTTie_endorsements.png "kade, nadia, roarie and arab like PuTTie!")

6. It's really really cute!

7. It ~~stabilises your Estradiol and Progesterone levels~~ ~~kindly tells
   your hypothalamus to be nice~~ gives your Estrogen Receptors a hug so they
   won't feel underappreciated when they're like doing a lot of hard work
   and adapting to and/or driving (if you don't actually have the whole
   reproductive bits and stuff) highly dynamic hormonal circumstances and
   totally prevents PMS!

## Screenshot
![Screenshot](https://github.com/lalbornoz/PuTTie/raw/master/PuTTie/PuTTie.png "Screenshot")

## List of newly introduced features
1. **Background {BMP,EMF,GIF,ICO,JPEG,PNG,TIFF,WMF}[[10](#r10)] images**, similarly to[[6](#r6)]  
   Absolute positioning, centered, fit w/ optional padding, scaled, or tiled, with configurable opacity  
   Melbourne shuffle slideshow with configurable frequency
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
4. **Change font size with \<Ctrl\> + \<Mouse wheel\>**, similarly to the mintty[[7](#r7)] and PuTTYTray[[11](#r11)] implementations  
5. **Always on top**, similarly to the PuTTYTray[[11](#r11)] implementation
6. **Minimise to system tray[[2](#r2)]**, similarly to[[11](#r11)]
7. **Store configuration in disk files[[1](#r1)]**, similarly to[[11](#r11)]  
   Selectable w/ command-line option ``--file`` in plink, pscp, psftp, pterm, putty & puttytel  
   User interfaces are provided within putty & puttytel to clear, copy, delete, rename, and
   move sessions and host keys from/to all storage backends (ephemeral, file, registry)
8. **Store configuration in ephemeral storage**  
   Selectable w/ command-line option ``--ephemeral`` in plink, pscp, psftp, pterm, putty & puttytel  
   Additionally, ephemeral storage may be initialised from disk files or the registry w/ the command-
   line options ``--ephemeral=file`` and ``--ephemeral=registry``, respectively  
   User interfaces are provided within putty & puttytel to clear, copy, delete, rename, and
   move sessions and host keys from/to all storage backends (ephemeral, file, registry)
  
   N.B. The random seed file ``PUTTY.RND`` stored beneath ``%LOCALAPPDATA`` is still read from,
   if present, on startup and written to on exit due to security concerns, even when ephemeral storage
   is selected.

## Built with & tested on
1. MinGW w/ GCC v11.3.0 & cmake v3.23.2 on Cygwin v3.4.2-1 & Windows 8.1 x64, resp.

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
usage: ./PuTTie/build.sh [-B <backend>] [-c] [--clang] [-d] [-h] [-i] [-j jobs] [-R] [-t <target>]
       -B <backend>..: select default storage backend to either of ephemeral, file, or registry (default)
       -c............: clean cmake(1) cache file(s) and output directory/ies before build
       --clang.......: regenerate compile_commands.json
       -d............: select Debug (vs. Release) build
       -h............: show this screen
       -i............: {clean,install} images {pre,post}-build
       -j............: set cmake(1) max. job count
       -R............: create release archive (implies -i)
       -t <target>...: build PuTTY <target> instead of default target
```

## TODO/wish list
1. **New feature**: Direct{2D,Write} backend, similarly to[[21](#r21)]
2. **New feature**: MS Visual C++ build environment support bits via Winelib & provide installer package(s) & images
3. **New feature**: ssh:// protocol handler registration, similarly to[[19](#r19)]
4. **New feature**: ReGIS[[22](#r22)] and/or Sixel[[23](#r23)] terminal graphics support
7. **New feature**: clickable URLs spanning >1 line(s)

## Pull requests policy
Pull requests are accepted & welcomed, unless you're Simon Tatham, Owen Dunn,
Ben Harris, or Jacob Nevins.

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
  
vim:tw=0
