### avupd
Another Vital Update for your rolling-release system.

### Summary
avupd is a tiny program that actually reminds you every day that you need to update your system (rolling-release distro or even half-stable). This can be accomplished with a little shell scripting.

### Usage
1. Clone the repo
2. Run make
3. Place avupd to somewhere in your system where you can locate it through your shell configuration script. Run `echo .${SHELL##*/}rc`

Now add "avupd -c" in your shell configuration file and run `source ~/.${SHELL##*/}rc`. If it shows an update is required, please update it by invoking `avupd -u`.\
You can also use the `-i` with the `-c` flag which will show an X11 window message instead of CLI message.

Note: I made it because I always forget to update my system and the background cron job seems unreliable to me.
