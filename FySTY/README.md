```
         _    ,-,    _             DO YOU LIKE PUTTY?
  ,--, /: :\/': :`\/: :\     DO YOU LIKE POINTLESS FRIPPERY?
 |`;  ' `,'   `.;    `: |             IF SO, THEN:
 |    |     |  '  |     |.     _____      ____ _______   __
 | :  |     | pb  |     ||    |  ___|   _/ ___|_   _\ \ / /
 | :. |  :  |  :  |  :  | \   | |_ | | | \___ \ | |  \ V /
  \__/: :.. : :.. | :.. |  )  |  _|| |_| |___) || |   | |
       `---',\___/,\___/ /'   |_|   \__, |____/ |_|   |_|
            `==._ .. . /'           |___/
                 `-::-'       MIGHT       BE EXACTLY WHAT
                                  YOU'RE LOOKING FOR!
```

# **F**\_\_\_ **y**ou **S**imon **T**atham telet**Y**pe
PuTTY plus *pointless frippery*, tremendous amounts of *bloat*, and sans obnoxious maintainers[[1](#r1)][[2](#r2)][[3](#r3)][[4](#r4)][[5](#r5)][[9](#r9)]

## Screenshot
![Screenshot](https://github.com/lalbornoz/FySTY/raw/master/FySTY/FySTY.png "Screenshot")

## List of newly introduced features (only in MinGW build environment & on Windows presently)
1. **Background BMP or {JPG,PNG,...}[[10](#r10)] images**, similarly to[[6](#r6)]  
   Absolute positioning, centered, scaled, or tiled, with configurable opacity
2. **Transparency**, similarly to the mintty[[7](#r7)] implementation  
   Optionally in conjunction with background images, configurable opacity on {focus,focus loss} behaviour
3. **Clickable URLs**, similarly to the mintty[[7](#r7)] implementation  
   Configurable URL pattern matching string(s)[[12](#r12)], sans DBCS support; are you fucking kidding me Simon Tatham?
4. **Click actions**, similarly to the mintty[[7](#r7)] implementation  
   Configurable normal or inhibit RMB processing behaviour (for alghazi)

## TODO/wish list
1. **Bug**: clickable URLs: correctly reset ATTR\_UNDER & honour scrollback position
2. **Bug**: original PuTTY code: fix emoji handling (on {paste,type} vs. on display)
3. **Cleanup**: optimise {reconf,size} bgimg logic, XXX document items & error messages
4. **New feature**: minimise to system tray[[2](#r2)], similarly to[[11](#r11)]
5. **New feature**: store configuration in disk file[[1](#r1)], similarly to[[11](#r11)]

## Rationale
1. **The “adds bloat”[[2](#r2)][[3](#r3)] “argument”**:  
   Unsubstantiated bullshit & a pattern! How about if you don't feel like doing something just say so and stop rationalising your unwillingness to do so?  
   Furthermore: well-near **150K** SLOC of code? Entire OS have been written in less than that! What the bloody hell is the point of DBCS support? Windows 9x support? Get the fuck out! The FySTY code accounts for **less than 1%** of PuTTY! Shame on you, Simon Tatham! *You're* the #1 cause of bloat in PuTTY!  
   The drop-dead obvious solution: lower maintenance cost as much as possible through (source code) modularity -- needless to say, this is only necessary on account of **far too much** imperative & impure code *very* poor in architecture & expressiveness -- yes that means you Simon Tatham!
2. **The “pointless frippery”[[3](#r3)] “argument”**:  
   This is nothing more than valuing your own opinion over that of everyone else: were it pointless, would anyone at all bother in the very first place?
   Do you fancy yourself better than others? Or is your excuse “I'm a benevolent dictator!” you pathetic Linus Torvalds clone?  
   Sod off you time-wasting parasite! Go do something else that isn't programming!
3. **What of ExtraPuTTY[[17](#r17)], KiTTY[[13](#r13)], Nutty[[16](#r16)], mintty[[7](#r7)], PuTTYTray[[11](#r11)], the covidimus patch[[14](#r14)], and transputty[[15](#r15)]?**  
   All of those are either dead or in a questionable state -- and yet: are they all wrong? Dualism much?
4. **Ultimately**: all of this entails **maintenance cost** & shows **lack of concern for end users** -- *who happen to be the only reason your project is of any relevance to anyone whatsoever!*  
   A complete waste of time, and above all: **net loss for everyone!** It's *vastly* preferable to have {code,features} that work{s,} *at all* as opposed to *nowt*, especially in the face of academic balderdash nonsense “design concerns”[[9](#r9)] that don't relate to the real world in any tangible way.

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
  
vim:tw=0
