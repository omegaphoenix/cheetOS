# cheetOS

To add pre-commit hook for style checking:

1. Download and set the path for Mike Vanier's C style checker.

    http://courses.cms.caltech.edu/cs11/material/c/mike/misc/c_style_guide.html

2. Navigate to project base directory

3. Run this command:

    ln -s ../../pre-commit.sh .git/hooks/pre-commit

Make sure to use the "../../" before the file name because when git is
evaluating the symlink, it does so using `.git/hooks` as its working
directory.
