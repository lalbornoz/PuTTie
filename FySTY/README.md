# **FY**odor do**ST**oevsky's telet**Y**pe!
PuTTY plus *pointless frippery* and tremendous amounts of *bloat*[[1](#r1)][[2](#r2)][[3](#r3)][[4](#r4)][[5](#r5)][[9](#r9)]

## Screenshot
![Screenshot](https://github.com/lalbornoz/FySTY/raw/master/FySTY/FySTY.png "Screenshot")

## Built & tested on
1. MinGW on Ubuntu 16.04.4 LTS & Windows 8.1 x64, resp.

## List of newly introduced features
1. **Background BMP or {JPG,PNG,...}[[10](#r10)] images**, similarly to[[6](#r6)]  
   Absolute positioning, centered, scaled, or tiled, with configurable opacity
2. **Transparency**, similarly to the mintty[[7](#r7)] implementation  
   Optionally in conjunction with background images, configurable opacity on {focus,focus loss} behaviour
3. **Clickable URLs**, similarly to the mintty[[7](#r7)] implementation  
   Configurable URL pattern matching string(s)[[12](#r12)], sans DBCS support; are you fucking kidding me Simon Tatham?
4. **Mouse behaviour changes**, similarly to the mintty[[7](#r7)] and PuTTYTray[[11](#r11)] implementations  
   Configurable {de,in}crease font size with \<Shift\> + \<Mouse wheel\>  
   Configurable {normal,inhibit} RMB processing behaviour (for alghazi)
5. **Always on top**, similarly to the PuTTYTray[[11](#r11)] implementation  
6. Fixes on-clipboard paste thread handle leak present in PuTTY v0.70 and in master.

## TODO/wish list
1. **Bug**: clickable URLs: correctly reset ATTR\_UNDER & honour scrollback position
2. **Bug**: original PuTTY code: fix emoji handling
3. **New feature**: Direct{2D,Write} backend, similarly to[[21](#r21)]
4. **New feature**: minimise to system tray[[2](#r2)], similarly to[[11](#r11)]
5. **New feature**: MS Visual C++ build environment support bits via Winelib & provide installer package(s) & images
6. **New feature**: ssh:// protocol handler registration, similarly to[[19](#r19)]
7. **New feature**: ReGIS[[22](#r22)] and/or Sixel[[23](#r23)] terminal graphics support
8. **New feature**: ephemeral configuration store backend  
   Selectable w/ command-line option in plink, pscp, psftp, puttygen, PuTTY & PuTTYtel
9. **New feature**: store configuration in disk file[[1](#r1)], similarly to[[11](#r11)]

## Pull requests policy
Pull requests are accepted & welcomed, unless you're Simon Tatham, Owen Dunn, Ben Harris, or Jacob Nevins.

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
``Fri, 22 Jun 2018 12:39:34 +0200 [12]`` <a href="https://msdn.microsoft.com/en-us/library/windows/desktop/bb773727%28v=vs.85%29.aspx" id="r12">PathMatchSpec function (Windows)</a>  
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
  
vim:tw=0
