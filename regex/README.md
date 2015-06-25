# pcre

Filtering input and output with Perl Compatible Regular Expressions.

## example

The included sample program filters on stdout.

```sh
$ >  flag echo "The flag is: DefCon{FOOBAR}"
$ >  expr echo "My password is"
$ >> expr echo "Def[cC]on{.*"
$ >> expr echo "H(a)?ck+ th(e|3) pl4[net]{3}"
$ ./regex expr sh
$ echo hello
hello
$ whoami
user
$ ls -la flag
-rw-r----- 1 user user 28 Jun 24 22:04 flag
$ cat flag
Sorry, Dave.  I cant allow you to do that.
```
