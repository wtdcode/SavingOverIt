# SavingOverIt

## Introduction

A Saving/Loading tool for "Getting Over It With Bennet Foddy".

Binary release can be found [here](https://github.com/wtdcode/SavingOverIt/releases).

Just be happy :).

## Usage

I register two hotkeys, F7 for saving and F8 for loading.

### Save

Saving can be anytime when you play the game by pressing F7, all saved data will be stored in current directory;

### Load

You can load the latest data by pressing F8 **ONLY IF** you have quitted the game to the main menu.

If you want to load previous data, please modify the index in config.json. For example, if you want to load save1.json, just set index to 1 and restart the program to load it.

## Known Bugs

- The narration wont be saved.

## Build

If you'd like to build it yourself, don't forget to add [jsoncpp](https://github.com/open-source-parsers/jsoncpp).

## TODO

- Add exception handle.
- Maybe GUI?

