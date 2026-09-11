# Password manager
A simple - yet effective - password manager dev without libc for a 0x41 project
--

**DO NOT USE AS A REAL PASSWORD MANAGER**

## Usage
This password manager use a repl inteface with a list of commands that allows to interact with a custom vault format.
You can check the list of commands using `help`:
```
> help
help: show this help
open <file>: open the db specified by file
create <file>: create a new db called file
close: save and close the actual db
save: save the actual db
list: list all the entries of a db
search <title>:search for an element by title
show <idx>:show the password of one entry
delete <idx>: delete one entry
edit <idx>: edit one entry
add: create a new entry
exit: exit without saving
```
