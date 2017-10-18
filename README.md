Info
----------

Something something readme here.

What? No Etrian Odyssey Untold 1 Undub? Not on my watch.

These tools are capable of modifying "Etrian Odyssey Untold: The Millenium Girl" to use Japanese audio, and not a whole lot else. Some assembly required - and with a few caveats (see section `How the Hell does this work?` below.)

The only reason this is possible is due to this guy: https://github.com/xdanieldzd and his tool UntoldUnpack, which served as documentation for implementing my own tool.

Now with support for EOV, due to using the exact same data format! Whee.

How to use
----------

First, get a compiler and run `build.sh`. I've chosen not to use a makefile since nothing is complex enough to justify doing so. You can use any flavor of a POSIX-compatible OS to compile. gcc is specified as the compiler, but you can likely get away with clang. If you're on Windows, you're on your own. Fix the mkdir call to use whatever flavor of CreateDirectory microsoft wants you to use, throw cygwin/msys/mingw/wsl at it, or get a sane OS.

You'll need an english copy of EoU:tMG and a Japanese copy (as in, Sekaiju no Meikyuu: Millenium no Shoujo)

More specifically: You need the data files MORI1R.HPI and MORI1R.HPB from both. The below specifies `ENG_` before a file for the english version of the asset, and `JPN_` for the japanese asset.

First, run the following:

```
./extract JPN_MORI1R.HPI JPN_MORI1R.HPB 1
```

This will generate an output folder named `VOICE` with the japanese voice data. Next, in the same directory, run the following:

```
./inject ENG_MORI1R.HPI ENG_MORI1R.HPB
```

This will generate two output files: out.hpi and out.hpb. These are your undub. Rebuild the romfs using these as MORI1R.HPI/MORI1R.HPB or use Luma's layeredfs functionality with the proper structure to avoid painful CIA install delays.

For a full undub, you'll also want to overwrite the video files that are not in the archive and in the folder named `Mobiclip` within the romfs.

If you want to do this with EOV, replace MORI1R with MORI5 and use '5' as the argument to extract instead of '1'. And use a 
Japanese and English copy of that game, not Untold.

How the hell does this work?
----------------------------

I abuse a few properties of the format. First: the archive is a blob of concatenated data and an index of files pointing to offsets in this blob. Voice files are not compressed in the archive, so recalculation is simple: change the offset and size.

Unfortunately, some differences exist bewteen voiced content across the verstions exist. There may be a few manual corrections needed; some files exist in the japanese files and not in the english ones, as well as vice versa.

This tool does not rebuild archives. It patches them to use new appended data, therefore the archive grows by ~80MB. It is most certainly possible to implement a smarter rebuilder, but ~80MB is not enough for me to care to do so.

An additional note to whoever looks at the code to improve it: The files in the `Mobiclip` folder exist inside the archive as zero-byte files. It's possible a zero-byte file causes the game to seek the file in the romfs instead, which means one may be able to pull some Oblivion archiveinvalidation-style shenanigans and make a "stub" HPI/HPB with all the files extracted outside the archive. This is speculation and not based on any hard reverse engineering, but it may be worth it to test. I have not done so.

Licensing
----------

You can use this under the terms of the WTFPL.
