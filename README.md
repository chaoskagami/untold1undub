Info
----------

Something something readme here.

What? No Etrian Odyssey Untold 1 Undub? Not on my watch.

These tools are capable of modifying "Etrian Odyssey Untold: The Millenium Girl" to use Japanese audio, and not a whole lot else. Some assembly required. and with caveats.

The only reason this is possible is due to this guy: https://github.com/xdanieldzd and his tool UntoldUnpack, which served as documentation for implementing my own tool.

How to use
----------

First, get a compiler and run `build.sh`. I've chosen not to use makefiles since nothing is complex enough to justify them.

You'll need an english copy of EoU:TmG and a Japanese copy (as in, Sekai no Meikyuu: Millenium no Shoujo.) More specifically: You need the data files MORI1R.HPI and MORI1R.HPB from both.

First, run the following:

```
./extract JPN_MORI1R.HPI JPN_MORI1R.HPB
```

Next, in the same directory, run the following:

```
./inject ENG_MORI1R.HPI ENG_MORI1R.HPB
```

This will generate two output files: out.hpi and out.hpb. Either rebuild the romfs using these as MORI1R.whatever or use Luma's layeredfs functionality.

For a full undub, you'll also want to overwrite the video files that are not in the archive but the folder in the romfs' `Mobiclip` folder.

How the hell does this work?
----------------------------

I abuse a few properties of the format. First: the archive is a blob of concatenated data and an index of files pointing to offsets in this blob. Voice files are not compressed in the archive, so recalculation is simple: change the offset and size.

The result is not perfect. Sadly, some differences exist bewteen voiced content exist. There may be a few manual corrections needed; a few files exist in the japanese files and not english as well as vice versa. This does not rebuild archives. It patches them to use new appended data, therefore the archive grows by ~80MB.

An additional note to people: A peculiarity may exist in the game's engine. The files in the `Mobiclip` folder exist inside the archive as zero-byte files. It's possible a zero-byte file causes the game to seek the file in the romfs instead, which means one may be able to pull some Oblivion archiveinvalidation-style shenanigans and make a "stub" HPI/HPB with all the files extracted outside the archive. I'll leave this for the next person to test, whoever that may be.

Licensing
----------

You can use this under the terms of the WTFPL.
