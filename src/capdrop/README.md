# capdrop

Drops all available capabilities.

## example

It has the nice side-effect of preventing `setuid` from taking effect.  This means things like `crontab` and `at` do not work anymore.

```sh
$ crontab -l
no crontab for user
$ ./capdrop crontab -l
crontabs/user/: fopen: Permission denied
```
