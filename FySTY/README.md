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
PuTTY plus *pointless frippery*, tremendous amounts of *bloat*, and sans obnoxious maintainers[[1](#r1)][[2](#r2)][[3](#r3)][[4](#r4)][[5](#r5)]

## Screenshot
![Screenshot](https://github.com/lalbornoz/FySTY/raw/master/FySTY/FySTY.png "Screenshot")

## List of newly introduced features (only on Windows presently)
1. **Background BMP or {JPG,PNG,...}[[10](#r10)] images**, similarly to[[6](#r6)]: absolute positioning, centered, scaled, or tiled, with variable opacity
2. **Transparency**, optionally in conjunction with background images, similarly to the mintty[[6](#r7)] implementation, with optional opacity on {focus,focus loss} behaviour
3. **Clickable URLs**, similarly to the mintty[[7](#r7)] implementation, with variable URL pattern matching string[[12](#r12)] (minus DBCS support, because: are you fucking kidding me Simon Tatham?)

## TODO/wish list
1. Bug: emojis are broken re: bgimg & selection
2. Bug: URLs w/ Arabic text in them are broken
3. Cleanup: optimise {reconf,size} bgimg logic, XXX document items & {debugging,error} messages
4. New feature: minimise to system tray[[2](#r2)]
5. New feature: optionally disable RMB paste action (for alghazi)
6. New feature: store configuration in disk file[[1](#r1)], similarly to[[11](#r11)]

## Rationale
1. The “adds bloat”[[2](#r2)][[3](#r3)] “argument” is unsubstantiated bullshit & a pattern:  
   how about if you don't feel like doing something you just say so and stop rationalising your  
   unwillingness to do so?  
   Furthermore: well-near **150K** SLOC of code? Entire OS have been written in less than that!  
   What the bloody hell is the point of DBCS support? Windows 9x support? Get the fuck out!  
   winfrip.[ch] accounts for **less than 1%** of PuTTY! Shame on you, Simon Tatham! *You're*  
   the #1 cause of bloat in PuTTY!  
   The drop-dead obvious solution: lower maintenance cost as much as possible through (source code)  
   modularity, within the context of imperative, impure code *very* poor in architecture & expressiveness  
   -- yes that means you Simon Tatham!
2. Calling features “pointless frippery” is nothing more than valuing your own opinion over that  
   of everyone else: were it pointless, would anyone at all bother in the very first place? Do you  
   fancy yourself better than others? Or is your excuse “I'm a benevolent dictator!” you pathetic  
   Linus Torvalds clone? Sod off you time-wasting parasite! Go do something else that isn't programming!
3. What of (at the very least) KiTTY[[13](#r13)], mintty[[7](#r7)], and PuTTYTray[[11](#r11)]?  
   All of those are either dead or in a questionable state -- are they all wrong? Dualism much?
4. Ultimately: all of this entails maintenance cost & shows lack of concern for end users:
   a complete waste of time, and above all: **net loss for everyone!** It's *vastly* preferable  
   to have {code,features} that work{s,} *at all* as opposed to *nowt*, especially in the face of  
   academic balderdash nonsense “design concerns”[[9](#r9)] that don't relate to the real world  
   in any tangible way.

## Pull requests policy
Pull requests are accepted & welcomed, unless you're Simon Tatham, Owen Dunn, Ben Harris, or Jacob Nevins.

## References
Wed, 20 Jun 2018 11:11:13 +0200 <span id="r1">[1]</span> [PuTTY wish config-locations](https://www.chiark.greenend.org.uk/~sgtatham/putty/wishlist/config-locations.html)  
Wed, 20 Jun 2018 11:11:14 +0200 <span id="r2">[2]</span> [PuTTY wish system-tray](https://www.chiark.greenend.org.uk/~sgtatham/putty/wishlist/system-tray.html)  
Wed, 20 Jun 2018 11:11:15 +0200 <span id="r3">[3]</span> [PuTTY wish transparency](https://www.chiark.greenend.org.uk/~sgtatham/putty/wishlist/transparency.html)  
Wed, 20 Jun 2018 11:11:16 +0200 <span id="r4">[4]</span> [PuTTY wish url-launching](https://www.chiark.greenend.org.uk/~sgtatham/putty/wishlist/url-launching.html)  
Wed, 20 Jun 2018 11:11:17 +0200 <span id="r5">[5]</span> [PuTTY Known Bugs and Wish List, heading: Non-wish list](https://www.chiark.greenend.org.uk/~sgtatham/putty/wishlist)  
Wed, 20 Jun 2018 11:11:18 +0200 <span id="r6">[6]</span> [putty-trans.diff](http://web.archive.org/web/20161013192410/http://www.covidimus.net/projects/putty/2005-10-6/putty-trans.diff)  
Wed, 20 Jun 2018 11:11:19 +0200 <span id="r7">[7]</span> [mintty/mintty: The Cygwin Terminal – terminal emulator for Cygwin, MSYS, and WSL](https://github.com/mintty/mintty)  
Wed, 20 Jun 2018 11:11:20 +0200 <span id="r8">[8]</span> [Project status · Issue #278 · FauxFaux/PuTTYTray](https://github.com/FauxFaux/PuTTYTray/issues/278)  
Wed, 20 Jun 2018 11:11:21 +0200 <span id="r9">[9]</span> [PuTTY hacking guide](https://tartarus.org/~simon/putty-snapshots/htmldoc/AppendixD.html)  
Wed, 21 Jun 2018 13:51:41 +0200 <span id="r10">[10]</span> [Using Image Encoders and Decoders in Managed GDI+ | Microsoft](https://docs.microsoft.com/en-us/dotnet/framework/winforms/advanced/using-image-encoders-and-decoders-in-managed-gdi)  
Thu, 21 Jun 2018 14:37:54 +0200 <span id="r11">[11]</span> [PuTTYTray](https://puttytray.goeswhere.com)  
Fri, 22 Jun 2018 12:39:34 +0200 <span id="r12">[12]</span> [PathMatchSpec function (Windows)](https://msdn.microsoft.com/en-us/library/windows/desktop/bb773727%28v=vs.85%29.aspx)  
  
vim:tw=0
