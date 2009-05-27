# How to hack on Cubby

This is just miscelaneous notes for now.

## Updating the Version String

Cubby's version string comes from `git describe` and is cached in `version.m4`.

However, currently the build system can't detect when you've made a new commit
and update the version number as it should.

So you want to remove `version.m4` every time you commit. This will cause the
build system to recreate it and rebuild necessary things so the version string
gets updated. You can do this pretty easily with a post-commit hook. Just
putting `rm version.m4` into `.git/hooks/post-commit` should do the trick.
