# regparser
Windows registry files (hives) interactive viewer. Can show some logically deleted but not overridden keys and parameters.

## Requirements
* [ncurses](https://www.gnu.org/software/ncurses/)
* [CDK (Curses Development Kit)](http://invisible-island.net/cdk/)

## Building
Install `ncurses` and `CDK` headers and libraries.

For example, on Ubuntu install packages:
```
libcdk5
libcdk5-dev
libncurses5
libncursesw5-dev
```

Then compile using command `make`.

## Usage
### Run
```
regparser [keys] <registry file>
Keys:
-h, --help
	show this help and exit
-s, --show-deleted
	show deleted keys
-d, --only-deleted
	show only deleted keys
```

### Interface & Commands
Press `<q>` to quit.

Top line shows current registry key path.
Lower located two panels.
Switch between them using `<Tab>`.

Left panel shows parent key (`..`) and names of child keys.
Navigation by `<Up>`, `<Down>`, `<PageUp>`, `<PageDown>`.
Command `<Enter>` changes current registry key to parent (`..`) or child key.

Right panel shows current registry key metadata and list of it's parameters.
Navigation by `<Up>`, `<Down>`, `<PageUp>`, `<PageDown>`.
Command `<Enter>` opens parameter view mode.
This mode shows selected parameter metadata and value.
To quit this mode press `<Enter>` again.

Press `<o>` to open another registry file. It shows dialog to choose file.

Press `<s>` to save current key and it's parameters into text .reg file.
