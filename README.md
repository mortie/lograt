# <img src="https://raw.githubusercontent.com/mortie/lograt/main/icons/lograt.svg" height="30"> Lograt

Lograt is a program to view log files.

![Screenshot](https://raw.githubusercontent.com/mortie/lograt/main/screenshot.png)

Lograt lets you:

* Open and scroll through log files
* Add patterns to highlight matching lines with particular colors
* Search for a pattern, which opens a separate pane with only the matching lines,
  letting you jump straight to particular matching lines

Lograt is conceptually similar to [Glogg](https://glogg.bonnefon.org/),
which does many of the same things as Lograt, and more. However, there are
a few things which Lograt aims to do differently from Glogg:

* Lograt is a GTK application, which tries to be well-integrated in a
  GNOME environment. Glogg uses Qt, which makes it less integrated into
  the environments I personally prefer.
* The main interaction with both Glogg and Lograt, apart from scrolling
  and reading, is to add or modify a pattern (_filter_ in Glogg).
  In Glogg, this is a many-step process, and requires the user to manually
  pick a background and foreground color, and to manually make sure that the
  foreground color is readable on the chosen background color. Lograt gives
  the patterns a dedicated sidebar for quick access to edit, add, remove and
  reorder patterns, and it automatically cycles through high-contrast pattern colors.

Glogg seems like a high-quality tool though, so if Lograt isn't for you,
maybe Glogg is.

## TODO

* Add keyboard navigation.
* Set up cross-compiling for Windows.
* Make releases with pre-compiled binaries.
* Add more sources for log files (SCP? Journald integration? Windows event log?? Suggestions welcome).
* Pick nicer default pattern colors.
* Refactor the log viewer (LogViewer.cc).
* Keep some state across launches.
* Support multiple logs at once using tabs.

## Attribution

The logo is made from icons made by [Smashicons](https://www.flaticon.com/authors/smashicons) from [www.flaticon.com](https://www.flaticon.com/)
