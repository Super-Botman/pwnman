<div align="center"><pre><h3>0x41sh</h3></pre></div>

<<<<<<< HEAD
=======

# Password manager

- help: show help
- open: open a db
- create: create a new db
- list: list all passwords
- search: search a password

## DB

magic: B0T\0

string {
  u32 size;
  u32 crc32;
  u64 off;
}

entry {
  string title;
  string username;
  string password;
}
>>>>>>> botman
